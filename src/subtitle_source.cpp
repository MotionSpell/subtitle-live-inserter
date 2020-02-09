#include "subtitle_source.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_media/common/metadata.hpp"
#include "lib_media/common/attributes.hpp"
#include "lib_media/common/sax_xml_parser.hpp"
#include "lib_media/common/xml.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/log_sink.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include <cassert>
#include <fstream>
#include <thread> //this_thread

extern const uint64_t g_segmentDurationInMs;

namespace {

//FIXME: duplicate from redash
Tag parseXml(span<const char> text) {
	Tag root;
	std::vector<Tag*> tagStack = { &root };

	auto onNodeStart = [&](std::string name, std::map<std::string, std::string> &attr) {
		Tag tag{name};

		for (auto &a : attr)
			tag.attr.push_back({a.first, a.second});

		tagStack.back()->add(tag);
		tagStack.push_back(&tagStack.back()->children.back());
	};

	auto onNodeEnd = [&](std::string /*id*/, std::string content) {
		tagStack.back()->content = content;
		tagStack.pop_back();
	};

	saxParse(text, onNodeStart, onNodeEnd);

	assert(tagStack.front()->children.size() == 1);
	return tagStack.front()->children[0];
}

using namespace Modules;

class SubtitleSource : public Module {
	public:
		SubtitleSource(KHost* host, SubtitleSourceConfig const& cfg)
			: m_host(host), filename(cfg.filename), segmentDurationInMs(cfg.segmentDurationInMs), utcStartTime(cfg.utcStartTime) {
			output = addOutput();

			auto meta = std::make_shared<MetadataPktSubtitle>();
			meta->timeScale = { IClock::Rate, 1 };
			output->setMetadata(meta);
			
			if (!filename.empty()) {
				std::ifstream file(filename);
				if (!file.is_open())
					throw error(format("Can't open subtitle source file \"%s\"", filename).c_str());
			}

			m_host->activate(true);
		}

		void process() override {
			if (!startTimeInMs)
				startTimeInMs = clockToTimescale(utcStartTime->query(), 1000);

			if (filename.empty()) {
				//generate timecode strings
				const size_t timecodeSize = 24;
				char timecodeShow[timecodeSize] = {};
				timeInMsToStr(startTimeInMs + numSegment * segmentDurationInMs, timecodeShow, ".");
				timecodeShow[timecodeSize - 1] = 0;
				char timecodeHide[timecodeSize] = {};
				timeInMsToStr(startTimeInMs + (numSegment + 1) * segmentDurationInMs, timecodeHide, ".");
				timecodeHide[timecodeSize - 1] = 0;
				char timecodeUtc[timecodeSize] = {};
				timeInMsToStr((uint64_t)(getUTC() * 1000), timecodeUtc, ".");

				//generate samples
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
    <div>
      <p region="Region" style="textAlignment_0" begin="%s" end="%s" xml:id="sub_0">
        <span style="Style0_0">IRT/GPAC-Licensing live subtitle inserter:</span>
        <br/>
        <span style="Style0_0">%s - %s (UTC=%s)</span>
      </p>
    </div>
  </body>
</tt>
)|", timecodeShow, timecodeHide, timecodeShow, timecodeHide, timecodeUtc);

				//send sample
				auto const size = content.size();
				auto pkt = output->allocData<DataRaw>(size);
				memcpy(pkt->buffer->data().ptr, content.c_str(), size);
				CueFlags flags{};
				flags.keyframe = true;
				pkt->set(flags);
				pkt->set(DecodingTime{timescaleToClock(numSegment * (int64_t)segmentDurationInMs, 1000)});
				pkt->setMediaTime(numSegment * segmentDurationInMs, 1000);
				output->post(pkt);

				numSegment++;
			} else {
				auto const sleepInMs = 200;
				std::ifstream file(filename);
				if (!file.is_open())
				{
					m_host->log(Error, format("Can't open subtitle playlist file \"%s\". Sleeping for %sms.", filename, sleepInMs).c_str());
					std::this_thread::sleep_for(std::chrono::milliseconds(sleepInMs));
					return;
				}
				file.seekg(lastFilePos);

				std::string line;
				if (std::getline(file, line))
				{
					m_host->log(Warning, format("Opening \"%s\"", line).c_str());

					//scan line
					auto const MAX_PATH = 4096;
					char filename[MAX_PATH];
					int hour, minute, second, ms;
					int ret = sscanf(line.c_str(), "%02d:%02d:%02d.%03d,%4095s",
							&hour, &minute, &second, &ms, filename);
					if(ret != 5)
					{
						m_host->log(Error, format("Invalid timing in line \"%s\": will retry in %sms.", line, sleepInMs).c_str());
						std::this_thread::sleep_for(std::chrono::milliseconds(sleepInMs));
						return;
					}

					//open file
					std::ifstream ifs(filename);
					if (!ifs.is_open())
					{
						m_host->log(Error, format("Can't open subtitle media file \"%s\": will retry in %sms.", filename, sleepInMs).c_str());
						std::this_thread::sleep_for(std::chrono::milliseconds(sleepInMs));
						return;
					}

					auto pbuf = ifs.rdbuf();
					std::size_t size = pbuf->pubseekoff(0, ifs.end, ifs.in);
					pbuf->pubseekpos(0, ifs.in);

					//get a buffer
					std::vector<char> input(size);
					pbuf->sgetn(input.data(), size);

					//deserialize
					auto ttml = parseXml({ input.data(), size });
					assert(ttml.name == "tt:tt");

					//find timings and increment them
					incrementTtmlTimings(ttml, startTimeInMs);

					//reserialize
					auto const newTtml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + serializeXml(ttml);
					auto const newTtmlSize = newTtml.size();
					auto pkt = output->allocData<DataRaw>(newTtmlSize);
					memcpy(pkt->buffer->data().ptr, newTtml.c_str(), newTtmlSize);

					CueFlags flags{};
					flags.keyframe = true;
					pkt->set(flags);
					auto timestamp = timescaleToClock((((int64_t)hour * 60 + minute) * 60 + second) * 1000 + ms, 1000);
					pkt->set(DecodingTime{timestamp});
					pkt->setMediaTime(timestamp);
					output->post(pkt);
					ifs.close();

					lastFilePos = lastFilePos + line.size() + 1;
					m_host->log(Warning, format("Current file position: %s\n", (int)lastFilePos).c_str());

					numSegment++;
				}
			}
		}

	private:
		void incrementTtmlTimings(Tag &xml, int64_t incrementInMs) {
			for (auto& elt : xml.children) {
				for (auto& attr : elt.attr) {
					if (attr.name == "begin" || attr.name == "end") {
						//serialize
						int hour, min, sec, msec;
						int ret = sscanf(attr.value.c_str(), "%02d:%02d:%02d.%03d", &hour, &min, &sec, &msec);
						assert(ret == 4);

						//increment
						auto totalInMs = incrementInMs + msec + 1000 * (sec + 60 * (min + 60 * hour));
						msec = totalInMs % 1000;
						totalInMs /= 1000;
						sec = totalInMs % 60;
						totalInMs /= 60;
						min = totalInMs % 60;
						totalInMs /= 60;
						hour = totalInMs;

						//deserialize
						char buffer[256];
						snprintf(buffer, sizeof buffer, "%02d:%02d:%02d.%03d", hour, min, sec, msec);
						attr.value = buffer;
					}
				}

				incrementTtmlTimings(elt, incrementInMs);
			}
		}

		KHost *const m_host;
		std::string filename;
		OutputDefault *output;
		const uint64_t segmentDurationInMs;
		int numSegment = 0;
		int64_t startTimeInMs = 0;
		IUtcStartTimeQuery const *utcStartTime;
		int lastFilePos = 0;
};

IModule* createObject(KHost* host, void* va) {
	auto config = (SubtitleSourceConfig*)va;
	enforce(host, "SubtitleSource: host can't be NULL");
	enforce(config, "SubtitleSource: filename can't be NULL");
	return createModule<SubtitleSource>(host, *config).release();
}

auto const registered = Factory::registerModule("SubtitleSource", &createObject);
}
