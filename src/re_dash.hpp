#pragma once

#include <functional>
#include <string>
#include <lib_media/common/file_puller.hpp>
#include <lib_media/common/utc_start_time.hpp>

struct UtcStartTime : IUtcStartTimeQuery {
	uint64_t query() const override {
		//unused g_NullStartTime because ReDash makes an unusual "write" access
		//to the the start time, so the module and the pipeline need to know
		//the class, not the interface
		(void)g_NullStartTime;

		return startTime;
	}
	uint64_t startTime;
};

struct ReDashConfig {
	std::string url;
	UtcStartTime *utcStartTime = nullptr;
	int delayInSec = 0;
	int segmentDurationInMs = 0;
	int64_t timeshiftBufferDepthInSec = 24 * 60 * 60;
	std::string manifestFn;
	std::string baseUrlSub;
	std::string baseUrlAV;
	Modules::In::IFilePullerFactory *filePullerFactory = nullptr;

	// set by constructor - blocking call
	std::function<void(int/*delayInSec*/)> updateDelayInSec = nullptr;
};
