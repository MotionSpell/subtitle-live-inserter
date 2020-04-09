#pragma once

#include <string>

struct Config {
	std::string url;
	int delayInSec = 0;
	int subtitleForwardTimeInSec = 0;
	std::string subListFn;
    std::string postUrl = ".";

	bool help = false;
};
