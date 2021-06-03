#include "subtitle_source.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_media/common/metadata.hpp"
#include "lib_media/common/attributes.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/log_sink.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include "lib_utils/xml.hpp"
#include <cassert>
#include <fstream>
#include <thread> //this_thread

Tag parseXml(span<const char> text);

namespace {

using namespace Modules;

class SubtitleSource : public Module {
	public:
		SubtitleSource(KHost* host, SubtitleSourceConfig const& cfg)
			: m_host(host), playlistFn(cfg.subtitleFn), segmentDurationInMs(cfg.segmentDurationInMs),
			  rectify(cfg.rectify), utcStartTime(cfg.utcStartTime),
			  sleepInMs(std::chrono::milliseconds(200)) {
			output = addOutput();

			auto meta = std::make_shared<MetadataPktSubtitle>();
			meta->timeScale = { IClock::Rate, 1 };
			output->setMetadata(meta);

			if (!playlistFn.empty()) {
				std::ifstream file(playlistFn);
				if (!file.is_open())
					m_host->log(Error, format("Can't open subtitle playlist file \"%s\". Start may occur with a delay.", playlistFn).c_str());
				
				playlistDir = playlistFn;
				while(playlistDir.back() != '\\' && playlistDir.back() != '/')
					playlistDir.pop_back();

				processContent = std::bind(&SubtitleSource::processEverGrowingFile, this, std::placeholders::_1);
			} else
				processContent = std::bind(&SubtitleSource::processSynthetic, this, std::placeholders::_1, false);

			m_host->activate(true);
		}

		void process() override {
			ensureStartTime();

			int64_t timestamp = -1;
			auto content = processContent(timestamp);
			if (content.empty()) {
				if (rectify && isLate()) {
					m_host->log(Warning, "Late: inserting empty content");
					// TODO: should we also discard some content if it really arrives afterward?
					content = processSynthetic(timestamp, true);
				} else {
					std::this_thread::sleep_for(sleepInMs);
					return;
				}
			}
			post(content, timestamp);
			numSegment++;
		}

	private:
		void ensureStartTime() {
			if (!startTimeInMs) {
				startTimeInMs = clockToTimescale(utcStartTime->query(), 1000);
				initClockTimeInMs = (int64_t)(g_SystemClock->now() * 1000);
			}
		}

		bool isLate() const {
			if (initClockTimeInMs + (numSegment+1) * segmentDurationInMs >= (uint64_t)(g_SystemClock->now() * 1000))
				return true;
			
			return false;
		}

		void incrementTtmlTimings(Tag &xml, int64_t incrementInMs) {
			for (auto& elt : xml.children) {
				for (auto& attr : elt.attr) {
					if (attr.name == "begin" || attr.name == "end") {
						auto const fmt = "%02d:%02d:%02d.%03d";

						//serialize
						int hour=0, min=0, sec=0, msec=0;
						sscanf(attr.value.c_str(), fmt, &hour, &min, &sec, &msec);

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
						snprintf(buffer, sizeof buffer, fmt, hour, min, sec, msec);
						attr.value = buffer;
					}
				}

				incrementTtmlTimings(elt, incrementInMs);
			}
		}

		std::string processSynthetic(int64_t &timestamp, bool empty = false) const {
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

			timestamp = timescaleToClock(numSegment * (int64_t)segmentDurationInMs, 1000);

			return content;
		}

		std::string processEverGrowingFile(int64_t &timestamp) {
			std::ifstream file(playlistFn);
			if (!file.is_open()) {
				m_host->log(Error, format("Can't open subtitle playlist file \"%s\". Sleeping for %sms.", playlistFn, sleepInMs.count()).c_str());
				return {};
			}
			file.seekg(lastFilePos);

			std::string line;
			if (!std::getline(file, line))
				return {};

			m_host->log(Warning, format("Opening \"%s\"", line).c_str());

			//scan line
			std::string subtitleFn;
			{
				auto const MAX_PATH = 4096;
				char subtitleFnRaw[MAX_PATH];
				int hour, minute, second, ms;
				int ret = sscanf(line.c_str(), "%d:%02d:%02d.%03d,%4095s",
						&hour, &minute, &second, &ms, subtitleFnRaw);
				if(ret != 5) {
					m_host->log(Error, format("Invalid arguments in line \"%s\": will retry in %sms.", line, sleepInMs.count()).c_str());
					return {};
				}

				subtitleFn = subtitleFnRaw;
				timestamp = timescaleToClock((((int64_t)hour * 60 + minute) * 60 + second) * 1000 + ms, 1000);
			}
			subtitleFn = playlistDir + subtitleFn;

			//open file
			std::ifstream ifs(subtitleFn);
			if (!ifs.is_open()) {
				m_host->log(Error, format("Can't open subtitle media file \"%s\": will retry in %sms.", subtitleFn, sleepInMs.count()).c_str());
				return {};
			}

			lastFilePos = lastFilePos + line.size() + 1;
			m_host->log(Warning, format("Current file position: %s, timestamp=%s, media filename=%s\n", (int)lastFilePos, timestamp, subtitleFn).c_str());

			//get size
			auto pbuf = ifs.rdbuf();
			std::size_t size = pbuf->pubseekoff(0, ifs.end, ifs.in);
			pbuf->pubseekpos(0, ifs.in);

			//get data as a buffer
			std::vector<char> input(size);
			pbuf->sgetn(input.data(), size);
			ifs.close();

			//deserialize
			auto ttml = parseXml({ input.data(), input.size() });
			assert(ttml.name == "tt:tt");

			//find timings and increment them
			incrementTtmlTimings(ttml, startTimeInMs);

			//reserialize and return
			return std::string("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n") + serializeXml(ttml);
		}

		void post(const std::string &content, int64_t timestamp) {
			auto const size = content.size();
			auto pkt = output->allocData<DataRaw>(size);
			memcpy(pkt->buffer->data().ptr, content.c_str(), size);

			CueFlags flags{};
			flags.keyframe = true;
			pkt->set(flags);
			pkt->set(DecodingTime{timestamp});
			pkt->setMediaTime(timestamp);

			output->post(pkt);
		}

		KHost *const m_host;
		OutputDefault *output;

		std::string playlistFn, playlistDir;
		const uint64_t segmentDurationInMs;
		bool rectify = false;
		IUtcStartTimeQuery const *utcStartTime;
		
		int64_t startTimeInMs = 0, initClockTimeInMs = 0;
		int numSegment = 0;
		const std::chrono::milliseconds sleepInMs;
		int lastFilePos = 0;
		std::function<std::string(int64_t&)> processContent;
};

IModule* createObject(KHost* host, void* va) {
	auto config = (SubtitleSourceConfig*)va;
	enforce(host, "SubtitleSource: host can't be NULL");
	enforce(config, "SubtitleSource: filename can't be NULL");
	return createModule<SubtitleSource>(host, *config).release();
}

auto const registered = Factory::registerModule("SubtitleSource", &createObject);
}
