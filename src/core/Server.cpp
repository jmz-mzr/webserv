#include "core/Server.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Server::Server(const ServerConfig& serverConfig,
					const listen_pair& listenPair):
								_socket(listenPair.first, listenPair.second),
								_configs(1, serverConfig)
	{
		LOG_INFO("New Server instance");
		LOG_DEBUG("fd=" << _socket.getFd() << " ; "
				<< "addr=" << listenPair.first << " ; "
				<< "port=" << listenPair.second);
	}

	Server::Server(const Server& src): _socket(src._socket),
										_configs(src._configs)
	{
		LOG_INFO("Server copied");
		LOG_DEBUG("fd=" << _socket.getFd() << " ; "
				<< "addr=" << _socket.getIpAddr() << " ; "
				<< "port=" << _socket.getPort());
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const ListenSocket&	Server::getSocket() const
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

}	// namespace webserv
