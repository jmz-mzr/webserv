#include "core/Webserv.hpp"
#include "core/Response.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>

#include "config/ConfigParser.hpp"
#include "utils/exceptions.hpp"
#include "utils/global_defs.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace	webserv
{

	volatile std::sig_atomic_t	Webserv::receivedSigInt = 0;

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Webserv::Webserv()
	{
		LOG_INFO("New Webserv instance");
		LOG_DEBUG("With buffer of capacity " << RECV_BUFFER_SIZE);
#ifdef MACOS
		_ioFlags = 0;
#else
		_ioFlags = MSG_DONTWAIT;
#endif
	}

	Webserv::~Webserv()
	{
		std::vector<Server>::iterator	server = _servers.begin();
		std::list<Client>::iterator		client = _clients.begin();

		while (server != _servers.end()) {
			server->closeSocket();
			++server;
		}
		while (client != _clients.end()) {
			client->closeSocket();
			++client;
		}
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	Webserv::_deleteRedundantNames(ServerConfig& newConfig,
											const Server& server) const
	{
		std::vector<ServerConfig>::const_iterator	serverConfig;
		ServerConfig::hostname_set::const_iterator	name;

		serverConfig = server.getConfigs().begin();
		while (serverConfig != server.getConfigs().end()) {
			name = serverConfig->getServerNames().begin();
			while (name != serverConfig->getServerNames().end()) {
				if (newConfig.eraseName(*name))
					LOG_WARN("Conflicting server name \"" << *name << "\" on "
							<< server.getSocket().getIpAddr() << ":"
							<< server.getSocket().getPort() << ", ignored");
				++name;
			}
			++serverConfig;
		}
	}

	bool	Webserv::_sameSocket(const sockaddr_in& listenPair,
									const sockaddr_in& serverAddr) const
	{
		return (listenPair.sin_port == serverAddr.sin_port &&
				listenPair.sin_addr.s_addr == serverAddr.sin_addr.s_addr);
	}

	Webserv::server_iter	Webserv::_findSameSocket(const sockaddr_in& listenPair)
	{
		std::vector<Server>::iterator	server = _servers.begin();

		while (server != _servers.end()) {
			if (_sameSocket(listenPair, server->getSocket().getAddr()))
				break ;
			++server;
		}
		return (server);
	}

	void	Webserv::_addServer(const Config& config)
	{
		Config::listen_set::const_iterator	listenPair;
		server_iter							server;
		struct pollfd						newPollFd;

		listenPair = config.getListens().begin();
		while (listenPair != config.getListens().end()) {
			ServerConfig	newConfig(config, *listenPair);

			server = _findSameSocket(*listenPair);
			if (server != _servers.end()) {
				_deleteRedundantNames(newConfig, *server);
				server->addConfig(newConfig);
			} else {
				_servers.push_back(Server(newConfig, *listenPair));
				newPollFd.fd = _servers.back().getSocket().getFd();
				newPollFd.events = POLLIN;
				_pollFds.push_back(newPollFd);
			}
			++listenPair;
		}
	}

	void	Webserv::_loadServers(const std::list<Config>& config)
	{
		std::list<Config>::const_iterator	configIter;

		configIter = config.begin();
		while (configIter != config.end()) {
			_addServer(*configIter);
			++configIter;
		}
	}

	void	Webserv::_usageHelper()
	{
		std::cerr << "Usage: ./webserv [CONFIG FILE]" << std::endl;
	}

	// struct print {
	// 	print() : depth(0) {}
	// 	void operator()(const Config& confs)
	// 	{
	// 		for (int i = 0; i < depth; i++)
	// 			std::cout << "\t";
	// 		std::cout << confs;
	// 		depth++;
	// 		for (std::map<std::string, Config>::const_iterator it = confs.getConfigs().begin();
	// 			it != confs.getConfigs().end();
	// 			it++) {
	// 			this->operator()(it->second);
	// 			}
	// 		depth--;
	// 	}
	// 	int depth;
	// };

	void	Webserv::init(int argc, char** argv)
	{
		std::string		webservRoot(XSTR(WEBSERV_ROOT));

		if (argc > 2 || argc < 1) {
			_usageHelper();
			LOG_DEBUG("argc=" << argc);
			THROW_LOGIC("Bad number of arguments");
		} else {
			ConfigParser config(
				(argc == 2) ? argv[1] : XSTR(CONF_FILE) );
			_loadServers(config.parseFile());
		}
		if (webservRoot.size() == 0)
			THROW_FATAL("WEBSERV_ROOT cannot be an empty path");
		if (webservRoot[0] != '/')
			THROW_FATAL("WEBSERV_ROOT must start at root ('/')");
		(void)Logger::getInstance();
		Response::initResponseMaps();
	}

	Webserv::client_iter	Webserv::_removeClient(client_iter client,
													pollFd_iter pollFd)
	{
		LOG_INFO("Removing client (fd=" << client->getSocket().getFd() << ")");
		client->closeSocket();
		if (pollFd != _pollFds.end())
			_pollFds.erase(pollFd);
		return (_clients.erase(client));
	}

	void	Webserv::_addClient(const int serverFd, const Server& server)
	{
		struct pollfd		newPollFd;

		try {
			_clients.push_back(Client(serverFd, server.getConfigs()));
		} catch (const std::exception& e) {
			LOG_ERROR("Could not add client to _clients: " << e.what());
			return ;
		}
		try {
			newPollFd.fd = _clients.back().getSocket().getFd();
			newPollFd.events = POLLIN | POLLOUT;
			newPollFd.revents = 0;
			_pollFds.push_back(newPollFd);
		} catch (const std::exception& e) {
			_removeClient(--_clients.end(), _pollFds.end());
			LOG_ERROR("Could not add client to _pollFds: " << e.what());
			return ;
		}
		LOG_INFO("Client added to Server listening on \""
				<< server.getSocket().getIpAddr() << ":"
				<< server.getSocket().getPort() << "\"");
	}

	Webserv::pollFd_iter	Webserv::_findPollFd(const int fdToFind,
													const size_t hint,
													const std::string& object)
	{
		size_t	i;

		if (hint < _pollFds.size() && _pollFds[hint].fd == fdToFind)
			return (_pollFds.begin() + static_cast<long>(hint));
		i = 0;
		while (i < _pollFds.size()) {
			if (_pollFds[i].fd == fdToFind)
				return (_pollFds.begin() + static_cast<long>(i));
			++i;
		}
		LOG_ERROR("Unable to find " << object << " (fd="
				<< fdToFind << ") in _pollFds");
		return (_pollFds.end());
	}

	void	Webserv::_showOtherRevents(pollFd_iter pollFd,
										const std::string& object) const
	{
		if ((object == SERVER
					&& (pollFd->revents & ~POLLIN) != 0)
				|| (object == CLIENT
					&& ((pollFd->revents & ~(POLLIN | POLLOUT)) != 0)))
			LOG_DEBUG(object << " (fd=" << pollFd->fd << ") "
					<< "received other poll flags:"
					<< " POLLERR=" << ((pollFd->revents & POLLERR) != 0)
					<< " POLLHUP=" << ((pollFd->revents & POLLHUP) != 0)
					<< " POLLNVAL=" << ((pollFd->revents & POLLNVAL) != 0));
	}

	void	Webserv::_acceptConnections()
	{
		std::vector<Server>::const_iterator		server;
		std::vector<struct pollfd>::iterator	pollFd;

		for (size_t i = 0; i < _servers.size(); ++i) {
			server = _servers.begin() + static_cast<long>(i);
			pollFd = _findPollFd(server->getSocket().getFd(), i, SERVER);
			if (pollFd == _pollFds.end())
				continue ;
			_showOtherRevents(pollFd, SERVER);
			if ((pollFd->revents & POLLIN) != 0)
				_addClient(pollFd->fd, *server);
		}
	}

	ssize_t	Webserv::_receiveClientRequest(Client& client, pollFd_iter pollFd)
	{
		// TO DO: 1) What if a request comes while last one is not terminated?
		// 		  Or while the last response has not been (fully) sent yet?
		// 		  Wait before the next recv? Ignore it?
		// 		  Drop previous request? prepareErrorResponse(...)?
		// 		  Implement request pipelining? Create a queue of responses?
		// 		  Or handle this in "_handleClientRequest()"?
		// 		  2) How to keep the client able to interact with the server
		// 		  while the server sends a large file to the client?
		// 		  Send it through a new socket connected to the client?

		int			clientFd = client.getSocket().getFd();
		ssize_t		received;

		_buffer[0] = '\0';

		if (client.hasTimedOut())
			return (1);
		// TODO : Recv before returning 1
		if (client.hasUnprocessedBuffer())
		{
			LOG_DEBUG("hasunprocessedBuffer, no recv");
		   return (1);
		}
		if (client.isProcessingRequest()) {
			LOG_DEBUG("Finish responding to the last request before receiving"
					<< " this new client request (fd=" << clientFd << ")");
			return (1);	// see "TO DO" for other options
		}
		if ((pollFd->revents & POLLIN))
		{
			LOG_DEBUG("Nothing to read");
		   	return (1);
		}
		received = recv(clientFd, _buffer, RECV_BUFFER_SIZE - 1, _ioFlags);
		if (received > 0) {
			_buffer[received] = '\0';
			client.updateTimeout();
			LOG_INFO("Received a client request (fd=" << clientFd << ")");
			LOG_DEBUG("Request: " << _buffer);
		} else if (received == -1)
			LOG_ERROR("Could not receive the client request "
					<< " (fd=" << clientFd << "): " << strerror(errno));
		return (received);
	}

	bool	Webserv::_handleClientRequest(Client& client)
	{
		int		errorCode;

		if (client.hasTimedOut()) {
			LOG_DEBUG("The client (fd=" << client.getSocket().getFd()
					<< ") timed out");
			return (client.prepareErrorResponse(408));
		}
		if (!client.hasRequestTerminated()) {
			errorCode = client.parseRequest(_buffer);
			if (errorCode != 0)
				return (client.prepareErrorResponse(errorCode));
		}
		if (client.hasRequestTerminated() && !client.hasResponseReady())
			return (client.prepareResponse());
		return (true);
	}

	bool	Webserv::_handleClientResponse(Client& client, pollFd_iter pollFd)
	{
		// NOTE: NGINX doesn't even send a response for the 408 code, it just
		// closes the connection (because of "return"/"timeout" directive)

		const Response&		response = client.getResponse();

		if (response.getResponseCode() == 408)
			return (false);
		if ((pollFd->revents & POLLOUT) != 0 && client.hasResponseReady()) {
			if (!client.sendResponse(_ioFlags))
				return (false);
			if (response.isPartialResponse())
				return (true);
			if (!response.isKeepAlive())
				return (false);
			client.clearResponse();
			client.clearRequest();	// not here?
		}
//		if (!client.isKeepAlive())
//			return (false);
		return (true);
	}

	void	Webserv::_handleClients()
	{
		client_iter		client = _clients.begin();
		size_t			i = _servers.size();
		pollFd_iter		pollFd;
		ssize_t			received;


		while (client != _clients.end()) {
			pollFd = _findPollFd(client->getSocket().getFd(), i, CLIENT);
			if (pollFd != _pollFds.end()) {
				_showOtherRevents(pollFd, CLIENT);
				received = _receiveClientRequest(*client, pollFd);
				if (received > 0 && !_handleClientRequest(*client)) {
					client = _removeClient(client, pollFd);
					continue ;
				}
				if (received <= 0 || !_handleClientResponse(*client, pollFd)) {
					client = _removeClient(client, pollFd);
					continue ;
				}
			}
			++client;
			++i;
		}
	}

	void	Webserv::run()
	{
		while (!Webserv::receivedSigInt) {
			if (poll(_pollFds.data(),
						static_cast<uint32_t>(_pollFds.size()), -1) == -1) {
				LOG_WARN("poll() error: " << strerror(errno));
				continue ;
			}
			_acceptConnections();
			_handleClients();
		}
	}

}	// namespace webserv
