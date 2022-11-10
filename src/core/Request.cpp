#include "core/Request.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Request::Request(): _requestMethod(kEmpty),
						_isChunkedRequest(false),
						_isTerminatedRequest(false)
	{
		LOG_INFO("New Request instance");
	}

	Request::Request(const Request& src): _requestMethod(src._requestMethod),
								_isChunkedRequest(src._isChunkedRequest),
								_isTerminatedRequest(src._isTerminatedRequest)
	{
		// TO DO: handle swap of std::ofstream (or other _chunks object)
		// 		  or not if Request in only copied at Client creation?

		LOG_INFO("Request copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

	const enum RequestMethod&	Request::getRequestMethod() const
	{
		return (_requestMethod);
	}

	int	Request::_parseChunkedRequest(const char* buffer)
	{
		// TO DO: parse [check and set Headers & Flags, etc]
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  try {
		// 		    save chunk;
		// 		  } catch (const std::exception& e) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  if (last chunk)
		// 		    set _isTerminatedRequest;
		// 		  return (0);

		(void)buffer;
		return (0);
	}

	int	Request::parseRequest(const char* buffer)
	{
		// TO DO: if (_isChunkedRequest)
		// 		    return (_parseChunkedRequest);
		// 		  parse [check and set Headers & Flags, etc,
		// 		  		 or set _isChunkedRequest and return (_parseChunked)]
		// 		  if (error) {
		// 		    log error;
		// 		    clearRequest();
		// 		    return (errorCode 4xx or 5xx);
		// 		  }
		// 		  set _isTerminatedRequest;
		// 		  return (0);

		(void)buffer;
		return (0);
	}

	void	Request::clearRequest()
	{
		// TO DO: clear saved chunks;	// with try-catch if necessary

		_requestMethod = kEmpty;
		_isChunkedRequest = false;
		_isTerminatedRequest = false;
	}

}	// namespace webserv
