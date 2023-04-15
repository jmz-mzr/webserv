#include <iostream>

#include "core/Server.hpp"
#include "utils/log.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Server::Server(const ServerConfig& serverConfig,
					const sockaddr_in& listenPair)
			: _socket(listenPair)
			, _configs(1, serverConfig)
	{ }

	Server::Server(const Server& src): _socket(src._socket),
										_configs(src._configs)
	{ }

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const ListenSocket&		Server::getSocket() const
	{
		return (_socket);
	}

	const std::vector<ServerConfig>&	Server::getConfigs() const
	{
		return (_configs);
	}

	void	Server::addConfig(const ServerConfig& serverConfig)
	{
		_configs.push_back(serverConfig);
		LOG_INFO("ServerConfig added to a Server");
		LOG_DEBUG("fd=" << _socket.getFd() << " ; "
			<< "addr=" << _socket.getIpAddr() << " ; "
			<< "port=" << _socket.getPort());
	}

	std::ostream&	operator<<(std::ostream& os, const Socket& socket)
	{
		os << "fd=" << socket.getFd() << " ; "
			<< "addr=" << socket.getIpAddr() << " ; "
			<< "port=" << socket.getPort();
		return (os);
	}

}	// namespace webserv
