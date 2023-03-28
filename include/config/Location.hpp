#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <stdint.h>	// int64_t

# include <map>
# include <set>
# include <string>
# include <utility>		// pair

# include "config/Config.hpp"
# include "config/ServerConfig.hpp"
# include "utils/utils.hpp"

namespace	webserv
{

	class	ServerConfig;

	class	Location {
	public:
		typedef std::map<int, std::string>				error_page_map;
		typedef std::set<std::string, strcmp_icase>		limit_except_set;
		typedef std::set<std::string, strcmp_icase>		ignore_except_set;
		typedef std::pair<const int, std::string>		return_pair;
		typedef std::map<std::string, Location,
							location_compare>			locations_map;

		Location(const ServerConfig& src1, const Config& src2,
					const std::string& path);
		Location(const Location& src1, const Config& src2,
					const std::string& path);
		~Location() { }

		const std::string&			getLocationName() const
												{ return (_locationName); }
		const int64_t&				getMaxBodySize() const
												{ return (_maxBodySize); }
		const limit_except_set&		getLimitExcept() const
												{ return (_limitExcept); }
		const ignore_except_set&	getIgnoreExcept() const
												{ return (_ignoreExcept); }
		const return_pair&			getReturnPair() const
												{ return (_returnPair); }
		const error_page_map&		getErrorPages() const
												{ return (_errorPages); }
		const std::string&			getRoot() const
												{ return (_root); }
		const std::string&			getAlias() const
												{ return (_alias); }
		const std::string&			getIndex() const
												{ return (_index); }
		bool						isAutoIndex() const
												{ return (_autoIndex); }
		bool						hideLimitRule() const
												{ return (_hideLimitRule); }
		bool						hideDirectory() const
												{ return (_hideDirectory); }
		const std::string&			getCgiPass() const
												{ return (_cgiPass); }
		const locations_map&		getNestedLocations() const
												{ return (_locations); }
	private:
		Location();

		Location&	operator=(const Location& rhs);

		std::string				_locationName;
		int64_t					_maxBodySize;
		limit_except_set		_limitExcept;
		ignore_except_set		_ignoreExcept;
		return_pair				_returnPair;
		error_page_map			_errorPages;
		std::string				_root;
		std::string				_alias;
		std::string				_index;
		bool					_autoIndex;
		bool					_hideLimitRule;
		bool					_hideDirectory;
		std::string				_cgiPass;

		locations_map			_locations;
	};

}	// namespace webserv

#endif	// LOCATION_HPP
