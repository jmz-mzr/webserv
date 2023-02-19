#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "core/Request.hpp"

# include <unistd.h>

# include <string>
# include <list>
# include <vector>
# include <cstdio>
# include <map>

namespace	webserv
{

	class	CgiHandler {
	public:
		CgiHandler(const std::string& cgiPass,
					const std::string& requestedFilename);
		~CgiHandler();

		void	loadEnv(const Request& request,
						const std::string& computedContentType);
		bool	prepareCgiIo(const Request& request);
		int		launchCgiProcess(const Request& request);

		FILE*	getOutputFile() { return (_outputFile); }

		int				responseCode;
		std::string		headers;
		bool			hasReadHeaders;
		int				nbHeaders;
		bool			hasBody;
	private:
		typedef std::map<std::string, std::string>	_env_map;

		CgiHandler(const CgiHandler& src);

		CgiHandler&	operator=(const CgiHandler& rhs);

		void	_logError(const Request& request, const char* errorAt,
						const char* errorType, const char* filename = 0) const;

		void		_loadEnv2(const Request& request);
		std::string	_convertEnvVarName(const std::string& str) const;
		void		_loadEnvContainers();
		void		_executeCgi(const Request& request, const char* workingDir);
		int			_getChildStatus(const Request& request) const;
		int			_waitChild(const Request& request, int* status) const;
		void		_closeCgiFiles();

		const std::string&		_cgiPass;
		const std::string&		_requestedFilename;
		char*					_argv[3];
		_env_map				_envMap;
		std::list<std::string>	_env;
		std::vector<char*>		_envp;
		int						_inputFd;
		FILE*					_outputFile;
		int						_outputFd;
		pid_t					_childPid;
	};

}	// namespace webserv

#endif	// CGIHANDLER_HPP