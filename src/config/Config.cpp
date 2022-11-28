#include "config/Config.hpp"

#include <map>
#include <string>

#include "utils/Logger.hpp"

namespace webserv {

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

Config::Config()
		: _maxBodySize(1048576UL)
{
	std::string path(STRINGIZE(WEBSERV_ROOT));

	const ErrorPage		defaultPages[] =
	{
		{ 400u, path + "/config/html/400.html" },
		{ 403u, path + "/config/html/403.html" },
		{ 404u, path + "/config/html/404.html" },
		{ 405u, path + "/config/html/405.html" },
		{ 410u, path + "/config/html/410.html" },
		{ 413u, path + "/config/html/413.html" },
		{ 500u, path + "/config/html/500.html" }
	};
	size_t length = sizeof(defaultPages) / sizeof(*defaultPages);
	const ErrorPage* it = defaultPages;

	while (it < defaultPages + length) {
		addErrorPage(it->status_code, it->uri);
		it++;
	}
}

Config::Config(const Config& src)
		: _maxBodySize(src.getMaxBodySize())
		, _errorPages(src.getErrorPages())
{ }

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/

const long long&					Config::getMaxBodySize() const
{ return (_maxBodySize); }

const Config::error_pages_map&		Config::getErrorPages() const
{ return (_errorPages); }

bool	Config::addErrorPage(const int status, const std::string& uri)
{ return (_errorPages.insert(std::make_pair(status, uri)).second); }

void	Config::setMaxBodySize(const long long size)
{ _maxBodySize = size; }

}	// namespace webserv