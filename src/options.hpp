#pragma once

#include <functional>
#include <string>

struct Config {
	std::string url;
	int segmentDurationInMs = 2000;
	int delayInSec = 0;
	int subtitleForwardTimeInSec = 0;
	std::string subListFn;
	std::string baseUrl = ".";
	std::string postUrl = ".";
	std::string manifestFn;
	std::string outputFormat = "dash";
	bool rectify = false;

	std::function<void(int/*delayInSec*/)> updateDelayInSec;

	bool help = false;
	bool shell = false;
};
