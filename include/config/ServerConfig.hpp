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
		ServerConfig&	operator=(const ServerConfig& rhs);

		// TO DO: We must resolve the eventual localhost address
		// with /etc/host and "*" (or no address) with "0.0.0.0"
		std::vector<listen_pair>	_listenPairs;
		std::vector<std::string>	_names;
//		...
	};

}	// namespace webserv

#endif	// SERVERCONFIG_HPP
