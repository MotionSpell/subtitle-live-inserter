#pragma once

#include <string>
#include <lib_media/common/utc_start_time.hpp>

struct SubtitleSourceConfig {
	std::string subtitleFn;
	uint64_t segmentDurationInMs = 2000;
	bool rectify = false;
	IUtcStartTimeQuery const *utcStartTime = nullptr;
};
