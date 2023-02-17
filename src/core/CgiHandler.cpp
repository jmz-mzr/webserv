#include "core/CgiHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <libgen.h>

#include <cstring>
#include <cerrno>
#include <cctype>

#include <sstream>

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
										_childPid(-1)
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
					<< errorType << " (" << errno << ": " << strerror(errno)
					<< "), " << debugInfos.str());
		} else
			LOG_ERROR(errorAt << space1 << "\"" << filename << "\""
					<< space2 << errorType << ", " << debugInfos.str());
	}
	catch (const std::exception& e) {
		LOG_ERROR("Logging error: " << e.what());
	}

	void	CgiHandler::_loadEnvContainers()
	{
		std::map<std::string, std::string>::const_iterator	var;
		std::string											envVar;
		std::list<std::string>::const_iterator				envIt;

		_envp.reserve(_envMap.size());
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

	void	CgiHandler::_loadEnv2(const Request& request)
	{
		Request::header_map					headers = request.getHeaders();
		Request::header_map::const_iterator	it = headers.begin();
		std::string							fieldName;
		size_t								found;

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
		found = _requestedFilename.find_last_of('/');
		found += (found == 0);
		_envMap["DOCUMENT_ROOT"] = _requestedFilename.substr(0, found);
		return (_loadEnvContainers());
	}

	void	CgiHandler::loadEnv(const Request& request,
								const std::string& computedContentType)
	{
		//TO DO: Add "remote_ident/user" for sessions?
		//		 And HTTP_COOKIE?

		if (request.getRequestMethod() == "POST")
			_envMap["CONTENT_LENGTH"] = to_string(request.getBodySize());
		if (!request.getContentType().empty())
			_envMap["CONTENT_TYPE"] = request.getContentType();
		else if (request.getRequestMethod() == "POST")
			_envMap["CONTENT_TYPE"] = computedContentType;
		_envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
		_envMap["PATH_INFO"] = _requestedFilename;
		_envMap["PATH_TRANSLATED"] = _requestedFilename;
		_envMap["QUERY_STRING"] = request.getQuery();
		_envMap["REDIRECT_STATUS"] = "200";
		_envMap["REMOTE_ADDR"] = request.getClientSocket().getIpAddr();
		_envMap["REQUEST_METHOD"] = request.getRequestMethod();
		_envMap["REQUEST_URI"] = request.getUri();
		_envMap["SCRIPT_NAME"] = _cgiPass;
		_envMap["SCRIPT_FILENAME"] = _cgiPass;
		_envMap["SERVER_ADDR"] = ft_inet_ntoa(request.
				getServerConfig()->getListenPair().sin_addr);
		_envMap["SERVER_NAME"] = request.getServerName();
		_envMap["SERVER_PORT"] = to_string(request.getClientSocket().getPort());
		_envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
		_envMap["SERVER_SOFTWARE"] = "webserv";
		return (_loadEnv2(request));
	}

	void	CgiHandler::_closeCgiFiles()
	{
		if (_inputFd >= 0) {
			if (close(_inputFd) < 0) {
				LOG_ERROR("Bad fclose() on cgiInputFd");
			} else
				_inputFd = -1;
		}
		if (_outputFd >= 0) {
			if (close(_outputFd) < 0) {
				LOG_ERROR("Bad fclose() on cgiOutputFd");
			} else
				_outputFd = -1;
		}
		if (_outputFile) {
			if (std::fclose(_outputFile) != 0) {
				LOG_ERROR("Bad fclose() on temporary cgiOutputFile");
				std::clearerr(_outputFile);
			} else
				_outputFile = 0;
		}
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
		LOG_DEBUG("CGI tmp output file opened (fd=" << _outputFd << ")");
		return (true);
	}

	void	CgiHandler::_executeCgi(const Request& request,
									const char* workingDir)
	{
		fclose(_outputFile);
		if (request.getRequestMethod() == "POST") {
			if (dup2(_inputFd, STDIN_FILENO) < 0) {
				close(_inputFd);
				close(_outputFd);
				exit(DUP2_ERROR);
			}
		}
		if (dup2(_outputFd, STDOUT_FILENO) < 0) {
			close(_inputFd);
			close(_outputFd);
			exit(DUP2_ERROR);
		}
		if (chdir(workingDir) < 0) {
			close(_inputFd);
			close(_outputFd);
			exit(CHDIR_ERROR);
		}
		close(_outputFd);
		execve(_argv[0], _argv, _envp.data());
		exit(EXECVE_ERROR);
	}

	int	CgiHandler::_waitChild(const Request& request, int* status) const
	{
		int		pid = waitpid(_childPid, status, WNOHANG);

		if (pid == 0) {
			sleep(1);
			if ((pid = waitpid(_childPid, status, WNOHANG)) == 0) {
				if (kill(_childPid, SIGTERM) < 0)
					_logError(request, "Error with", "in CGI wait", "kill()");
				usleep(10000);
				if ((pid = waitpid(_childPid, status, WNOHANG)) == 0) {
					if (kill(_childPid, SIGKILL) < 0)
						_logError(request, "Error with", "in CGI wait",
								"kill()");
				}
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
			if (WEXITSTATUS(status) == DUP2_ERROR)
				_logError(request, "Error in", "before CGI launch", "dup2()");
			else if (WEXITSTATUS(status) == CHDIR_ERROR)
				_logError(request, "Error in", "before CGI launch", "chdir()");
			else if (WEXITSTATUS(status) == EXECVE_ERROR)
				_logError(request, "Error in", "before CGI launch", "execve()");
			else
				_logError(request, "The CGI process failed while serving", "");
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
		std::string	filePath(_requestedFilename);
		char*		workingDir = dirname(const_cast<char*>(filePath.c_str()));

		_childPid = fork();
		if (_childPid < 0) {
			_logError(request, "Error with", "while loading the CGI", "fork()");
			_closeCgiFiles();
			return (500);
		} else if (_childPid == 0)
			_executeCgi(request, workingDir);
		LOG_DEBUG("CGI launch (pid=" << _childPid << ")");
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
