#include "core/Response.hpp"
#include "utils/Logger.hpp"

#include <utility>

#define CRLF	"\r\n"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Response::Response(): _responseCode(0),
							_contentType("text/html"),
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

	void	Response::_loadHeaders()
	{
		//_responseBuffer << "HTTP/1.1 " << ... ;
	}

	void	Response::_prepareChunkedResponse(const Request& request)
	{
		// TO DO: try {	// or try-catch in client?
		// 		    load chunk [set Headers & Flags, response code, etc]
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

	void	Response::prepareResponse(const Request& request)
	{
		// TO DO: if (_isChunkedResponse)
		// 		    return (_prepareChunkedResponse);
		// 		  try {	// or try-catch in client?
		// 		    load response [set Headers & Flags, response code, etc,
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
	}

	void	Response::prepareErrorResponse(int errorCode)
	{
		// TO DO: if (errorCode == 0
		// 		    && _responseCode < 300 && _responseCode > 599)
		// 		      errorCode = 500;
		//		  clearResponse();
		// 		  load error response corresponding to the errorCode;
		// 		  set _isResponseReady;
		// 		  First code simple error, then with _errorPages

		if (errorCode == 0 && (_responseCode < 300 || _responseCode > 599))
			_responseCode = 500;
		else if (_responseCode == 0)
			_responseCode = errorCode;
		if (_responseCode == 400 || _responseCode == 413 || _responseCode == 414
				|| _responseCode == 497 || _responseCode == 495
				|| _responseCode == 496 || _responseCode == 500
				|| _responseCode == 501)
			_isKeepAlive = false;
		_loadHeaders();
	}

	void	Response::clearResponse()
	{
		// TO DO: clear requested file;	// with try-catch if necessary

		_responseBuffer.clear();
		_responseCode = 0;
		_contentType = "text/html";
		_contentLength = 0;
		_isKeepAlive = true;
		_isChunkedResponse = false;
		_isResponseReady = false;
	}

	const std::string& getSpecialResponseBody(int responseCode)
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

		for (int i = 0; i < size; ++i) {
			if (responsePairs[i].first == responseCode)
				return (responsePairs[i].second);
		}
		return (emptyString);
	}

}	// namespace webserv
