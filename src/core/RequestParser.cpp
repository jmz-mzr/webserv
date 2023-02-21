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

	bool	Request::_checkHeader(std::string str)
	{
		/*
		The value of the HTTP request header you want to set can only contain:
		Alphanumeric characters: a-z, A-Z, and 0-9
		The following special characters: _ :;.,\/"'?!(){}[]@<>=-+*#$&`|~^%
		*/

		std::string	format = "0123456789_ :;.,\\/\"\'?!(){}[]@<>=-+*#$&`|~^%";
		size_t		i = 0;
		size_t		len = str.length();

		if (str[str.size() - 1] == ' ')
			return false;
		while (i < len)
		{
			if (format.find(str[i]) != std::string::npos
			|| (str[i] >= 'a' && str[i] <= 'z')
			|| (str[i] >= 'A' && str[i <= 'Z']))
				i++;
			else
				return false;
		}
		return	true;
	}

	std::string        Request::_getKey(std::string line)
	{
		size_t i;
		std::string ret;

		i = line.find_first_of(':');
		if (i == std::string::npos)
		{
			LOG_ERROR("Key parsing error no \' : \' at \'" << line << "\'");
			_code = 400;
			return "";
		}
		if (_isCtlCharacter(line[i - 1]))
		{
			LOG_ERROR("Key parsing error CTL at \'" << ret << "\'index " << i - 1
			<< " : " << static_cast<int>(ret[i - 1]));
			_code = 400;
			return "";
		}
		ret = line.substr(0, i);
		_checkHeader(ret);
		return ret;
	}

	bool				Request::_isCtlCharacter(int c)
	{
		return ((c >= 0 && c <= 31) || c == 127);
	}

	bool				Request::_isNotCtlString(std::string s)
	{
		for (size_t i = 0; i < s.length() - 1; i++)
		{
			if (_isCtlCharacter(s[i]))
			{
				LOG_ERROR("CTL Character found at index " << i
				<< ": " << static_cast<int>(s[i]));
				return false;
			}
		}
		return true;
	}

	std::string       	Request::_getValue(std::string line)
	{
		size_t i;
		std::string ret;

		i = line.find_first_of(':');
		if (i == std::string::npos)
			return "";
		ret = line.substr(i + 1, line.size() - i + 1);
		i = ret.find_first_not_of(" \t");
		if (i == std::string::npos)
			return "";
		ret = ret.substr(i, ret.size() - i);
		i = ret.find_first_of(" \t");
		if (i != std::string::npos)
			ret = ret.substr(0, i);
		if (!_isNotCtlString(ret))
		{
			LOG_ERROR("CTL characters in value : " << ret);
			_code = 400;
		}
		return ret;
	}

	std::string		Request::_sanitizeUri(std::string uri)
	{
		size_t		i = 0;
		size_t		j = 0;
		std::string	res_uri(uri);

		while ((i = res_uri.find("/", i)) != std::string::npos)
		{
			j = res_uri.find_first_not_of("/", i);
			if (j == std::string::npos)
			{
				res_uri = res_uri.substr(0, res_uri.find_first_of('/')) + "/";
				break ;
			}
			res_uri = res_uri.substr(0, i + 1) + res_uri.substr(j, std::string::npos);
			i += j - i;
		}
		return	res_uri;
	}

	std::string		Request::_decodeUri(std::string uri)
	{
		size_t i = 0;
		size_t j = 0;
		std::string hexcode;
		std::string tmp_uri(uri);
		int 	ascii;

		//sanitize le path
		tmp_uri = _sanitizeUri(tmp_uri);
		//convertir les %hex en ascii
		while ((i = tmp_uri.find("%", i)) != std::string::npos)
		{
			hexcode = tmp_uri.substr(i + 1, 2);
			ascii = static_cast<int>(std::strtol(hexcode.c_str(), NULL, 16));
			//replace '%' by hexcode
			tmp_uri[i] = static_cast<char>(ascii);
			j = i + 1;
			while (j + 2 < tmp_uri.length() + 1)
			{
				tmp_uri[j] = tmp_uri[j + 2];
				j++;
			}
			tmp_uri[j] = '\0';
			//remove the processed part to prevent infinite loop
			i++;
		}
		//mettre la string en lowercase
		j = 0;
		while (j < tmp_uri.length())
		{
			tmp_uri[j] = static_cast<char>(tolower(tmp_uri[j]));
			j++;
		}
		return tmp_uri;
	}

	void        Request::_parsePath(std::string line)
	{
		size_t j ;
		std::string str;

		if((j = line.find_first_of(' ')) == std::string::npos)
		{
			LOG_ERROR("Path parsing error");
			_code = 400;
			return ;
		}
		str = line.substr(j + 1, line.size() - j + 1);
		if ((j = str.find_first_not_of(' ')) == std::string::npos)
		{
			LOG_ERROR("Path parsing error");
			_code = 400;
			return ;
		}
		str = str.substr(j, str.find_first_of(' ', j) - j);
		_raw_uri = str;
		if (_raw_uri.empty())
		{
			LOG_ERROR("Path parsing error");
			_code = 400;
			return ;
		}
		if (_raw_uri.length() > URI_MAX_LENGTH)
		{
			LOG_ERROR("Path parsing error : uri too long");
			_code = 414;
			return ;
		}
		_uri = _decodeUri(_raw_uri);
	}

	void        Request::_parseMethod(std::string line)
	{
		size_t  i;
		std::string str;

		if (line.empty())
		{
			return ;
		}
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

		j = line.find_first_of(_raw_uri) + _raw_uri.size();
		if (line.find("HTTP/", j) == std::string::npos)
		{
			LOG_ERROR("Version parsing error");
			_code = 400;
			return ;
		}
		str = line.substr(j + 1, line.size() - j + 1);
		j = str.find_first_not_of(' ');
		str = str.substr(j, 8);
		size_t i = str.find("HTTP/");
		if (i != 0 || i == std::string::npos)
		{
			LOG_ERROR("Version parsing error");
			_code = 400;
			return ;
		}
		this->_version = str.substr(str.find_first_of('/') + 1, 3);
		if (_version != "1.0" && _version != "1.1")
		{
			LOG_ERROR("Version parsing error");
			_code = 400;
			return ;
		}
	}

	void        Request::_parse(std::string str)
	{
		std::string line;
		std::string key;
		std::string value;

		_bufferIndex = 0;
		_buffer = str;
		_parseMethod(_readLine());
		LOG_DEBUG("code : " << _code);
		while((line = _readLine()) != "" && _code == 0 &&
			_bufferIndex != std::string::npos)
		{
			if (line == "\r" || line == "\n")
				break;
			LOG_DEBUG("Parsing line : \'" << line << "\'");
			key = _getKey(line);
			LOG_DEBUG("key found : " << key);
			if (key == "")
				break ;
			value = _getValue(line);
			if (value == "")
				break ;
			LOG_DEBUG("value found : " << value);
			if (_findHeader(key))
				_setHeader(key, value);
		}
	}

	void Request::_printRequestInfo()
	{
		LOG_INFO("method : " << _method);
		LOG_INFO("_uri : " << _uri);
		LOG_INFO("parsed Request headers : ");
		for (header_map::const_iterator it = _headers.begin();
				it != _headers.end(); ++it)
		{
			LOG_INFO("[" << it->first << "]: " << it->second);
		}
	}

} //namespace webserv
