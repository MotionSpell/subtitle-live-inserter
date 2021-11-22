#include "hls_webvtt_rephaser.hpp"
#include "lib_media/common/attributes.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_modules/core/connection.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_modules/utils/loader.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "plugins/HlsDemuxer/hls_demux.hpp"
#include "plugins/TsDemuxer/ts_demuxer.hpp"
#include <sstream>

using namespace Modules;

const char *variantPlaylistFn = "index_sub.m3u8";
extern const char *g_appName;
extern const char *g_version;

namespace {

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}

// Compute the phase between the media (first PTS) and the subtitles (WebVTT first timestamp)
// Also handles the variant playlist generation for subtitles
// TODO: also support CMAF
class HlsWebvttRephaser : public ModuleS {
	public:
		HlsWebvttRephaser(KHost* host, HlsWebvttRephaserConfig *cfg) : m_host(host), url(cfg->url), segmentDurationInMs(cfg->segmentDurationInMs) {
			auto meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = variantPlaylistFn;
			outputVariantPlaylist = addOutput();
			outputVariantPlaylist->setMetadata(meta);

			outputSegment = addOutput();
		}

		void processOne(Data data) override {
			if (phase == -1) {
				phase = computePhase(); //TODO: do we need to recompute it from time to time? e.g. after stopping streams at night?
			}
			updatePhaseInWebvttSample(data);
			genVariantPlaylist();
			segNum++;
		}

		int64_t computePhase() {
			int64_t firstPts = INT64_MAX;

			try {
				struct MyHost : NullHostType {
					bool finished = false;
					void activate(bool activated) override {
						if(!activated)
							finished = true;
					}
				};

				MyHost host;

				HlsDemuxConfig hlsCfg;
				hlsCfg.url = url;
				auto hls = loadModule("HlsDemuxer", &host, &hlsCfg);

				TsDemuxerConfig tsCfg;
				tsCfg.pids = {};
				tsCfg.timestampStartsAtZero = false;
				auto ts = loadModule("TsDemuxer", &host, &tsCfg);

				ConnectOutputToInput(hls->getOutput(0), ts->getInput(0));
				ConnectOutput(ts->getOutput(0), [&](Data data) {
					firstPts = data->get<PresentationTime>().time ;
				});

				while (firstPts != INT64_MAX && !host.finished)
					hls->process();

				if (firstPts == INT64_MAX) {
					m_host->log(Error, "Phase couldn't be computed. Set to zero until next iteration. Contact your vendor.");
					firstPts = -1;
				}

				ts->getOutput(0)->disconnect();
			} catch (std::exception const& e) {
				m_host->log(Error, (std::string("error caught while computing phase between media and subtitles: ") + e.what()).c_str());
			}

			//instanciate HLS and grab first PTS
			return firstPts;
		}

		void updatePhaseInWebvttSample(Data data) {
			auto phase = this->phase == -1 ? 0 : this->phase;

			std::string line, output;
			std::stringstream ss((const char*)data->data().ptr);
			while(std::getline(ss, line)) {
				auto addLine = [&]() {
					for (auto c : line)
						output.push_back(c);

					output.push_back('\n');
				};

				if (startsWith(line, "WEBVTT")) {
					addLine();
					output += format("X-TIMESTAMP-MAP=LOCAL:00:00:00.000,MPEGTS:%s\n", phase);
				} else {
					addLine();
				}

			}

			auto const segName = format("subs_%s.vtt", segNum);
			segEntries.push_back(segName);

			auto out = outputSegment->allocData<DataRaw>(output.size());
			auto metadata = make_shared<MetadataFile>(PLAYLIST);
			metadata->filename = segName;
			metadata->filesize = output.size();
			out->setMetadata(metadata);
			memcpy(out->buffer->data().ptr, output.data(), output.size());
			outputSegment->post(out);
		}

		void genVariantPlaylist() {
			std::stringstream variantPl(variantPlaylistFn);
			variantPl << "#EXTM3U\n";
			variantPl << "#EXT-X-VERSION:3\n";
			variantPl << std::string("## Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version + "\n";
			variantPl << "#EXT-X-TARGETDURATION: " << segmentDurationInMs/1000.0 << "\n";
			variantPl << "#EXT-X-MEDIA-SEQUENCE:" << segNum << "\n";
			variantPl << "\n";

			for (auto fn : segEntries) {
				//variantPl << "#EXT-X-PROGRAM-DATE-TIME:2021-11-22T12:53:24.540+02:00" << ",\n";
				variantPl << "#EXTINF:" << segmentDurationInMs/1000.0 << ",\n";
				variantPl << fn << "\n";
			}

			variantPl << "\n";

			auto const variantPlStr = variantPl.str();
			auto out = outputVariantPlaylist->allocData<DataRaw>(variantPlStr.size());
			auto metadata = make_shared<MetadataFile>(PLAYLIST);
			metadata->filename = safe_cast<const MetadataFile>(outputVariantPlaylist->getMetadata())->filename;;
			metadata->filesize = variantPlStr.size();
			out->setMetadata(metadata);
			memcpy(out->buffer->data().ptr, variantPlStr.data(), variantPlStr.size());
			outputVariantPlaylist->post(out);
		}

	private:
		KHost *m_host;
		OutputDefault *outputSegment, *outputVariantPlaylist;
		const std::string url;
		const int segmentDurationInMs;
		int segNum = 0;
		std::vector<std::string> segEntries;
		int64_t phase = -1; // this is the first PTS in HLS corresponding to WebVTT timing zero.
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (HlsWebvttRephaserConfig*)va;
	enforce(host, "HlsWebvttRephaser: host can't be NULL");
	enforce(cfg, "HlsWebvttRephaser: config can't be NULL");
	return createModule<HlsWebvttRephaser>(host, cfg).release();
}

auto const registered = Factory::registerModule("HlsWebvttRephaser", &createObject);

}