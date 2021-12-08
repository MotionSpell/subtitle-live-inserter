#include "subtitle_source_interface.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include "lib_utils/clock.hpp"

const int magicOffsetInSec = 24;

SubtitleSourceProcessorSyntheticWebvtt::SubtitleSourceProcessorSyntheticWebvtt(uint64_t segmentDurationInMs) : segmentDurationInMs(segmentDurationInMs) {
}

ISubtitleSourceProcessor::Result SubtitleSourceProcessorSyntheticWebvtt::generate(int64_t /*startTimeInMs*/, int segNum, int64_t segmentDurationInMs, bool empty) {
	//generate timecode strings
	const size_t timecodeSize = 24;
	char timecodeShow[timecodeSize] = {};
	timeInMsToStr(/*startTimeInMs + */segNum * segmentDurationInMs + magicOffsetInSec, timecodeShow, ".");
	timecodeShow[timecodeSize - 1] = 0;
	char timecodeHide[timecodeSize] = {};
	timeInMsToStr(/*startTimeInMs + */(segNum + 1) * segmentDurationInMs + magicOffsetInSec, timecodeHide, ".");
	timecodeHide[timecodeSize - 1] = 0;
	char timecodeUtc[timecodeSize] = {};
	timeInMsToStr((uint64_t)(getUTC() * 1000), timecodeUtc, ".");

	auto content = format(R"|(WEBVTT

)|");

	if (!empty && segNum * segmentDurationInMs >= 0) {
        content += format(R"|(%s --> %s
[%s - %s (UTC=%s)]

)|", timecodeShow, timecodeHide, timecodeShow, timecodeHide, timecodeUtc);
    }

  auto timestampIn180k = timescaleToClock(segNum * (int64_t)segmentDurationInMs, 1000);
  return { content, timestampIn180k };
}

ISubtitleSourceProcessor::Result SubtitleSourceProcessorSyntheticWebvtt::process(int64_t startTimeInMs, int segNum) {
  return SubtitleSourceProcessorSyntheticWebvtt::generate(startTimeInMs, segNum, segmentDurationInMs, false);
}
