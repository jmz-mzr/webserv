#ifndef PRINT_CONFIG_TREE_HPP
# define PRINT_CONFIG_TREE_HPP

# include "config/Location.hpp"
# include "config/ServerConfig.hpp"
# include "core/Server.hpp"

namespace webserv {

	struct print_config_tree {

		print_config_tree() : depth(0) {
			std::cout
				<< "========================================" << std::endl
				<< "          CONFIG TREE           " << std::endl
				<< "========================================" << std::endl;
		}

		void operator()(const Location& location) {
			for (int i = 0; i < depth; i++)
				std::cout << "\t";
			std::cout << "-> \"" << location.getLocationName() << "\""
						<< std::endl;
			depth++;
			webserv::Location::locations_map::const_iterator it = location.getNestedLocations().begin();
			if (it != location.getNestedLocations().end()) {
				for (int i = 0; i < depth; i++)
					std::cout << "\t";
				std::cout << "-> ";
				while (it != location.getNestedLocations().end()) {
					std::cout << " \"" << it->first << "\"";
					it++;
				}
				std::cout << std::endl;
			}
			depth--;
		}

		void operator()(const ServerConfig& server)
		{
			for (int i = 0; i < depth; i++)
				std::cout << "\t" << "->";
			for (std::set<std::string, strcmp_icase>::const_iterator it = server.getServerNames().begin();
					it != server.getServerNames().end();
					it++) {
				std::cout << " \"" << *it << "\"";
			}
			std::cout << std::endl;
			depth++;
			for (webserv::ServerConfig::location_map::const_iterator it = server.getLocations().begin();
					it != server.getLocations().end();
					it++) {
				this->operator()(it->second);
			}
			depth--;
		}

		void operator()(const Server& server)
		{
			for (int i = 0; i < depth; i++)
				std::cout << "\t";
			std::cout << "-> " << server.getSocket() << std::endl;
			depth++;
			for (std::vector<ServerConfig>::const_iterator it = server.getConfigs().begin();
					it != server.getConfigs().end();
					it++) {
				this->operator()(*it);
			}
			depth--;
			std::cout << std::endl;
		}

		int depth;
	};

}	// namespace webserv

#endif	// PRINT_CONFIG_TREE_HPP
