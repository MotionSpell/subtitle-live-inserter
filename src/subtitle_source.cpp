#include "subtitle_source.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_media/common/metadata.hpp"
#include "lib_media/common/attributes.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/log_sink.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/time.hpp" //timeInMsToStr
#include <fstream>

extern const uint64_t g_segmentDurationInMs;

namespace {

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
        <span style="Style0_0">%s - %s</span>
      </p>
    </div>
  </body>
</tt>
)|", timecodeShow, timecodeHide, timecodeShow, timecodeHide);

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
			} else {
				std::ifstream file(filename);
				if (!file.is_open())
				{
					m_host->log(Error, format("Can't open subtitle source file \"%s\"", filename).c_str());
					return;
				}
				file.seekg(lastFilePos);

				std::string line;
				if (std::getline(file, line))
				{
					m_host->log(Warning, format("Opening \"%s\"", line).c_str());

					std::ifstream ifs(line);
					if (!ifs.is_open())
						return;

					auto pbuf = ifs.rdbuf();
					std::size_t size = pbuf->pubseekoff(0, ifs.end, ifs.in);
					pbuf->pubseekpos(0, ifs.in);

					auto pkt = output->allocData<DataRaw>(size);
					pkt->set(CueFlags{});
					pkt->set(DecodingTime{timescaleToClock(numSegment * (int64_t)segmentDurationInMs, 1000)});
					pkt->setMediaTime(numSegment * segmentDurationInMs, 1000);
					pbuf->sgetn((char *)pkt->buffer->data().ptr, size);
					output->post(pkt);
					ifs.close();

					lastFilePos = lastFilePos + line.size() + 1;
					m_host->log(Warning, format("Current file position: %s\n", (int)lastFilePos).c_str());
				}
			}

			numSegment++;
		}

	private:
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
