#pragma once

#include <lib_media/common/utc_start_time.hpp>
#include <string>

struct HlsWebvttRephaserConfig {
	std::string url;
	int segmentDurationInMs = 0;
	int64_t timeshiftBufferDepthInSec = -1;
	int delayInSec = 0;
	int subtitleForwardTimeInSec = 0;
};
