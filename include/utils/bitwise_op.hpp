#ifndef BITWISE_OP_HPP
# define BITWISE_OP_HPP

namespace webserv
{

	template<class T>
	inline T operator~(T a)
	{
		return (static_cast<T>( ~(static_cast<int>(a)) ));
	}

	template<class T>
	inline T operator|(T a, T b)
	{
		return (static_cast<T>( (static_cast<int>(a) | static_cast<int>(b)) ));
	}

	template<class T>
	inline T operator&(T a, T b)
	{
		return (static_cast<T>( (static_cast<int>(a) & static_cast<int>(b)) ));
	}

	template<class T>
	inline T operator^(T a, T b)
	{
		return (static_cast<T>( (static_cast<int>(a) ^ static_cast<int>(b)) ));
	}

	template<class T>
	inline T& operator|=(T& a, T b)
	{
		return (reinterpret_cast<T&>(
					(reinterpret_cast<int&>(a) |= static_cast<int>(b)) ));
	}

	template<class T>
	inline T& operator&=(T& a, T b)
	{
		return (reinterpret_cast<T&>(
					(reinterpret_cast<int&>(a) &= static_cast<int>(b)) ));
	}

	template<class T>
	inline T& operator^=(T& a, T b)
	{
		return (reinterpret_cast<T&>(
					(reinterpret_cast<int&>(a) ^= static_cast<int>(b)) ));
	}

}	// namespace webserv

#endif	// BITWISE_OP_HPP
