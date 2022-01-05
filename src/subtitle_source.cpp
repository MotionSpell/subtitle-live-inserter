#include "subtitle_source.hpp"
#include "subtitle_source_interface.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_media/common/metadata.hpp"
#include "lib_media/common/attributes.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/log_sink.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/system_clock.hpp"
#include <fstream>
#include <thread> //this_thread

using namespace Modules;

namespace {

static const int maxDelayInMs = 200;

class SubtitleSource : public Module {
	public:
		SubtitleSource(KHost* host, SubtitleSourceConfig const& cfg)
			: m_host(host), playlistFn(cfg.subtitleFn), segmentDurationInMs(cfg.segmentDurationInMs),
			  rectify(cfg.rectify), ttml(cfg.format == "ttml"), utcStartTime(cfg.utcStartTime),
			  sleepInMs(std::chrono::milliseconds(maxDelayInMs)) {
			output = addOutput();

			auto meta = std::make_shared<MetadataPktSubtitle>();
			meta->timeScale = { IClock::Rate, 1 };
			output->setMetadata(meta);

			if (!playlistFn.empty()) {
				generator = std::make_unique<SubtitleSourceProcessorEverGrowingFile>(m_host, playlistFn, segmentDurationInMs, sleepInMs.count());
			} else {
				// synthetic subtitle fallback
				if (ttml)
					generator = std::make_unique<SubtitleSourceProcessorSyntheticTtml>(segmentDurationInMs);
				else
					generator = std::make_unique<SubtitleSourceProcessorSyntheticWebvtt>(segmentDurationInMs);
			}

			m_host->activate(true);
		}

		void process() override {
			ensureStartTime();

			auto content = generator->process(startTimeInMs, segNum);

			if (content.text.empty()) {
				content = processLate();
				if (content.text.empty()) {
					std::this_thread::sleep_for(sleepInMs);
					return;
				}
			}

			post(content.text, content.timestampIn180k);
			segNum++;
		}

	private:
		void ensureStartTime() {
			if (!startTimeInMs) {
				startTimeInMs = clockToTimescale(utcStartTime->query(), 1000);
				initClockTimeInMs = (int64_t)(g_SystemClock->now() * 1000);
			}
		}

		void post(const std::string &content, int64_t timestampIn180k) {
			auto const size = content.size();
			auto pkt = output->allocData<DataRaw>(size);
			memcpy(pkt->buffer->data().ptr, content.c_str(), size);

			CueFlags flags{};
			flags.keyframe = true;
			pkt->set(flags);
			pkt->set(DecodingTime{timestampIn180k});
			pkt->setMediaTime(timestampIn180k);

			output->post(pkt);
		}

		ISubtitleSourceProcessor::Result processLate() {
			const int64_t diffInMs = (int64_t)(g_SystemClock->now() * 1000) - (initClockTimeInMs + (segNum+1) * segmentDurationInMs);
			if (diffInMs > maxDelayInMs) {
				if (rectify) {
					m_host->log(Warning, format("Late from %sms. Rectifier activated: inserting empty content", diffInMs).c_str());

					// TODO: should we also discard some content if it really arrives but afterward?
					if (ttml)
						return SubtitleSourceProcessorSyntheticTtml::generate(startTimeInMs, segNum, segmentDurationInMs, true);
					else
						return SubtitleSourceProcessorSyntheticWebvtt::generate(startTimeInMs, segNum, segmentDurationInMs, true);
				} else {
					m_host->log(Warning, format("Data is late from %sms. Sleep for %sms.", diffInMs, sleepInMs.count()).c_str());
				}
			}
			return { "", -1 };
		}

		KHost *const m_host;
		OutputDefault *output;

		const std::string playlistFn;
		const uint64_t segmentDurationInMs;
		const bool rectify;
		const bool ttml;
		IUtcStartTimeQuery const *utcStartTime;

		int64_t startTimeInMs = 0, initClockTimeInMs = 0;
		int segNum = 0;
		const std::chrono::milliseconds sleepInMs;
		std::unique_ptr<ISubtitleSourceProcessor> generator;
};

IModule* createObject(KHost* host, void* va) {
	auto config = (SubtitleSourceConfig*)va;
	enforce(host, "SubtitleSource: host can't be NULL");
	enforce(config, "SubtitleSource: filename can't be NULL");
	return createModule<SubtitleSource>(host, *config).release();
}

auto const registered = Factory::registerModule("SubtitleSource", &createObject);
}
