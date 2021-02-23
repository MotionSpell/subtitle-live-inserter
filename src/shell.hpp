#pragma once

#include "lib_utils/small_map.hpp"
#include <functional>
#include <string>

struct Shell {
		typedef std::function<void(std::string/*parameters*/)> ActionParseFct;

		Shell();
		void helpAction(std::string);
		void exitAction(std::string);
		void addAction(std::string name, ActionParseFct fct);
		void run();
		void processCommand();

	private:
		bool exit = false;
		SmallMap<std::string/*name*/, ActionParseFct> actions;
};
