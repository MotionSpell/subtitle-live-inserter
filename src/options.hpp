#pragma once

#include <functional>
#include <string>

struct Config {
	std::string url;
	int segmentDurationInMs = 2000;
	int delayInSec = 0;
	int subtitleForwardTimeInSec = 0;
	std::string subListFn;
	std::string baseUrlSub = ".";
	std::string baseUrlAV; /*empty: copy from source*/
	std::string displayedName = "subtitle"; /*used by players*/
	int timeshiftBufferDepthInSec = -1; /*0 = infinite, -1 = same as source*/
	std::string postUrl = ".";
	std::string manifestFn;
	std::string outputFormat = "dash";
	bool rectify = false;
	bool legacy = false;

	std::function<void(int/*delayInSec*/)> updateDelayInSec;

	bool help = false;
	bool shell = false;
};
