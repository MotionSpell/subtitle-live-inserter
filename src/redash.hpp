#pragma once

#include <string>
#include <lib_media/common/utc_start_time.hpp>

struct UtcStartTime : IUtcStartTimeQuery {
	uint64_t query() const override {
		return startTime;
	}
	uint64_t startTime;
};

struct ReDashConfig {
	std::string url;
	UtcStartTime *utcStartTime;
	int delayInSec = 0;
	int64_t timeshiftBufferDepthInSec = 24 * 60 * 60;
	std::string mpdFn;
};
