#pragma once

#include <lib_media/common/utc_start_time.hpp>
#include <string>

struct HlsWebvttRephaserConfig {
	std::string url;
	int segmentDurationInMs = 0;
	IUtcStartTimeQuery const *utcStartTime = nullptr;
};
