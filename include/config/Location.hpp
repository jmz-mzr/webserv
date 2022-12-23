#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <map>
# include <string>
# include <set>
# include <utility>

# include "config/ServerConfig.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	ServerConfig;

	class	Location {
	public:
		typedef std::map<int, std::string>				error_page_map;
		typedef std::set<std::string>					limit_except_set;
		typedef std::pair<const int, std::string>		return_pair;
		typedef std::map<std::string, Location,
							location_compare>			locations_map;

		Location(const ServerConfig& src1, const Config& src2);
		Location(const Location& src);
		~Location() { }

		const error_page_map&	getErrorPages() const { return (_errorPages); }
		const int64_t&		getMaxBodySize() const { return (_maxBodySize);}

		const limit_except_set&	getLimitExcept() const { return (_limitExcept);}
		const return_pair&		getReturnPair() const { return (_return); }
		const std::string&		getRoot() const { return (_root); }
		bool					isAutoIndex() const { return (_autoIndex); }
		const std::string&		getIndex() const { return (_index); }
		const std::string&		getFastCgiPass() const { return (_fastCgiPass);}

		const locations_map&	getLocations() const
												{ return (_locations); }
	private:
		Location();

		Location&	operator=(const Location& rhs);

		int64_t				_maxBodySize;
		limit_except_set		_limitExcept;
		return_pair				_return;
		std::string				_fastCgiPass;
		std::string				_index;
		bool					_autoIndex;
		std::string				_root;
		error_page_map			_errorPages;

		locations_map			_locations;
	};

}	// namespace webserv

#endif	// LOCATION_HPP
