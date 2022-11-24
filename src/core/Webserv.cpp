#include "core/Webserv.hpp"

#include <cstring>
#include <cerrno>
#include <iostream>

#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>

#include "config/ParseConfig.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

#define SERVER		"server"
#define CLIENT		"client"

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
		std::vector<std::string>::const_iterator	name;

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

	bool	Webserv::_sameSocket(const listen_pair& listenPair,
									const Server& server) const
	{
		const std::string&	ipAddr = listenPair.first;
		uint16_t			port = listenPair.second;

		if (server.getSocket().getIpAddr() == ipAddr
				&& server.getSocket().getPort() == port)
			return (true);
		return (false);
	}

	Webserv::server_iter
		Webserv::_findSameSocket(const listen_pair& listenPair)
	{
		std::vector<Server>::iterator	server = _servers.begin();

		while (server != _servers.end()) {
			if (_sameSocket(listenPair, *server))
				break ;
			++server;
		}
		return (server);
	}

	void	Webserv::_addServer(const ServerConfig& serverConfig)
	{
		std::vector<std::pair<const std::string, uint16_t> >::const_iterator
												listenPair;
		std::vector<Server>::iterator			server;
		struct pollfd							newPollFd;

		listenPair = serverConfig.getListenPairs().begin();
		while (listenPair != serverConfig.getListenPairs().end()) {
			ServerConfig	newConfig(serverConfig);

			newConfig.clearListenPairs();
			newConfig.addListenPair(*listenPair);
			server = _findSameSocket(*listenPair);
			if (server != _servers.end()) {
				_deleteRedundantNames(newConfig, *server);
				server->addConfig(newConfig);
			} else {
				_servers.push_back(Server(newConfig, *listenPair));
				newPollFd.fd = _servers.back().getSocket().getFd();
				newPollFd.events = POLLIN;
				_pollFds.push_back(newPollFd);
				LOG_INFO("Server added to Webserv");
			}
			++listenPair;
		}
	}

	void	Webserv::_loadServers()
	{
		std::vector<ServerConfig>::const_iterator	serverConfig;

		serverConfig = _config.getServerConfigs().begin();
		while (serverConfig != _config.getServerConfigs().end()) {
			_addServer(*serverConfig);
			++serverConfig;
		}
	}

	void	Webserv::_usageHelper()
	{
		std::cerr << "Usage: ./webserv [CONFIG FILE]" << std::endl;
	}

	void	Webserv::_parseConfig(std::string configFilePath)
	{
		ParseConfig	parser(configFilePath);

		parser();
		LOG_DEBUG("_tokens=[ " << parser.getTokens() << BHWHT << " ]");
	}

	void	Webserv::init(int argc, char** argv)
	{
		if (argc > 2 || argc < 1) {
			_usageHelper();
			LOG_EMERG("bad number of arguments");
			LOG_DEBUG("argc=" << argc);
			throw LogicErrorException();
		} else if (argc == 2) {
			_parseConfig(argv[1]);
		} else {
			_parseConfig(DEFAULT_CONF_FILE);
		}
		_loadServers();
	}

	void	Webserv::_broadcastMsg(const std::string& msg,
									const int senderId) const	// tmp exam version
	{
		for (std::list<Client>::const_iterator client = _clients.begin();
				client != _clients.end(); ++client) {
			if (client->getId() == senderId)
				continue ;
			if (send(client->getSocket().getFd(), msg.c_str(), msg.size(),
						MSG_DONTWAIT) == -1)
				LOG_ERROR("Could not send message to client (id="
						<< client->getSocket().getFd() << "): "
						<< strerror(errno));
		}
	}

	Webserv::client_iter	Webserv::_removeClient(client_iter client,
													pollFd_iter pollFd)
	{
		std::ostringstream		msg;

		msg << "server: client " << client->getId() << " just left\n";	// tmp exam version
		_broadcastMsg(msg.str(), client->getId());	// tmp exam version
		LOG_INFO("Removing client (fd=" << client->getSocket().getFd() << ")");
		client->closeSocket();
		if (pollFd != _pollFds.end())
			_pollFds.erase(pollFd);
		return (_clients.erase(client));
	}

	void	Webserv::_addClient(const int serverFd, const Server& server)
	{
		static int			clientId = 0;	// tmp exam version
		struct pollfd		newPollFd;
		std::ostringstream	msg;	// tmp exam version

		try {
			_clients.push_back(Client(clientId, serverFd, server.getConfigs())); // tmp version
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
		msg << "server: client " << clientId << " just arrived\n";	// tmp exam version
		_broadcastMsg(msg.str(), clientId);	// tmp exam version
		++clientId;	// tmp exam version
	}

	Webserv::pollFd_iter	Webserv::_findPollFd(const int fdToFind,
													const size_t hint,
													const std::string& object)
	{
		size_t	i;

		if (hint < _pollFds.size() && _pollFds[hint].fd == fdToFind)
			return (_pollFds.begin() + hint);
		i = 0;
		while (i < _pollFds.size()) {
			if (_pollFds[i].fd == fdToFind)
				return (_pollFds.begin() + i);
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
			server = _servers.begin() + i;
			pollFd = _findPollFd(server->getSocket().getFd(), i, SERVER);
			if (pollFd == _pollFds.end())
				continue ;
			_showOtherRevents(pollFd, SERVER);
			if ((pollFd->revents & POLLIN) != 0)
				_addClient(pollFd->fd, *server);
		}
	}

	ssize_t	Webserv::_receiveClientRequest(Client& client)
	{
		// TO DO: What if a request comes while the last one is not terminated?
		// 		  Or while the last response has not been (fully) sent yet?
		// 		  Wait before the next recv? Ignore it?
		// 		  Drop previous request? prepareErrorResponse(...)?
		// 		  Implement request pipelining? Create a queue of responses?
		// 		  Or handle this in "_handleClientRequest()"?

		int			clientFd = client.getSocket().getFd();
		ssize_t		received;

		if (client.isProcessingRequest()) {
			LOG_DEBUG("Finish processing the last request before receiving"
					<< " this new client request (fd=" << clientFd << ")");
			return (-1);	// see "TO DO" for other options
		}
		received = recv(clientFd, _buffer, RECV_BUFFER_SIZE - 1, _ioFlags);
		if (received > 0) {
			_buffer[received] = '\0';
			LOG_INFO("Received a client request (fd=" << clientFd << ")");
			LOG_DEBUG("Request: " << _buffer);
		} else if (received == -1)
			LOG_ERROR("Could not receive the client request "
					<< " (fd=" << clientFd << "): " << strerror(errno));
		return (received);
	}

	int	Webserv::_extractMsg(std::string& buffer, std::string& msg)	// tmp exam version
	{
		size_t	i;

		msg.clear();
		if (buffer.empty())
			return (0);
		i = 0;
		while (buffer[i]) {
			if (buffer[i] == '\n') {
				msg.swap(buffer);
				buffer.assign(msg, i + 1, std::string::npos);
				msg.erase(i + 1);
				return (1);
			}
			++i;
		}
		return (0);
	}

	void	Webserv::_bufferAndSendMsg(Client& client)	// tmp exam version
	{
		int					extracted;
		std::string			msg;
		std::ostringstream	msgWithHeader;

		try {
			client.buffer += _buffer;
		} catch (const std::exception& e) {
			LOG_ERROR("Unable to buffer the client message (id="
					<< client.getId() << ")");
		}
		while ((extracted = _extractMsg(client.buffer, msg)) > 0) {
			msgWithHeader << "client " << client.getId() << ": " << msg;
			_broadcastMsg(msgWithHeader.str(), client.getId());
		}
	}

	void	Webserv::_handleClientRequest(Client& client)
	{
		client.parseRequest(_buffer);
		if (client.hasError())
			client.prepareErrorResponse();
		else if (client.hasRequestTerminated())
			client.prepareResponse();
	}

	bool	Webserv::_sendResponse(Client& client, int clientFd)
	{
		const char*	buff = client.getResponse().getResponseBuffer().data();
		size_t		len = client.getResponse().getResponseBuffer().size();
		ssize_t		sent;
		size_t		totalSent = 0;

		for (int retry = 3; retry > 0; --retry) {
			sent = send(clientFd, buff + totalSent, len - totalSent, _ioFlags);
			if (sent > 0)
				totalSent += sent;
			if (totalSent == len)
				return (true);
			if (sent < 0 || totalSent != len) {
				if (sent < 0)
					LOG_ERROR("Could not send response to client (fd="
							<< clientFd << "): " << strerror(errno));
				ft_sleep(0.1);
				continue ;
			}
		}
		if (client.hasError())
			return (false);
		client.prepareErrorResponse(500);
		return (_sendResponse(client, clientFd));
	}

	bool	Webserv::_handleClientResponse(Client& client, pollFd_iter pollFd)
	{
		// TO DO: 1) Some errorCodes (like 400) go with a "Connection: close",
		// 		  meaning that the connection won't be kept alive
		// 		  2) At the project's end, implement keepalive with its
		// 		  parameters "timeout"/"max"

		if ((pollFd->revents & POLLOUT) != 0 && client.hasResponseReady()) {
			if (!_sendResponse(client, client.getSocket().getFd()))
				return (true);
			if (client.getResponse().isChunkedResponse())
				client.prepareResponse();
			else
				client.clearResponse();
		}
		if (!client.isProcessingRequest() && !client.isKeepAlive())
			return (true);
		return (false);
	}

	void	Webserv::_handleClients()
	{
		std::list<Client>::iterator				client = _clients.begin();
		size_t									i = _servers.size();
		std::vector<struct pollfd>::iterator	pollFd;
		ssize_t									received;

		while (client != _clients.end()) {
			pollFd = _findPollFd(client->getSocket().getFd(), i, CLIENT);
			if (pollFd != _pollFds.end()) {
				_showOtherRevents(pollFd, CLIENT);
				if ((pollFd->revents & POLLIN) != 0) {
					received = _receiveClientRequest(*client);
					if (received == 0) {
						client = _removeClient(client, pollFd);
						continue ;
					} else if (received > 0)
						_bufferAndSendMsg(*client);	// tmp exam version
//						_handleClientRequest(*client);
				}
				if (_handleClientResponse(*client, pollFd)) {
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
			if (poll(_pollFds.data(), _pollFds.size(), -1) == -1) {
				LOG_WARN("poll() error: " << strerror(errno));
				continue ;
			}
			_acceptConnections();
			_handleClients();
		}
	}

}	// namespace webserv
