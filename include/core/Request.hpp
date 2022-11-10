#ifndef REQUEST_HPP
# define REQUEST_HPP

namespace	webserv
{

	enum RequestMethod {
		kEmpty,
		kGet,
		kPost,
		kDelete
	};

	class	Request {
	public:
		Request();
		Request(const Request& src);
		~Request() { }	// clear _chunks if not automatic

		const enum RequestMethod&	getRequestMethod() const;

		bool	isChunkedRequest() const { return (_isChunkedRequest); }
		bool	isTerminatedRequest() const { return (_isTerminatedRequest); }
//		bool	isKeepAlive() const;

		int		parseRequest(const char* buffer);

		void	clearRequest();
	private:
		Request&	operator=(const Request& rhs);

		int		_parseChunkedRequest(const char* buffer);

		enum RequestMethod	_requestMethod;
		bool				_isChunkedRequest;
//		std::ofstream		_chunks;	// or with a swap space?
		bool				_isTerminatedRequest;
//		bool				_keepAlive;	// divide in a Class with parameters
//										// or use with SO_KEEPALIVE
	};

}	// namespace webserv

#endif	// REQUEST_HPP
