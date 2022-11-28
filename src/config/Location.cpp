#include "config/Location.hpp"
#include "config/Config.hpp"
#include "utils/Logger.hpp"
#include "webserv_config.hpp"

namespace	webserv
{

	/**************************************************************************/
	/*                       CONSTRUCTORS / DESTRUCTORS                       */
	/**************************************************************************/

	Location::Location()
			: Config()
			, _limitExcept( Method::kGet | Method::kPost | Method::kDelete )
			, _index("index.html")
	{
		LOG_INFO("New Location instance");
	}

	Location::Location(const ServerConfig& src)
			: Config(src)
	{
		LOG_INFO("New Location instance");
	}

	Location::Location(const Location& src)
			: Config(src)
			, _limitExcept(src._limitExcept)
			, _return(src._return)
			, _fastCgiPass(src._fastCgiPass)
			, _index(src._index)
			, _autoIndex(src._autoIndex)
			, _root(src._root)
			, _locations(src._locations)
	{
		LOG_INFO("Location copied");
	}

	/**************************************************************************/
	/*                            MEMBER FUNCTIONS                            */
	/**************************************************************************/
	const int&						Location::getLimitExcept() const
	{ return (_limitExcept); }

	const Location::return_pair&	Location::getReturnPair() const
	{ return (_return); }

	const std::string&				Location::getFastCgiPass() const
	{ return (_fastCgiPass); }

	const std::string&				Location::getIndex() const
	{ return (_index); }

	bool							Location::isAutoIndex() const
	{ return (_autoIndex); }

	const std::string&				Location::getRoot() const
	{ return (_root); }

	const Location::locations_map&	Location::getLocations() const
	{ return (_locations); }

	void	Location::setLimitExcept(int flags)
	{ _limitExcept &= flags; }

	void	Location::setReturnPair(Location::return_pair returnPair)
	{ _return = std::make_pair(returnPair.first, returnPair.second); }

	void	Location::setFastCgiPass(std::string path)
	{ _fastCgiPass = path; }

	void	Location::setIndex(std::string path)
	{ _index = path; }

	void	Location::setAutoIndex(bool b)
	{ _autoIndex = b; }

	void	Location::setRoot(std::string path)
	{ _root = path; }

	bool	Location::addLocation(const std::string& path,
									const Location& location)
	{
		return (_locations.insert(std::make_pair(path, location)).second);
	}

}	// namespace webserv
