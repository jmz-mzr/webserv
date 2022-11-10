#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <fstream>
# include <vector>
# include <exception>

# include "config/ServerConfig.hpp"

namespace	webserv
{

	class	Config {
	public:
		Config();
		~Config() { closeConfigFile(); }

		class FatalErrorException: public std::exception {
		public:
			FatalErrorException(const char* msg = "A fatal error occured"):
																_msg(msg) { }
			virtual const char*	what() const throw() { return (_msg); }
		private:
			const char*	_msg;
		};

		void		openConfigFile(const std::string& configFileName);
		void		parseConfig();
		void		closeConfigFile();

		const std::ifstream&				getConfigFile() const
													{ return (_configFile); }
		const std::vector<ServerConfig>&	getServerConfigs() const
													{ return (_serverConfigs); }
	private:
		Config(const Config& src);

		Config&	operator=(const Config& rhs);

		std::string					_configFileName;
		std::ifstream				_configFile;
		std::vector<ServerConfig>	_serverConfigs;
	};

}	// namespace webserv

#endif	// CONFIG_HPP
