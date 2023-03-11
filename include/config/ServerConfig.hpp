#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <netinet/in.h>	// sockaddr_in
# include <stdint.h>		// int64_t

# include <map>
# include <set>
# include <string>
# include <utility>			// pair
# include <vector>

# include "config/Config.hpp"
# include "config/Location.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	Location;

	class	ServerConfig {
	public:
		typedef std::map<int, std::string>					error_page_map;
		typedef std::pair<const int, std::string>			return_pair;
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
		const return_pair&				getReturnPair() const
													{ return (_returnPair); }
		const std::string&				getRoot() const
													{ return (_root); }
		bool							isAutoIndex() const
													{ return (_autoIndex); }
		const std::string&				getIndex() const
													{ return (_index); }
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
		int64_t						_maxBodySize;
		return_pair					_returnPair;
		std::string					_root;
		bool						_autoIndex;
		std::string					_index;
		location_map				_locations;
	};

}	// namespace webserv

#endif	// SERVERCONFIG_HPP
