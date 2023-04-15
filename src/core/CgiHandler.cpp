#include <fcntl.h>		// open
#include <libgen.h>		// dirname
#include <signal.h>		// kill
#include <stddef.h>		// size_t
#include <stdio.h>		// fileno
#include <stdlib.h>		// exit, getenv
#include <sys/wait.h>	// waitpid
#include <unistd.h>		// close, chdir, dup2, execve, fork, (u)sleep,
						// STD(IN/OUT/ERR)_FILENO

#include <cctype>		// toupper
#include <cerrno>		// errno
#include <cstring>		// strerror
#include <cstdio>		// clearerr, fclose, fgets, fprintf, rewind

#include <list>
#include <map>
#include <string>
#include <sstream>

#include "core/CgiHandler.hpp"
#include "webserv_config.hpp"
#include "utils/global_defs.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

#define DUP2_ERROR			4
#define CHDIR_ERROR			8
#define EXECVE_ERROR		12

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	CgiHandler::CgiHandler(const std::string& cgiPass,
							const std::string& requestedFilename):
										responseCode(0),
										hasReadHeaders(false),
										nbHeaders(0),
										hasBody(false),
										_cgiPass(cgiPass),
										_requestedFilename(requestedFilename),
										_inputFd(-1),
										_outputFile(0),
										_outputFd(-1),
										_errorFile(0),
										_pid(-1)
	{
		LOG_INFO("New CgiHandler instance");
		LOG_DEBUG("cgiPass=" << cgiPass << " ; "
				<< "requestedFilename=" << requestedFilename);
		_argv[0] = const_cast<char*>(_cgiPass.c_str());
		_argv[1] = const_cast<char*>(_requestedFilename.c_str());
		_argv[2] = 0;
	}

	CgiHandler::~CgiHandler()
	{
		_closeCgiFiles();
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	void	CgiHandler::_logError(const Request& request,
									const char* errorAt,
									const char* errorType,
									const char* filename) const try
	{
		std::ostringstream	debugInfos;
		std::string			space1((errorAt[0] != '\0'), ' ');
		std::string			space2((errorType[0] != '\0'), ' ');

		debugInfos << "client: " << request.getClientSocket().getIpAddr()
			<< ", server: " << request.getServerName()
			<< ", request: \"" << request.getRequestLine()
			<< "\", host: \"" << request.getHost() << "\"";
		if (!filename)
			filename = _requestedFilename.c_str();
		if (errorAt[0] != '\0' && errorType[0] != '\0') {
			LOG_ERROR(errorAt << space1 << "\"" << filename << "\" "
					<< errorType << " (" << errno << ": "
					<< std::strerror(errno) << "), " << debugInfos.str());
		} else
			LOG_ERROR(errorAt << space1 << "\"" << filename << "\""
					<< space2 << errorType << ", " << debugInfos.str());
	}
	catch (const std::exception& e) {
		LOG_ERROR("Logging error: " << e.what());
	}

	void	CgiHandler::_loadCustomEnvVars()
	{
		std::string									envVar;
		std::list<std::string>::const_iterator		envIt;

		envVar = std::string("PATH=") + getenv("PATH");
		LOG_DEBUG("CGI variable: " << envVar);
		envIt = _env.insert(_env.end(), envVar);
		_envp.push_back(const_cast<char*>(envIt->c_str()));
		envVar = std::string("CGI_SESSION=") + XSTR(CGI_SESSION);
		LOG_DEBUG("CGI variable: " << envVar);
		envIt = _env.insert(_env.end(), envVar);
		_envp.push_back(const_cast<char*>(envIt->c_str()));
		envVar = std::string("LIB_PERL_CGI=") + XSTR(LIB_PERL_CGI);
		LOG_DEBUG("CGI variable: " << envVar);
		envIt = _env.insert(_env.end(), envVar);
		_envp.push_back(const_cast<char*>(envIt->c_str()));
	}

	void	CgiHandler::_loadEnvContainers()
	{
		std::map<std::string, std::string>::const_iterator	var;
		std::string											envVar;
		std::list<std::string>::const_iterator				envIt;

		_envp.reserve(_envMap.size() + 4);
		_loadCustomEnvVars();
		var = _envMap.begin();
		while (var != _envMap.end()) {
			envVar = var->first + "=" + var->second;
			LOG_DEBUG("CGI meta-variable: " << envVar);
			envIt = _env.insert(_env.end(), envVar);
			_envp.push_back(const_cast<char*>(envIt->c_str()));
			++var;
		}
		_envp.push_back(0);
		_envMap.clear();
	}

	std::string	CgiHandler::_convertEnvVarName(const std::string& str) const
	{
		std::string		name;

		for (std::string::const_iterator c = str.begin(); c != str.end(); ++c) {
			if (*c == '-')
				name += '_';
			else
				name += std::toupper(*c);
		}
		return (name);
	}

	void	CgiHandler::_loadEnv3(const Request& request)
	{
		Request::header_map					headers = request.getHeaders();
		Request::header_map::const_iterator	it = headers.begin();
		std::string							fieldName;

		while (it != headers.end()) {
			if (!ft_strcmp_icase(it->first, "Content-Length")
					&& !ft_strcmp_icase(it->first, "Content-Type")
					&& !ft_strcmp_icase(it->first, "Authorization")
					&& !ft_strcmp_icase(it->first, "Proxy-Authorization")) {
				fieldName = "HTTP_";
				fieldName += _convertEnvVarName(it->first);
				_envMap[fieldName] = it->second;
			}
			++it;
		}
		return (_loadEnvContainers());
	}

	void	CgiHandler::_loadEnv2(const Request& request)
	{
		size_t					i = request.getUri().find_first_of('.');

		while (i != std::string::npos
				&& std::isalnum(request.getUri().c_str()[i + 1]))
			++i;
		i += (i != std::string::npos);
		_envMap["SCRIPT_NAME"] = request.getUri().substr(0, i);
		_envMap["SCRIPT_FILENAME"] = _cgiPass;
		_envMap["DOCUMENT_ROOT"] = std::string(XSTR(WEBSERV_ROOT)) + "/";
		if (request.getLocation()->getAlias().c_str()[0] == '/'
				|| (request.getLocation()->getAlias().empty()
					&& request.getLocation()->getRoot().c_str()[0] == '/'))
			_envMap["DOCUMENT_ROOT"] = "";
		if (!request.getLocation()->getAlias().empty())
			_envMap["DOCUMENT_ROOT"] += request.getLocation()->getAlias();
		else
			_envMap["DOCUMENT_ROOT"] += request.getLocation()->getRoot();
		_envMap["PHP_SELF"] = request.getUri();
		_envMap["REQUEST_URI"] = request.getUri();
		if (!request.getQuery().empty())
			_envMap["REQUEST_URI"] += std::string("?") + request.getQuery();
		_envMap["QUERY_STRING"] = request.getQuery();
		return (_loadEnv3(request));
	}

	void	CgiHandler::loadEnv(const Request& request,
								const std::string& computedContentType)
	{
		const std::string&	contentType = request.getContentType();
		const std::string&	contentLength = (request.getContentLength() >= 0 ?
									to_string(request.getContentLength()) : "");

		_envMap["CONTENT_LENGTH"] = contentLength;
		_envMap["CONTENT_TYPE"] = contentType;
		if (contentType.empty() && request.getRequestMethod() == "POST")
			_envMap["CONTENT_TYPE"] = computedContentType;
		_envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
		_envMap["PATH_INFO"] = request.getUri();
		_envMap["PATH_TRANSLATED"] = _requestedFilename;
		_envMap["REDIRECT_STATUS"] = "200";
		_envMap["REMOTE_ADDR"] = request.getClientSocket().getIpAddr();
		_envMap["REQUEST_METHOD"] = request.getRequestMethod();
		_envMap["SERVER_ADDR"] = ft_inet_ntoa(request.
				getServerConfig()->getListenPair().sin_addr);
		_envMap["SERVER_NAME"] = request.getServerName();
		_envMap["SERVER_PORT"] = to_string(request.getClientSocket().getPort());
		_envMap["LISTEN_PORT"] = to_string(ntohs(request.getServerConfig()
												->getListenPair().sin_port));
		_envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
		_envMap["SERVER_SOFTWARE"] = "webserv";
		return (_loadEnv2(request));
	}

	void	CgiHandler::_closeCgiFiles()
	{
		if (_inputFd >= 0) {
			if (close(_inputFd) < 0) {
				LOG_ERROR("Bad close() on cgiInputFd");
			} else
				_inputFd = -1;
		}
		if (_outputFile) {
			if (std::fclose(_outputFile) != 0) {
				LOG_ERROR("Bad fclose() on temporary cgiOutputFile");
				std::clearerr(_outputFile);
			} else {
				_outputFd = -1;
				_outputFile = 0;
			}
		}
		if (_errorFile) {
			if (std::fclose(_errorFile) != 0) {
				LOG_ERROR("Bad fclose() on temporary cgiErrorFile");
				std::clearerr(_errorFile);
			} else
				_errorFile = 0;
		}
	}

	bool	CgiHandler::_prepareCgiIoFiles(const Request& request)
	{
		_outputFile = std::tmpfile();
		if (!_outputFile) {
			_logError(request, "tmpfile()", "failed", "(cgiOutputFile)");
			return (false);
		}
		_outputFd = fileno(_outputFile);
		if (_outputFd < 0) {
			_logError(request, "Error with", "while preparing cgi", "fileno()");
			_closeCgiFiles();
			return (false);
		}
		_errorFile = std::tmpfile();
		if (!_errorFile) {
			_logError(request, "tmpfile()", "failed", "(cgiErrorFile)");
			_closeCgiFiles();
			return (false);
		}
		LOG_DEBUG("CGI tmp output file opened (fd=" << _outputFd << ")");
		return (true);
	}

	bool	CgiHandler::prepareCgiIo(const Request& request)
	{
		if (request.getRequestMethod() == "POST") {
			_inputFd = open(request.getTmpFilename().c_str(),
					O_RDONLY | O_NONBLOCK | O_CLOEXEC);
			if (_inputFd < 0) {
				_logError(request, "open()", "failed",
						request.getTmpFilename().c_str());
				return (false);
			}
			LOG_DEBUG("CGI input file opened: " << request.getTmpFilename());
		}
		if (!_prepareCgiIoFiles(request))
			return (false);
		return (true);
	}

	void	CgiHandler::_handleCgiChildError(bool beforeExecve)
	{
		// NOTE: 1) Prevent any leaks in the child process when an error occurs
		// before the CGI launch by creating a time out with "sleep(4)" to
		// let the parent process gracefully kill the child
		// 2) To have details on the error if execve crashes, comment out the
		// line "close(fileno(_errorFile))" in "_executeCgi()"

		std::fprintf(_errorFile, "(%d: %s)", errno, std::strerror(errno));
		if (beforeExecve) {
			close(_inputFd);
			close(_outputFd);
			sleep(4);
		}
	}

	std::string	CgiHandler::_loadWorkingDir() const
	{
		struct stat		fileInfos;
		std::string		filePath(_requestedFilename);

		std::memset(&fileInfos, 0, sizeof(fileInfos));
		if (stat(filePath.c_str(), &fileInfos) < 0
				|| !S_ISDIR(fileInfos.st_mode))
			return (dirname(const_cast<char*>(filePath.c_str())));
		if (*filePath.rbegin() != '/')
			filePath += "/.";
		return (dirname(const_cast<char*>(filePath.c_str())));
	}

	void	CgiHandler::_executeCgi(const Request& request,
									const char* workingDir)
	{
		if (request.getRequestMethod() == "POST") {
			if (dup2(_inputFd, STDIN_FILENO) < 0) {
				_handleCgiChildError();
				exit(DUP2_ERROR);
			}
		}
		if (dup2(_outputFd, STDOUT_FILENO) < 0) {
			_handleCgiChildError();
			exit(DUP2_ERROR);
		}
		if (chdir(workingDir) < 0) {
			_handleCgiChildError();
			exit(CHDIR_ERROR);
		}
		close(_outputFd);
		close(fileno(_errorFile));
		const_cast<AcceptSocket&>(request.getClientSocket()).closeFd();
		CLOSE_LOG_FILE();
		execve(_argv[0], _argv, _envp.data());
		_handleCgiChildError(false);
		exit(EXECVE_ERROR);
	}

	int	CgiHandler::_waitChild(const Request& request, int* status) const
	{
		// NOTE: To let valgrind or another tool run through the CGI process
		// without time out, [ft_u]sleep must be incremented (minimum 3s)

		int		pid;
		int		retry = 20;

		while ((pid = waitpid(_pid, status, WNOHANG)) == 0 && --retry > 0)
			usleep(50000);
		if (pid == 0) {
			ft_usleep(2, 200000);
			if ((pid = waitpid(_pid, status, WNOHANG)) == 0) {
				if (kill(_pid, SIGTERM) < 0)
					_logError(request, "Error with", "in CGI wait", "kill()");
				usleep(10000);
				if ((pid = waitpid(_pid, status, WNOHANG)) == 0)
					if (kill(_pid, SIGKILL) < 0)
						_logError(request, "Error with", "in CGI wait",
								"kill()");
			}
		}
		if (pid < 0) {
			_logError(request, "Error with", "after fork()", "waitpid()");
			return (500);
		} else if (pid == 0) {
			_logError(request, "The CGI timed out while requesting", "");
			return (504);
		}
		return (0);
	}

	void	CgiHandler::_loadChildErrorLog(const Request& request,
											int status) const
	{
		char	errorStr[256];
		char*	resultPtr;

		std::rewind(_errorFile);
		resultPtr = std::fgets(errorStr, 256, _errorFile);
		if (!resultPtr)
			errorStr[0] = '\0';
		if (WEXITSTATUS(status) == DUP2_ERROR)
			_logError(request, "Error in dup2() before CGI launch",
					"", errorStr);
		else if (WEXITSTATUS(status) == CHDIR_ERROR)
			_logError(request, "Error in chdir() before CGI launch",
					"", errorStr);
		else if (WEXITSTATUS(status) == EXECVE_ERROR)
			_logError(request, "Error in execve() before CGI launch",
					"", errorStr);
		else
			_logError(request, "The CGI process failed while serving", "");
	}

	int	CgiHandler::_getChildStatus(const Request& request) const
	{
		int		status;
		int		errorCode;

		if ((errorCode = _waitChild(request, &status)) != 0)
			return (errorCode);
		if (!WIFEXITED(status)) {
			_logError(request, "The CGI ended abnormally while requesting", "");
			return (502);
		}
		LOG_DEBUG("CGI exited with code: " << WEXITSTATUS(status));
		if (WEXITSTATUS(status) != 0) {
			_loadChildErrorLog(request, status);
			if (WEXITSTATUS(status) == DUP2_ERROR || WEXITSTATUS(status)
					== CHDIR_ERROR || WEXITSTATUS(status) == EXECVE_ERROR)
				return (500);
			return (502);
		}
		return (0);
	}

	int	CgiHandler::launchCgiProcess(const Request& request)
	{
		int			errorCode;
		std::string	workingDir = _loadWorkingDir();

		_pid = fork();
		if (_pid < 0) {
			_logError(request, "Error with", "while loading the CGI", "fork()");
			_closeCgiFiles();
			return (500);
		} else if (_pid == 0)
			_executeCgi(request, workingDir.c_str());
		LOG_DEBUG("CGI launch (pid=" << _pid << ")");
		_env.clear();
		_envp.clear();
		errorCode = _getChildStatus(request);
		if (errorCode) {
			_closeCgiFiles();
			return (errorCode);
		}
		return (0);
	}

}	// namespace webserv
