#include "core/Response.hpp"
#include "utils/Logger.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Response::Response(): _responseCode(0),
							_isKeepAlive(true),
							_isChunkedResponse(false),
							_isResponseReady(false)
	{
		LOG_INFO("New Response instance");
	}

	Response::Response(const Response& src):
									_responseBuffer(src._responseBuffer),
									_responseCode(src._responseCode),
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

	void	Response::_prepareChunkedResponse(const Request& request)
	{
		// TO DO: try {
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
		// 		  try {
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
		// 		    && _responseCode < 400 && _responseCode > 599)
		// 		      errorCode = 500;
		//		  clearResponse();
		// 		  load error response corresponding to the errorCode;
		// 		  set _isResponseReady;

		(void)errorCode;
	}

	void	Response::clearResponse()
	{
		// TO DO: clear requested file;	// with try-catch if necessary

		_responseBuffer.clear();
		_responseCode = 0;
		_isKeepAlive = true;
		_isChunkedResponse = false;
		_isResponseReady = false;
	}

}	// namespace webserv
