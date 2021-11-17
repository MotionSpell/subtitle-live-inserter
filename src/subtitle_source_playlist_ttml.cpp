#include "lib_modules/core/module.hpp"
#include "lib_modules/core/buffer.hpp" //span
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/xml.hpp"
#include <cassert>

Tag parseXml(span<const char> text);

static void incrementTtmlTimings(Tag &xml, int64_t referenceTimeInMs, int64_t incrementInMs, int64_t segmentDurationInMs, int64_t ebuttdOffsetInMs, Modules::KHost *host) {
	for (auto& elt : xml.children) {
		for (auto& attr : elt.attr) {
			if (attr.name == "begin" || attr.name == "end") {
				auto const fmt = "%02d:%02d:%02d.%03d";

				//serialize
				int hour=0, min=0, sec=0, msec=0;
				sscanf(attr.value.c_str(), fmt, &hour, &min, &sec, &msec);
				int64_t timestampInMs = msec + 1000 * (sec + 60 * (min + 60 * hour));

				//some inputs contain a media timestampIn180k offset (e.g. local time of the day...)
				//begin/end times should be less than referenceTimeInMs + segmentDurationInMs
				int64_t offsetInMs = 0;
				if (attr.name == "begin" && timestampInMs < referenceTimeInMs)
					offsetInMs = referenceTimeInMs - timestampInMs;
				else if (attr.name == "end" && timestampInMs > referenceTimeInMs + (int)segmentDurationInMs)
					offsetInMs = referenceTimeInMs + (int)segmentDurationInMs - timestampInMs;

				if (offsetInMs) {
					if (ebuttdOffsetInMs == 0) {
						host->log(Warning, format("Dealing with a media offset of %sms. Media times are expected to be zero-based.", ebuttdOffsetInMs).c_str());
						ebuttdOffsetInMs = offsetInMs;
					} else if (ebuttdOffsetInMs != offsetInMs) {
						host->log(Error, format("Media offset change detected: %sms -> %sms.", ebuttdOffsetInMs, offsetInMs).c_str());
						ebuttdOffsetInMs = offsetInMs;
					}
				}

				timestampInMs += ebuttdOffsetInMs;

				//increment
				timestampInMs += incrementInMs;
				msec = timestampInMs % 1000;
				timestampInMs /= 1000;
				sec = timestampInMs % 60;
				timestampInMs /= 60;
				min = timestampInMs % 60;
				timestampInMs /= 60;
				hour = timestampInMs;

				//deserialize
				char buffer[256];
				snprintf(buffer, sizeof buffer, fmt, hour, min, sec, msec);
				attr.value = buffer;
			}
		}

		incrementTtmlTimings(elt, referenceTimeInMs, incrementInMs, segmentDurationInMs, ebuttdOffsetInMs, host);
	}
}

std::string getContentTtml(const std::vector<char> &input, int64_t referenceTimeInMs, uint64_t segmentDurationInMs, int64_t startTimeInMs, int64_t ebuttdOffsetInMs, Modules::KHost *host) {
	//deserialize
	auto ttml = parseXml({ input.data(), input.size() });
	assert(ttml.name == "tt:tt");

	//find timings and increment them
	incrementTtmlTimings(ttml, referenceTimeInMs, startTimeInMs, segmentDurationInMs, ebuttdOffsetInMs, host);

	//reserialize and return
	return std::string("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n") + serializeXml(ttml);
}
