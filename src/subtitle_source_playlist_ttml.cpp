#include "lib_modules/core/module.hpp"
#include "lib_modules/core/buffer.hpp" //span
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/xml.hpp"
#include <cassert>

Tag parseXml(span<const char> text);

//FIXME: this is inaccurate because it makes the assumption that the first found attribute "begin"/"end" would match with a segment boundary
static int64_t probeTtmlTimings(Tag &xml, int64_t referenceTimeInMs, uint64_t segmentDurationInMs) {
	for (auto& elt : xml.children) {
		for (auto& attr : elt.attr) {
			if (attr.name == "begin" || attr.name == "end") {
				//serialize
				int hour=0, min=0, sec=0, msec=0;
				auto const fmt = "%d:%02d:%02d.%03d";
				sscanf(attr.value.c_str(), fmt, &hour, &min, &sec, &msec);
				int64_t timestampInMs = msec + 1000 * (sec + 60 * (min + 60 * (int64_t)hour));

				//some inputs contain a media timestamp offset (e.g. local time of the day...)
				//begin/end times should start at referenceTimeInMs and last segmentDurationInMs
				if (attr.name == "begin" && timestampInMs < referenceTimeInMs)
					return referenceTimeInMs - timestampInMs;
				else if (attr.name == "end" && timestampInMs > referenceTimeInMs + (int)segmentDurationInMs)
					return referenceTimeInMs + (int)segmentDurationInMs - timestampInMs;
			}
		}

		auto ret = probeTtmlTimings(elt, referenceTimeInMs, segmentDurationInMs);
		if (ret)
			return ret;
	}

	return 0;
}

int64_t getTtmlMediaOffset(const std::vector<char> &input, int64_t referenceTimeInMs, uint64_t segmentDurationInMs) {
	auto xml = parseXml({ input.data(), input.size() });
	return probeTtmlTimings(xml, referenceTimeInMs, segmentDurationInMs);
}

static void incrementTtmlTimings(Modules::KHost *host, Tag &xml, int segNum, int64_t segmentDurationInMs, int64_t startTimeInMs) {
	for (auto& elt : xml.children) {
		for (auto& attr : elt.attr) {
			if (attr.name == "begin" || attr.name == "end") {
				//serialize
				int hour=0, min=0, sec=0, msec=0;
				auto const fmt1 = "%d:%02d:%02d.%03d";
				sscanf(attr.value.c_str(), fmt1, &hour, &min, &sec, &msec);
				int64_t timestampInMs = msec + 1000 * (sec + 60 * (min + 60 * (int64_t)hour));

#if 0
				//some inputs contain a media timestamp offset (e.g. local time of the day...)
				//begin/end times should start at referenceTimeInMs and last segmentDurationInMs
				const int64_t referenceTimeInMs = startTimeInMs + segNum * segmentDurationInMs;
				if (attr.name == "begin" && timestampInMs < referenceTimeInMs)
					timestampInMs += referenceTimeInMs - timestampInMs;
				else if (attr.name == "end" && timestampInMs > referenceTimeInMs + (int)segmentDurationInMs)
					timestampInMs += referenceTimeInMs + (int)segmentDurationInMs - timestampInMs;
#endif

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

		incrementTtmlTimings(host, elt, segNum, segmentDurationInMs, startTimeInMs);
	}
}

std::string getContentTtml(Modules::KHost *host, const std::vector<char> &input, int segNum, uint64_t segmentDurationInMs, int64_t startTimeInMs) {
	//deserialize
	auto ttml = parseXml({ input.data(), input.size() });
	assert(ttml.name == "tt" || ttml.name == "tt:tt");

	//find timings and increment them
	incrementTtmlTimings(host, ttml, segNum, segmentDurationInMs, startTimeInMs);

	//reserialize and return
	return std::string("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n") + serializeXml(ttml);
}
