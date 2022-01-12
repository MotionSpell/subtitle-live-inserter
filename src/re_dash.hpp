#pragma once

#include <functional>
#include <string>
#include <lib_media/common/file_puller.hpp>

struct ReDashConfig {
	std::string url;
	int delayInSec = 0;
	int segmentDurationInMs = 0;
	int64_t timeshiftBufferDepthInSec = 24 * 60 * 60;
	std::string manifestFn;
	std::string baseUrlSub;
	std::string baseUrlAV;
	std::string postUrl;
	Modules::In::IFilePullerFactory *filePullerFactory = nullptr;

	// set by constructor - blocking call
	std::function<void(int/*delayInSec*/)> updateDelayInSec = nullptr;
};
