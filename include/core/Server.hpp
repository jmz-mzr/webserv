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
		typedef std::pair<const std::string, uint16_t>	listen_pair;

		Server(const ServerConfig& serverConfig, const listen_pair& listenPair);
		Server(const Server& src);
		~Server() { }

		const ListenSocket&					getSocket() const;
		const std::vector<ServerConfig>&	getConfigs() const;

		void	addConfig(const ServerConfig& serverConfig);

	private:
		Server();

		Server&	operator=(const Server& rhs);

		ListenSocket				_socket;
		std::vector<ServerConfig>	_configs;
	};

}	// namespace webserv

#endif	// SERVER_HPP
