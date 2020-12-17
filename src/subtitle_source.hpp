#pragma once

#include <string>
#include <lib_media/common/utc_start_time.hpp>

struct SubtitleSourceConfig {
	std::string filename;
	uint64_t segmentDurationInMs;
	IUtcStartTimeQuery const *utcStartTime;
};
