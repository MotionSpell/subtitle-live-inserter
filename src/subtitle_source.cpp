#include "subtitle_source.hpp"
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

std::string processSynthetic(int64_t &timestampIn180k, int segNum, int64_t startTimeInMs, int64_t segmentDurationInMs, bool empty);
std::string processEverGrowingFile(KHost *host, int64_t &timestampIn180k, int segNum, int64_t startTimeInMs, int64_t segmentDurationInMs,
    const std::string &playlistFn, const std::string &playlistDir, int64_t sleepInMs, int lastFilePos, int64_t ebuttdOffsetInMs);

namespace {

static const int maxDelayInMs = 200;

class SubtitleSource : public Module {
	public:
		SubtitleSource(KHost* host, SubtitleSourceConfig const& cfg)
			: m_host(host), playlistFn(cfg.subtitleFn), segmentDurationInMs(cfg.segmentDurationInMs),
			  rectify(cfg.rectify), utcStartTime(cfg.utcStartTime),
			  sleepInMs(std::chrono::milliseconds(maxDelayInMs)) {
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

				processContent = std::bind(&processEverGrowingFile, m_host, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
				        segmentDurationInMs, playlistFn, playlistDir, sleepInMs.count(), lastFilePos, ebuttdOffsetInMs);
			} else {
				// synthetic subtitle fallback
				processContent = std::bind(&processSynthetic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
				        segmentDurationInMs, false);
			}

			m_host->activate(true);
		}

		void process() override {
			ensureStartTime();

			int64_t timestampIn180k = -1;
			auto content = processContent(timestampIn180k, segNum, startTimeInMs);

			if (content.empty()) {
				content = processLate();
				if (content.empty()) {
					std::this_thread::sleep_for(sleepInMs);
					return;
				}
			}

			post(content, timestampIn180k);
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

		std::string processLate() {
			const int64_t diffInMs = (int64_t)(g_SystemClock->now() * 1000) - (initClockTimeInMs + (segNum+1) * segmentDurationInMs);
			if (diffInMs < -maxDelayInMs) {
				if (rectify) {
					m_host->log(Warning, format("Late from %sms. Rectifier activated: inserting empty content", -diffInMs).c_str());
					// TODO: should we also discard some content if it really arrives but afterward?
					int64_t timestampIn180k = -1;
					return processSynthetic(timestampIn180k, segNum, startTimeInMs, segmentDurationInMs, true);
				} else {
					m_host->log(Warning, format("Late from %sms. Sleep for %sms", -diffInMs, sleepInMs.count()).c_str());
				}
			}
			return "";
		}

		KHost *const m_host;
		OutputDefault *output;

		const std::string playlistFn;
		std::string playlistDir;
		const uint64_t segmentDurationInMs;
		const bool rectify = false;
		IUtcStartTimeQuery const *utcStartTime;

		int64_t startTimeInMs = 0, initClockTimeInMs = 0;
		int64_t ebuttdOffsetInMs = 0;
		int segNum = 0;
		const std::chrono::milliseconds sleepInMs;
		int lastFilePos = 0;
		std::function<std::string(int64_t&, int, int64_t)> processContent;
};

IModule* createObject(KHost* host, void* va) {
	auto config = (SubtitleSourceConfig*)va;
	enforce(host, "SubtitleSource: host can't be NULL");
	enforce(config, "SubtitleSource: filename can't be NULL");
	return createModule<SubtitleSource>(host, *config).release();
}

auto const registered = Factory::registerModule("SubtitleSource", &createObject);
}
