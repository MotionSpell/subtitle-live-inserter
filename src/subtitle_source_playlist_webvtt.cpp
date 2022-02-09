#include "lib_modules/core/module.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include <sstream>
#include <string>
#include <vector>

extern int hlsPlaylistOffsetInSec;

std::string getContentWebvtt(const std::vector<char> &input, int segNum, int64_t segmentDurationInMs) {
	std::string line, out, in(input.begin(), input.end());
	std::stringstream ss(in);
	while(std::getline(ss, line)) {
		if (line.find(" --> ") != line.npos) {
			const size_t timecodeSize = 24;
			char timecodeShow[timecodeSize] = {};
			timeInMsToStr(segNum * segmentDurationInMs + hlsPlaylistOffsetInSec * 1000, timecodeShow, ".");
			timecodeShow[timecodeSize - 1] = 0;
			char timecodeHide[timecodeSize] = {};
			timeInMsToStr((segNum + 1) * segmentDurationInMs + hlsPlaylistOffsetInSec * 1000, timecodeHide, ".");
			timecodeHide[timecodeSize - 1] = 0;
			auto const bufferSize = timecodeSize * 2 + 5 + 1;
			char buffer[bufferSize] {};
			snprintf(buffer, bufferSize, "%s --> %s", timecodeShow, timecodeHide);

			out += buffer;
		} else {
			out += line;
		}

		out += "\n";
	}

	return out;
}
