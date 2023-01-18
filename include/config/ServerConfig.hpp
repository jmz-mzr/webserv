#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <map>
# include <set>
# include <string>
# include <utility>
# include <vector>

# include <stdint.h>

# include "config/Config.hpp"
# include "config/Location.hpp"
# include "core/Socket.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	Location;

	class	ServerConfig {
	public:
		typedef std::map<int, std::string>					error_page_map;
		typedef std::map<std::string, Location,
							location_compare>				location_map;
		typedef std::set<std::string, strcmp_icase>			hostname_set;

		ServerConfig(const Config& src, const sockaddr_in& listenPair);
		~ServerConfig() { }

		const sockaddr_in&				getListenPair() const
													{ return (_listenPair); }
		const hostname_set&				getServerNames() const
													{ return (_serverNames); }
		const error_page_map&			getErrorPages() const
													{ return (_errorPages); }
		const int64_t&					getMaxBodySize() const
													{ return (_maxBodySize); }
		const location_map&				getLocations() const
													{ return (_locations); }

		bool	eraseName(const std::string& name);
		void	addName(const std::string& name);

	private:
		ServerConfig();

		ServerConfig&	operator=(const ServerConfig& rhs);

		sockaddr_in					_listenPair;
		hostname_set				_serverNames;
		error_page_map				_errorPages;
		int64_t					_maxBodySize;
		location_map				_locations;
	};

}	// namespace webserv

#endif	// SERVERCONFIG_HPP
