#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <utility>
# include <string>
# include <vector>

# include <stdint.h>

namespace	webserv
{

	class	ServerConfig {
	public:
		typedef std::pair<const std::string, uint16_t>	listen_pair;

		ServerConfig();
		ServerConfig(const ServerConfig& src);
		~ServerConfig() { }

		ServerConfig&	operator=(const ServerConfig& rhs);

		const std::vector<listen_pair>&	getListenPairs() const
													{ return (_listenPairs); }
		const std::vector<std::string>&	getNames() const
													{ return (_names); }

		bool	empty() { return (_listenPairs.empty()); }

		void	clearListenPairs() { _listenPairs.clear(); }
		bool	eraseName(const std::string& name);
		void	clearConfig();

		void	addListenPair(const listen_pair& listenPair);
		void	addName(const std::string& name);
	private:
		// TO DO: 1) We must resolve the eventual localhost address
		// (case-insensitively: it must work with "LOCalhOST:80")
		// with /etc/host and "*" (or no address) with "0.0.0.0"
		// 2) We must add port 80 if no port is given
		// 3) After testing with NGINX, if no listen directive is
		// given, we must silently ignore the current server config
		std::vector<listen_pair>	_listenPairs;
		// TO DO: 1) The names are case-insensitive, so it would be better
		// to record them in lowercase with str_tolower
		// 2) An explicit "" name is a valid empty name that must be added
		// to the vector as a empty string
		// 3) If no server_name is given, we must add an empty string name
		// to the vector, so the vector.size() cannot be 0
		// 4) An IP address is a valid name
		std::vector<std::string>	_names;
//		...
	};

}	// namespace webserv

#endif	// SERVERCONFIG_HPP
