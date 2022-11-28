#ifndef CONFIG_HPP
#define CONFIG_HPP

# include <map>
# include <string>
# include <set>
# include <utility>

# include "utils/utils.hpp"

namespace webserv {

struct ErrorPage {
	int			status_code;
	std::string uri;
};

class Location;

class Config
{
public:
	typedef std::map<int, std::string>						error_pages_map;

	bool	addErrorPage(const int status, const std::string& uri);
	void	setMaxBodySize(const long long size);

	const long long&			getMaxBodySize() const;
	const error_pages_map&		getErrorPages() const;

protected:
	Config();
	Config(const Config& src);
	~Config() { }

	long long			_maxBodySize;
	error_pages_map		_errorPages;

private:
	Config& operator=(const Config& rhs);

};

}	// namespace webserv

#endif // CONFIG_HPP
