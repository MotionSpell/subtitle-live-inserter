#include "shell.hpp"
#include <iostream>  // cin
#include <algorithm> // transform
#include <sstream>   // istringstream
#include <cctype>    // toupper

extern const char *g_appName;

Shell::Shell() {
	addAction("help", std::bind(&Shell::helpAction, this, std::placeholders::_1));
	addAction("exit", std::bind(&Shell::exitAction, this, std::placeholders::_1));
}

void Shell::helpAction(std::string) {
	std::cout << "Help for \"" << g_appName << "\"" << std::endl;
	std::cout << "Available commands:" << std::endl;
	for (auto &a : actions)
		std::cout << "\t" << a.key << std::endl;
}

void Shell::exitAction(std::string) {
	exit = true;
}

void Shell::addAction(std::string name, ActionParseFct fct) {
	if (actions.find(name) != actions.end())
		std::cerr << "[" << g_appName << "] " << "Overwriting shell function \"" << name << "\"" << std::endl;

	actions[name] = fct;
}

void Shell::run() {
	while (!exit) {
		try {
			processCommand();
		} catch (std::exception const &e) {
			std::cerr << "[" << g_appName << "] " << "Error: " << e.what() << std::endl;
		}
	}
}

void Shell::processCommand() {
	std::string line;
	std::getline(std::cin, line);
	std::istringstream isLine(line);

	std::string command;
	isLine >> command;
	line.erase(0, command.size() + 1/*separator*/);

	if (actions.find(command) == actions.end())
		std::cerr << "[" << g_appName << "] " << "Unknown command \"" << command << "\"" << " in line \"" << line << "\"" << std::endl;

	actions[command](line);
}
