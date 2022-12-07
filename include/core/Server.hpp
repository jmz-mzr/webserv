#ifndef SERVER_HPP
# define SERVER_HPP

# include <utility>
# include <string>
# include <vector>

# include <stdint.h>

# include "config/ServerConfig.hpp"
# include "core/ListenSocket.hpp"

namespace	webserv
{
	class	Server {
	public:
		Server(const ServerConfig& serverConfig, const Address& listenPair);
		Server(const Server& src);
		~Server() { }

		const ListenSocket&					getSocket() const;
		const std::vector<ServerConfig>&	getConfigs() const;

		void	addConfig(const ServerConfig& serverConfig);

		void	closeSocket() { _socket.closeFd(); }

	private:
		Server();

		Server&	operator=(const Server& rhs);

		ListenSocket					_socket;
		std::vector<ServerConfig>		_configs;
	};

}	// namespace webserv

#endif	// SERVER_HPP
