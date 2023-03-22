#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <netinet/in.h>	// sockaddr_in
# include <poll.h>			// struct pollfd
# include <stddef.h>		// size_t
# include <sys/types.h>		// ssize_t

# include <csignal>			// sig_atomic_t

# include <list>
# include <string>
# include <vector>

# include "webserv_config.hpp"
# include "config/Config.hpp"
# include "config/ServerConfig.hpp"
# include "core/Server.hpp"
# include "core/Client.hpp"
# include "utils/print_config_tree.hpp"

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
										const sockaddr_in& serverAddr) const;
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
		void		_handleClients();
		ssize_t		_receiveClientRequest(Client& client, pollFd_iter pollFd);
		bool		_handleClientRequest(Client& client);
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
