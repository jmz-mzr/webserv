#ifndef GLOBAL_DEFS_HPP
#define GLOBAL_DEFS_HPP

#define CRLF		"\r\n"

#define SERVER		"server"
#define CLIENT		"client"

// Concatenates strings
# define JOIN_IMPL_(foo, bar)	foo##bar
# define JOIN(foo, bar)			JOIN_IMPL_(foo, bar)

// Creates a string literal, enclosing the argument between quotes
//! Use XSTR() for macro stringize (see below)
# define STRINGIZE(name)		#name
// First expand macro, then stringize it
# define XSTR(macro)			STRINGIZE(macro)

#endif // GLOBAL_DEFS_HPP
