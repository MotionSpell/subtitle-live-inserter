#include "subtitle_source_interface.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include "lib_utils/clock.hpp"

SubtitleSourceProcessorSyntheticTtml::SubtitleSourceProcessorSyntheticTtml(uint64_t segmentDurationInMs) : segmentDurationInMs(segmentDurationInMs) {
}

ISubtitleSourceProcessor::Result SubtitleSourceProcessorSyntheticTtml::generate(int64_t startTimeInMs, int segNum, int64_t segmentDurationInMs, bool empty) {
	//generate timecode strings
	const size_t timecodeSize = 24;
	char timecodeShow[timecodeSize] = {};
	timeInMsToStr(startTimeInMs + segNum * segmentDurationInMs, timecodeShow, ".");
	timecodeShow[timecodeSize - 1] = 0;
	char timecodeHide[timecodeSize] = {};
	timeInMsToStr(startTimeInMs + (segNum + 1) * segmentDurationInMs, timecodeHide, ".");
	timecodeHide[timecodeSize - 1] = 0;
	char timecodeUtc[timecodeSize] = {};
	timeInMsToStr((uint64_t)(getUTC() * 1000), timecodeUtc, ".");

	//generate samples
	std::string timing;

	if (!empty) {
		timing = format(R"|(
      <p region="Region" style="textAlignment_0" begin="%s" end="%s" xml:id="sub_0">
        <span style="Style0_0">IRT/GPAC-Licensing live subtitle inserter:</span>
        <br/>
        <span style="Style0_0">%s - %s (UTC=%s)</span>
      </p>)|",
		        timecodeShow, timecodeHide, timecodeShow, timecodeHide, timecodeUtc);
	}

	auto content = format(R"|(
<?xml version="1.0" encoding="UTF-8" ?>
<tt xmlns="http://www.w3.org/ns/ttml" xmlns:tt="http://www.w3.org/ns/ttml" xmlns:ttm="http://www.w3.org/ns/ttml#metadata" xmlns:tts="http://www.w3.org/ns/ttml#styling" xmlns:ttp="http://www.w3.org/ns/ttml#parameter" xmlns:ebutts="urn:ebu:tt:style" xmlns:ebuttm="urn:ebu:tt:metadata" xml:lang="" ttp:timeBase="media">
  <head>
    <metadata>
      <ebuttm:documentMetadata>
        <ebuttm:conformsToStandard>urn:ebu:tt:distribution:2014-01</ebuttm:conformsToStandard>
      </ebuttm:documentMetadata>
    </metadata>
    <styling>
      <style xml:id="Style0_0" tts:fontFamily="proportionalSansSerif" tts:backgroundColor="#00000099" tts:color="#FFFF00" tts:fontSize="100%%" tts:lineHeight="normal" ebutts:linePadding="0.5c" />
      <style xml:id="textAlignment_0" tts:textAlign="center" />
    </styling>
    <layout>
      <region xml:id="Region" tts:origin="10%% 10%%" tts:extent="80%% 80%%" tts:displayAlign="after" />
    </layout>
  </head>
  <body>
    <div>%s
    </div>
  </body>
</tt>
)|", timing);

  auto timestampIn180k = timescaleToClock(segNum * (int64_t)segmentDurationInMs, 1000);
  return { content, timestampIn180k };
}

ISubtitleSourceProcessor::Result SubtitleSourceProcessorSyntheticTtml::process(int64_t startTimeInMs, int segNum) {
  return SubtitleSourceProcessorSyntheticTtml::generate(startTimeInMs, segNum, segmentDurationInMs, false);
}
