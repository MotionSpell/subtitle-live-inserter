#pragma once

#include <string>

struct HlsWebvttRephaserConfig {
	std::string url;
	int segmentDurationInMs = 0;
};
