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
};
