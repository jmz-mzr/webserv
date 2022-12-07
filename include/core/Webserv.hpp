#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "webserv_config.hpp"
# include "config/ConfigParser.hpp"
# include "config/ServerConfig.hpp"
# include "core/Server.hpp"
# include "core/Client.hpp"

# include <utility>
# include <string>
# include <vector>
# include <list>
# include <csignal>

# include <stdint.h>
# include <stddef.h>
# include <poll.h>

namespace	webserv
{

	class	Webserv {
	public:
		Webserv();
		~Webserv();

		void	init(int argc, char** argv);
		void	run();

		volatile static std::sig_atomic_t	receivedSigInt;
	private:
		typedef std::vector<Server>::iterator				server_iter;
		typedef std::vector<struct pollfd>::iterator		pollFd_iter;
		typedef std::list<Client>::iterator					client_iter;

		Webserv(const Webserv& src);

		Webserv&	operator=(const Webserv& rhs);

		void		_addServer(const Config& serverConfig);
		server_iter	_findSameSocket(const sockaddr_in& listenPair);
		bool		_sameSocket(const sockaddr_in& listenPair,
								const Server& server) const;
		void		_deleteRedundantNames(ServerConfig& serverConfig,
											const Server& server) const;
		void		_loadServers(const std::list<Config>& config);
		void		_acceptConnections();
		pollFd_iter	_findPollFd(const int fdToFind, const size_t hint,
											const std::string& object);
		void		_showOtherRevents(pollFd_iter pollFd,
										const std::string& object) const;
		void		_addClient(const int serverFd, const Server& server);
		client_iter	_removeClient(client_iter client, pollFd_iter pollFd);
		void		_broadcastMsg(const std::string& msg,
									const int senderId) const;	// tmp exam version
		void		_handleClients();
		ssize_t		_receiveClientRequest(Client& client, pollFd_iter pollFd);
		void		_bufferAndSendMsg(Client& client);	// tmp exam version
		int			_extractMsg(std::string& buffer, std::string& msg);	// tmp exam version
		void		_handleClientRequest(Client& client);
		bool		_handleClientResponse(Client& client, pollFd_iter pollFd);
		bool		_sendResponse(Client& client, int clientFd);
		void		_usageHelper();
		void		_parseConfig(std::string configFilePath);

		char						_buffer[RECV_BUFFER_SIZE];
		std::vector<Server>			_servers;
		std::list<Client>			_clients;
		std::vector<struct pollfd>	_pollFds;
		int							_ioFlags;
	};

}	// namespace webserv

#endif	// WEBSERV_HPP
