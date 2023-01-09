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
		typedef std::set<std::string, strcmp_icase>		limit_except_set;
		typedef std::pair<const int, std::string>		return_pair;
		typedef std::map<std::string, Location,
							location_compare>			locations_map;

		Location(const ServerConfig& src1, const Config& src2,
					const std::string& path);
		Location(const Location& src);
		~Location() { }

		const std::string&		getLocationName() const { return (_locationName); }
		const int64_t&			getMaxBodySize() const { return (_maxBodySize);}
		const limit_except_set&	getLimitExcept() const { return (_limitExcept);}
		const return_pair&		getReturnPair() const { return (_return); }
		const error_page_map&	getErrorPages() const { return (_errorPages); }
		const std::string&		getRoot() const { return (_root); }
		const std::string&		getAlias() const { return (_alias); }
		const std::string&		getIndex() const { return (_index); }
		bool					isAutoIndex() const { return (_autoIndex); }
		const std::string&		getFastCgiPass() const { return (_fastCgiPass);}

		const locations_map&	getLocations() const
												{ return (_locations); }
	private:
		Location();

		Location&	operator=(const Location& rhs);

		std::string				_locationName;
		int64_t					_maxBodySize;
		limit_except_set		_limitExcept;
		return_pair				_return;
		error_page_map			_errorPages;
		std::string				_root;
		std::string				_alias;
		std::string				_index;
		bool					_autoIndex;
		std::string				_fastCgiPass;

		locations_map			_locations;
	};

}	// namespace webserv

#endif	// LOCATION_HPP
