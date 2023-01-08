#include "core/Request.hpp"
#include "utils/utils.hpp"
#include "utils/Logger.hpp"
#include <iostream>

namespace webserv
{
	void    Request::_initHeaders()
	{
		_headers["Accept"] = "";
		_headers["Accept-Charset"] = "";
		_headers["Accept-Encoding"] = "";
		_headers["Accept-Language"] = "";
		_headers["Authorization"] = "";
		_headers["Cache-Control"] = "";
		_headers["Connection"] = "";
		_headers["Content-Encoding"] = "";
		_headers["Content-Length"] = "";
		_headers["Content-Type"] = "";
		_headers["Date"] = "";
		_headers["From"] = "";
		_headers["Host"] = "";
		_headers["Transfer-Encoding"] = "";
		_headers["User-Agent"] = "";
	}

	bool    Request::_findHeader(std::string key)
	{
		if (_headers.count(key))
			return true;
		return false;
	}

	void    Request::_setHeader(std::string key, std::string value)
	{
		_headers[key] = value;
	}

	std::string Request::_readLine()
	{
		std::string line;
		size_t      i;

		if (_bufferIndex == std::string::npos)
			return "";
		i = _buffer.find_first_of('\n', _bufferIndex);
		if (i == std::string::npos)
			return "";
		line = _buffer.substr(_bufferIndex, i - _bufferIndex);
		if (i == std::string::npos)
			_bufferIndex = i;
		else
			_bufferIndex = i + 1;
		return line;
	}

	std::string        Request::_getKey(std::string line)
	{
		size_t i;
		std::string ret;

		i = line.find_first_of(':');
		if (i == std::string::npos)
			return "";
		ret = line.substr(0, i);
		return ret;
	}

	std::string        Request::_getValue(std::string line)
	{
		size_t i;
		std::string ret;

		i = line.find_first_of(':');
		if (i == std::string::npos)
			return "";
		ret = line.substr(i + 1, line.size() - i + 1);
		i = ret.find_first_of(' ');
		if (i == std::string::npos)
			return "";
		ret = ret.substr(i + 1, ret.size() - i + 1);
		return ret;
	}

	void        Request::_parsePath(std::string line)
	{
		size_t j ;
		std::string str;

		j = line.find_first_of(' ');
		str = line.substr(j + 1, line.size() - j + 1);
		j = str.find_first_of(' ');
		str = str.substr(0, j);
		_uri = str;
	}

	void        Request::_parseMethod(std::string line)
	{
		size_t  i;
		std::string str;

		i = line.find_first_of(' ');
		_method.assign(line, 0, i);
		_parsePath(line);
		_checkVersion(line);
	}

	//Check Request HTTP Version (accept 1.0 and 1.1)
	void		Request::_checkVersion(std::string line)
	{
		size_t 		j;
		std::string	str;

		j = line.find_last_of(' ');
		str = line.substr(j + 1, line.size() - j + 1);
		size_t i = str.find("HTTP/");
		if (i != 0 || i == std::string::npos)
		{
			_code = 400;
			LOG_DEBUG("HTTP BAD VERSION" << std::endl);
			return ;
		}
		this->_version = str.substr(str.find_first_of('/') + 1, 3);
	}

	void        Request::_parse(std::string str)
	{
		std::string line;
		std::string key;
		std::string value;

		LOG_DEBUG("Raw buffer before at parsing: " << str);
		_bufferIndex = 0;
		_buffer = str;
		_parseMethod(_readLine());
		while((line = _readLine()) != "" && _bufferIndex != std::string::npos)
		{
			key = _getKey(line);
			if (key == "")
				break ;
			value = _getValue(line);
			if (value == "")
				break ;
			if (_findHeader(key))
				_setHeader(key, value);
		}
		LOG_DEBUG("method : " << _method);
		LOG_DEBUG("_uri : " << _uri);
		LOG_DEBUG("parsed headers : ");
		for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		{
			LOG_DEBUG("[" << it->first << "]: " << it->second);
		}
	}
} //namespace webserv
