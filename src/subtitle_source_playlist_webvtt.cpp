#include "lib_modules/core/module.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include <cassert>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

extern int hlsPlaylistOffsetInSec;

std::string getContentWebvtt(const std::vector<char> &input, int segNum, int64_t segmentDurationInMs) {
	std::string line, out, in(input.begin(), input.end());
	std::stringstream ss(in);
	while(std::getline(ss, line)) {
		auto const sep = " --> ";
		auto const sepPos = line.find(sep);
		if (sepPos != line.npos) {
			// times default value: full segment
			int64_t startTimeInMs = segNum * segmentDurationInMs + hlsPlaylistOffsetInSec * 1000;
			int64_t endTimeInMs = (segNum + 1) * segmentDurationInMs + hlsPlaylistOffsetInSec * 1000;

			// parse times
			auto const sepLen = strlen(sep);
			int h=0, m=0, s=0, ms=0;
			int ret = sscanf(line.substr(0, sepPos).c_str(), "%d:%02d:%02d.%03d", &h, &m, &s, &ms);
			assert(ret == 4);
			startTimeInMs = ((h * 60 + m) * 60 + s) * 1000 + ms;
			h=0, m=0, s=0, ms=0;
			ret = sscanf(line.substr(sepPos + sepLen).c_str(), "%d:%02d:%02d.%03d", &h, &m, &s, &ms);
			assert(ret == 4);
			endTimeInMs = ((h * 60 + m) * 60 + s) * 1000 + ms;

			// offset times
			startTimeInMs = segNum * segmentDurationInMs + hlsPlaylistOffsetInSec * 1000 + (startTimeInMs % segmentDurationInMs);
			endTimeInMs = segNum * segmentDurationInMs + hlsPlaylistOffsetInSec * 1000 + ((endTimeInMs % segmentDurationInMs) ?: segmentDurationInMs);

			const size_t timecodeSize = 24;
			char timecodeShow[timecodeSize] = {};
			timeInMsToStr(startTimeInMs, timecodeShow, ".");
			timecodeShow[timecodeSize - 1] = 0;
			char timecodeHide[timecodeSize] = {};
			timeInMsToStr(endTimeInMs, timecodeHide, ".");
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
