#include "lib_modules/core/module.hpp"
#include "lib_modules/core/buffer.hpp" //span
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/xml.hpp"
#include <cassert>

Tag parseXml(span<const char> text);

//FIXME: this is inaccurate because it makes the assumption that the first found attribute "begin"/"end" would match with a segment boundary
//       this will be fixed when we know which absolute time is associated with which playlist entry
static int64_t probeTtmlTimings(Tag &xml, int64_t referenceTimeInMs, uint64_t segmentDurationInMs) {
	for (auto& elt : xml.children) {
		for (auto& attr : elt.attr) {
			if (attr.name == "begin") {
				//serialize
				int hour=0, min=0, sec=0, msec=0;
				auto const fmt = "%d:%02d:%02d.%03d";
				sscanf(attr.value.c_str(), fmt, &hour, &min, &sec, &msec);
				int64_t timestampInMs = msec + 1000 * (sec + 60 * (min + 60 * (int64_t)hour));

				//some inputs contain a media timestamp offset (e.g. local time of the day...)
				//begin/end times should start at referenceTimeInMs
				if (attr.name == "begin")
					return referenceTimeInMs - timestampInMs;
			}
		}

		auto offsetInMs = probeTtmlTimings(elt, referenceTimeInMs, segmentDurationInMs);
		if (offsetInMs)
			return offsetInMs;
	}

	return 0;
}

int64_t getTtmlMediaOffset(const std::vector<char> &input, int64_t referenceTimeInMs, uint64_t segmentDurationInMs) {
	auto xml = parseXml({ input.data(), input.size() });
	return probeTtmlTimings(xml, referenceTimeInMs, segmentDurationInMs);
}

static void incrementTtmlTimings(Modules::KHost *host, Tag &xml, int64_t startTimeInMs) {
	for (auto& elt : xml.children) {
		for (auto& attr : elt.attr) {
			if (attr.name == "begin" || attr.name == "end") {
				//serialize
				int hour=0, min=0, sec=0, msec=0;
				auto const fmt1 = "%d:%02d:%02d.%03d";
				sscanf(attr.value.c_str(), fmt1, &hour, &min, &sec, &msec);
				int64_t timestampInMs = msec + 1000 * (sec + 60 * (min + 60 * (int64_t)hour));

				//increment by UTC start time
				timestampInMs += startTimeInMs;

				msec = timestampInMs % 1000;
				timestampInMs /= 1000;
				sec = timestampInMs % 60;
				timestampInMs /= 60;
				min = timestampInMs % 60;
				timestampInMs /= 60;
				hour = timestampInMs;

				//deserialize
				char buffer[256];
				auto const fmt2 = "%02d:%02d:%02d.%03d";
				snprintf(buffer, sizeof buffer, fmt2, hour, min, sec, msec);
				attr.value = buffer;
			}
		}

		incrementTtmlTimings(host, elt, startTimeInMs);
	}
}

std::string getContentTtml(Modules::KHost *host, const std::vector<char> &input, int64_t startTimeInMs) {
	//deserialize
	auto ttml = parseXml({ input.data(), input.size() });
	assert(ttml.name == "tt" || ttml.name == "tt:tt");

	//find timings and increment them
	incrementTtmlTimings(host, ttml, startTimeInMs);

	//reserialize and return
	return std::string("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n") + serializeXml(ttml, true, false);
}
