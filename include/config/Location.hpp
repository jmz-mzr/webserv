#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <map>
# include <string>
# include <set>
# include <utility>

# include "config/Config.hpp"
# include "config/ServerConfig.hpp"
# include "utils/utils.hpp"

namespace	webserv {

	class	ServerConfig;

	struct	Method {
		enum Type {
			kEmpty = 0x00,
			kGet = 0x01,
			kPost = 0x02,
			kDelete = 0x04,
		};
	};

	class	Location : public Config {
	public:
		typedef std::map<int, std::string>				error_pages_map;
		typedef std::pair<int, std::string>				return_pair;
		typedef std::map<std::string, Location,
							location_compare>			locations_map;

		Location();
		Location(const ServerConfig& src);
		Location(const Location& src);
		~Location() { }

		const int&				getLimitExcept() const;
		const return_pair&		getReturnPair() const;
		const std::string&		getFastCgiPass() const;
		const std::string&		getIndex() const;
		bool					isAutoIndex() const;
		const std::string&		getRoot() const;
		const locations_map&	getLocations() const;

		void	setLimitExcept(int bitfield);
		void	setReturnPair(return_pair returnPair);
		void	setFastCgiPass(std::string path);
		void	setIndex(std::string path);
		void	setAutoIndex(bool b);
		void	setRoot(std::string path);
		bool	addLocation(const std::string& path, const Location& location);

	private:
		Location&			operator=(const Location& rhs);

		int					_limitExcept;
		return_pair			_return;
		std::string			_fastCgiPass;
		std::string			_index;
		bool				_autoIndex;
		std::string			_root;
		locations_map		_locations;
	};

}	// namespace webserv

#endif	// LOCATION_HPP
