#pragma once

#include <functional>
#include <string>

struct Config {
	std::string url;
	int delayInSec = 0;
	int subtitleForwardTimeInSec = 0;
	std::string subListFn;
	std::string postUrl = ".";
	std::string mpdFn;

	std::function<void(int/*delayInSec*/)> updateDelayInSec;

	bool help = false;
	bool shell = false;
};
