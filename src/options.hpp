#pragma once

#include <functional>
#include <string>

struct Config {
	std::string url;
	int segmentDurationInMs = 8000; // Romain 2000;
	int delayInSec = 0;
	int subtitleForwardTimeInSec = 0;
	std::string subListFn;
	std::string baseUrlSub = ".";
	std::string baseUrlAV; /*empty: copy from source*/
	int timeshiftBufferDepthInSec = 0; /*0 = infinite, -1 = same as source*/
	std::string postUrl = ".";
	std::string manifestFn;
	std::string outputFormat = "dash";
	bool rectify = false;

	std::function<void(int/*delayInSec*/)> updateDelayInSec;

	bool help = false;
	bool shell = false;
};
