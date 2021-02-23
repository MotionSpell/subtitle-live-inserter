#pragma once

#include <functional>
#include <string>
#include <lib_media/common/file_puller.hpp>
#include <lib_media/common/utc_start_time.hpp>

struct UtcStartTime : IUtcStartTimeQuery {
	uint64_t query() const override {
		return startTime;
	}
	uint64_t startTime;
};

struct ReDashConfig {
	std::string url;
	UtcStartTime *utcStartTime = nullptr;
	int delayInSec = 0;
	int64_t timeshiftBufferDepthInSec = 24 * 60 * 60;
	std::string mpdFn;
	std::string postUrl;
	Modules::In::IFilePullerFactory *filePullerFactory = nullptr;

	// set by constructor - blocking call
	std::function<void(int/*delayInSec*/)> updateDelayInSec = nullptr;
};
