#include "core/Response.hpp"

#include <sstream>
#include <ctime>
#include <utility>

#include "utils/global_defs.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Response::Response(): _responseCode(0),
							_contentType("application/octet-stream"),
							_contentLength(0),
							_isKeepAlive(true),
							_isChunkedResponse(false),
							_isResponseReady(false)
	{
		LOG_INFO("New Response instance");
	}

	Response::Response(const Response& src):
									_responseBuffer(src._responseBuffer),
									_responseCode(src._responseCode),
									_contentType(src._contentType),
									_contentLength(src._contentLength),
									_isKeepAlive(src._isKeepAlive),
									_location(src._location),
									_isChunkedResponse(src._isChunkedResponse),
									_isResponseReady(src._isResponseReady)
	{
		// TO DO: handle swap of std::ifstream (or other _requestedFile object)
		// 		  or not if Response in only copied at Client creation?

		LOG_INFO("Response copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const std::string&	Response::getResponseBuffer() const
	{
		return (_responseBuffer);
	}

	void	Response::setResponseCode(int responseCode)
	{
		_responseCode = responseCode;
	}

	bool	Response::isKeepAlive() const
	{
		if (!_isKeepAlive)
			return (false);
		if (_responseCode == 400 || _responseCode == 408 || _responseCode == 413
				|| _responseCode == 414 || _responseCode == 495
				|| _responseCode == 496 || _responseCode == 497
				|| _responseCode == 500 || _responseCode == 501)
			return (false);
		return (true);
	}

	const std::string	Response::_loadLocation() const
	{
		std::string		location;

		if (_responseCode == 301 || _responseCode == 302 || _responseCode == 303
				|| _responseCode == 307 || _responseCode == 308) {
			location = "Location: ";
			location += _location;
		}
		return (location);
	}

	void	Response::_loadHeaders()
	{
		const char*			connection = _isKeepAlive ? "keep-alive" : "close";
		std::ostringstream	headers;

		headers << "HTTP/1.1 " << _responseCode
			<< Response::_getResponseStatus(_responseCode) << CRLF
			<< "Server: webserv" << CRLF
			<< "Date: " << Response::_getDate() << CRLF
			<< "Content-Type: " << _contentType << CRLF
			<< "Content-Length: " << _contentLength << CRLF
			<< _loadLocation() << CRLF
			// load Transfer-Encoding
			<< "Connection: " << connection << CRLF
			// load ETag
			<< CRLF;
		_responseBuffer = headers.str();
	}

	void	Response::_prepareChunkedResponse(Request& request)
	{
		// TO DO: try {	// or try-catch in client?
		// 		    load chunk [set Headers & Flags, code (if unset), etc]
		// 		  } catch (const std::exception& e) {
		// 		    log error;
		// 		    return (prepareErrorResponse(errorCode));
		// 		  }
		// 		  if (error) {
		// 		    log error;
		// 		    return (prepareErrorResponse(errorCode));
		// 		  }
		// 		  set _isResponseReady;
		// 		  if (last chunk)
		// 		    unset _isChunkedResponse;

		(void)request;
	}

	void	Response::prepareResponse(Request& request)
	{
		// TO DO: if (_isChunkedResponse)
		// 		    return (_prepareChunkedResponse);
		// 		  try {	// or try-catch in client?
		// 		    load response [set Headers & Flags, code (if unset), check
		// 		    	 request method (405 like if POST or G_ET for GET), etc
		// 		  		 or set _isChunkedResponse and return (_prepareChunked)
		// 		  		 if (response size > (SEND_BUFFER_SIZE || SO_SNDBUF))?]
		// 		  } catch (const std::exception& e) {
		// 		    log error;
		// 		    return (prepareErrorResponse(errorCode));
		// 		  }
		// 		  if (error) {
		// 		    log error;
		// 		    return (prepareErrorResponse(errorCode));
		// 		  }
		// 		  set _isResponseReady;

		(void)request;
		if (request.getLocation()->getReturnPair().first >= 0)
			return (prepareErrorResponse(request,
						request.getLocation()->getReturnPair().first));
	}

	bool	Response::_loadErrorPage(Request& request)
	{
		Location::error_page_map::const_iterator	errorPage;
		int											errorCode;

		if (request.isInternalRedirect())
			return (false);
		errorPage = request.getLocation()->getErrorPages().find(_responseCode);
		if (errorPage != request.getLocation()->getErrorPages().end()) {
			if (errorPage->second[0] == '/') {
				if (request.getRequestMethod() != "GET")
					request.setRequestMethod("GET");
				errorCode = request.loadInternalRedirect(errorPage->second);
				if (errorCode != 0) {
					prepareErrorResponse(request, errorCode);
				} else
					prepareResponse(request);
				return (true);
			} else {
				_responseCode = 302;
				_location = errorPage->second;
			}
		}
		return (false);
	}

	bool	Response::_loadReturn(Request& request)
	{
		int			returnCode = request.getLocation()->getReturnPair().first;
		std::string	returnText = request.getLocation()->getReturnPair().second;
		int			serverPort = ntohs(request.getServerConfig()
												->getListenPair().sin_port);

		if (returnCode == 444 && returnText.empty()) {
			_isKeepAlive = false;
			_isResponseReady = true;
			return (true);
		}
		if (returnCode == 301 || returnCode == 302 || returnCode == 303
				|| returnCode == 307 || returnCode == 308) {
			if (!returnText.empty() && returnText[0] == '/') {
				_location = std::string("http://") + request.getHost();
				if (serverPort != 80)
					_location += std::string(":") + STRINGIZE(serverPort);
			}
			_location += returnText;
			return (false);
		} else if (returnCode < 0 || returnText.empty())
			return (false);
		_loadHeaders();
		_responseBuffer += returnText;
		_isResponseReady = true;
		return (true);
	}

	void	Response::prepareErrorResponse(Request& request, int errorCode)
	{
		const std::string*	specialBody = &Response::_getSpecialResponseBody(0);

		if (errorCode == 0 && (_responseCode < 300 || _responseCode > 599))
			errorCode = 500;
		else if (errorCode == 0)
			errorCode = _responseCode;
		clearResponse(request, errorCode);
		if (request.getLocation() && _loadReturn(request))
			return ;
		_isKeepAlive = isKeepAlive();
		if (request.getLocation() && _loadErrorPage(request))
			return ;
		specialBody = &(Response::_getSpecialResponseBody(_responseCode));
		if (!specialBody->empty()) {
			_contentType = "text/html";
			_contentLength = specialBody->length();
		}
		_loadHeaders();
		_responseBuffer += *specialBody;
		_isResponseReady = true;
	}

	void	Response::clearResponse(const Request& request,
									int responseCodeToKeep)
	{
		// TO DO: clear requested file;	// with try-catch if necessary

		_responseBuffer.clear();
		_responseCode = responseCodeToKeep;
		_contentType = "application/octet-stream";
		_contentLength = 0;
		_isKeepAlive = request.isKeepAlive();
		_location.clear();
		_isChunkedResponse = false;
		_isResponseReady = false;
	}

	/**************************************************************************/
	/*                         STATIC MEMBER FUNCTIONS                        */
	/**************************************************************************/

	const std::string&	Response::_getDate()
	{
		static char			buffer[32];
		static std::string	date(32, '\0');
		time_t				rawTime = std::time(0);
		struct tm*			gmtTime = std::gmtime(&rawTime);

		std::strftime(buffer, 32, "%a, %d %b %Y %H:%M:%S GMT", gmtTime);
		date = buffer;
		return (date);
	}

	const std::string&	Response::_getResponseStatus(int responseCode)
	{
		static std::string					emptyStatus(" ");
		static std::pair<int, std::string>	responseStatus[] = {
			std::make_pair(200, " OK"),
			std::make_pair(201, " Created"),
			std::make_pair(202, " Accepted"),
			std::make_pair(204, " No Content"),
			std::make_pair(206, " Partial Content"),
			std::make_pair(301, " Moved Permanently"),
			std::make_pair(302, " Moved Temporarily"),
			std::make_pair(303, " See Other"),
			std::make_pair(304, " Not Modified"),
			std::make_pair(307, " Temporary Redirect"),
			std::make_pair(308, " Permanent Redirect"),
			std::make_pair(400, " Bad Request"),
			std::make_pair(401, " Unauthorized"),
			std::make_pair(402, " Payment Required"),
			std::make_pair(403, " Forbidden"),
			std::make_pair(404, " Not Found"),
			std::make_pair(405, " Not Allowed"),
			std::make_pair(406, " Not Acceptable"),
			std::make_pair(408, " Request Time-out"),
			std::make_pair(409, " Conflict"),
			std::make_pair(410, " Gone"),
			std::make_pair(411, " Length Required"),
			std::make_pair(412, " Precondition Failed"),
			std::make_pair(413, " Request Entity Too Large"),
			std::make_pair(414, " Request-URI Too Large"),
			std::make_pair(415, " Unsupported Media Type"),
			std::make_pair(416, " Requested Range Not Satisfiable"),
			std::make_pair(421, " Misdirected Request"),
			std::make_pair(429, " Too Many Requests"),
			std::make_pair(500, " Internal Server Error"),
			std::make_pair(501, " Not Implemented"),
			std::make_pair(502, " Bad Gateway"),
			std::make_pair(503, " Service Temporarily Unavailable"),
			std::make_pair(504, " Gateway Time-out"),
			std::make_pair(505, " HTTP Version Not Supported"),
			std::make_pair(507, " Insufficient Storage")
		};
		static int		size = sizeof(responseStatus) / sizeof(*responseStatus);

		if (responseCode < 200 || responseCode > 507)
			return (emptyStatus);
		for (int i = 0; i < size; ++i) {
			if (responseStatus[i].first == responseCode)
				return (responseStatus[i].second);
		}
		return (emptyStatus);
	}

	const std::string&	Response::_getSpecialResponseBody(int responseCode)
	{
		static std::string					emptyString("");
		static std::pair<int, std::string>	responsePairs[] = {
			std::make_pair(301, "<html>" CRLF
			"<head><title>301 Moved Permanently</title></head>" CRLF
			"<body>" CRLF "<center><h1>301 Moved Permanently</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(302, "<html>" CRLF
			"<head><title>302 Found</title></head>" CRLF
			"<body>" CRLF "<center><h1>302 Found</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(303, "<html>" CRLF
			"<head><title>303 See Other</title></head>" CRLF
			"<body>" CRLF "<center><h1>303 See Other</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(307, "<html>" CRLF
			"<head><title>307 Temporary Redirect</title></head>" CRLF
			"<body>" CRLF "<center><h1>307 Temporary Redirect</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(308, "<html>" CRLF
			"<head><title>308 Permanent Redirect</title></head>" CRLF
			"<body>" CRLF "<center><h1>308 Permanent Redirect</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(400, "<html>" CRLF
			"<head><title>400 Bad Request</title></head>" CRLF
			"<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(401, "<html>" CRLF
			"<head><title>401 Authorization Required</title></head>" CRLF
			"<body>" CRLF "<center><h1>401 Authorization Required</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(402, "<html>" CRLF
			"<head><title>402 Payment Required</title></head>" CRLF
			"<body>" CRLF "<center><h1>402 Payment Required</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(403, "<html>" CRLF
			"<head><title>403 Forbidden</title></head>" CRLF
			"<body>" CRLF "<center><h1>403 Forbidden</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(404, "<html>" CRLF
			"<head><title>404 Not Found</title></head>" CRLF
			"<body>" CRLF "<center><h1>404 Not Found</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(405, "<html>" CRLF
			"<head><title>405 Not Allowed</title></head>" CRLF
			"<body>" CRLF "<center><h1>405 Not Allowed</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(406, "<html>" CRLF
			"<head><title>406 Not Acceptable</title></head>" CRLF
			"<body>" CRLF "<center><h1>406 Not Acceptable</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(408, "<html>" CRLF
			"<head><title>408 Request Time-out</title></head>" CRLF
			"<body>" CRLF "<center><h1>408 Request Time-out</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(409, "<html>" CRLF
			"<head><title>409 Conflict</title></head>" CRLF
			"<body>" CRLF "<center><h1>409 Conflict</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(410, "<html>" CRLF
			"<head><title>410 Gone</title></head>" CRLF
			"<body>" CRLF "<center><h1>410 Gone</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(411, "<html>" CRLF
			"<head><title>411 Length Required</title></head>" CRLF
			"<body>" CRLF "<center><h1>411 Length Required</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(412, "<html>" CRLF
			"<head><title>412 Precondition Failed</title></head>" CRLF
			"<body>" CRLF "<center><h1>412 Precondition Failed</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(413, "<html>" CRLF
			"<head><title>413 Request Entity Too Large</title></head>" CRLF
			"<body>" CRLF "<center><h1>413 Request Entity Too Large</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(414, "<html>" CRLF
			"<head><title>414 Request-URI Too Large</title></head>" CRLF
			"<body>" CRLF "<center><h1>414 Request-URI Too Large</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(415, "<html>" CRLF
			"<head><title>415 Unsupported Media Type</title></head>" CRLF
			"<body>" CRLF "<center><h1>415 Unsupported Media Type</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(416, "<html>" CRLF
			"<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
			"<body>" CRLF "<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(421, "<html>" CRLF
			"<head><title>421 Misdirected Request</title></head>" CRLF
			"<body>" CRLF "<center><h1>421 Misdirected Request</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(429, "<html>" CRLF
			"<head><title>429 Too Many Requests</title></head>" CRLF
			"<body>" CRLF "<center><h1>429 Too Many Requests</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(494, "<html>" CRLF
			"<head><title>400 Request Header Or Cookie Too Large</title></head>" CRLF
			"<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>Request Header Or Cookie Too Large</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(495, "<html>" CRLF
			"<head><title>400 The SSL certificate error</title></head>" CRLF
			"<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>The SSL certificate error</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(496, "<html>" CRLF
			"<head><title>400 No required SSL certificate was sent</title></head>"
			CRLF "<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>No required SSL certificate was sent</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(497, "<html>" CRLF
			"<head><title>400 The plain HTTP request was sent to HTTPS port</title></head>"
			CRLF "<body>" CRLF "<center><h1>400 Bad Request</h1></center>" CRLF
			"<center>The plain HTTP request was sent to HTTPS port</center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(498, "<html>" CRLF
			"<head><title>404 Not Found</title></head>" CRLF
			"<body>" CRLF "<center><h1>404 Not Found</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(500, "<html>" CRLF
			"<head><title>500 Internal Server Error</title></head>" CRLF
			"<body>" CRLF "<center><h1>500 Internal Server Error</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(501, "<html>" CRLF
			"<head><title>501 Not Implemented</title></head>" CRLF
			"<body>" CRLF "<center><h1>501 Not Implemented</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(502, "<html>" CRLF
			"<head><title>502 Bad Gateway</title></head>" CRLF
			"<body>" CRLF "<center><h1>502 Bad Gateway</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(503, "<html>" CRLF
			"<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
			"<body>" CRLF "<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(504, "<html>" CRLF
			"<head><title>504 Gateway Time-out</title></head>" CRLF
			"<body>" CRLF "<center><h1>504 Gateway Time-out</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(505, "<html>" CRLF
			"<head><title>505 HTTP Version Not Supported</title></head>" CRLF
			"<body>" CRLF "<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF),
			std::make_pair(507, "<html>" CRLF
			"<head><title>507 Insufficient Storage</title></head>" CRLF
			"<body>" CRLF "<center><h1>507 Insufficient Storage</h1></center>" CRLF
			"<hr><center>webserv</center>" CRLF "</body>" CRLF "</html>"
			CRLF)
		};
		static int		size = sizeof(responsePairs) / sizeof(responsePairs[0]);

		if (responseCode < 301 || responseCode > 507)
			return (emptyString);
		for (int i = 0; i < size; ++i) {
			if (responsePairs[i].first == responseCode)
				return (responsePairs[i].second);
		}
		return (emptyString);
	}

}	// namespace webserv
