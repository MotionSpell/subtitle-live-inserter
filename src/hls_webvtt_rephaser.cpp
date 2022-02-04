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
#include <cassert>
#include <ctime> //gmtime
#include <list>
#include <sstream>

using namespace Modules;

const char *variantPlaylistFn = "index_sub.m3u8";
extern const char *g_appName;
extern const char *g_version;

namespace {

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}

std::string formatDate(int64_t timestamp) {
	auto t = (time_t)timestamp;
	std::tm date = *std::gmtime(&t);

	char buffer[256];
	sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ",
	    1900 + date.tm_year,
	    1 + date.tm_mon,
	    date.tm_mday,
	    date.tm_hour,
	    date.tm_min,
	    date.tm_sec);
	return buffer;
}

// Compute the phase between the TS media (first PTS) and the subtitles (WebVTT first timestamp)
// Also handles the variant playlist generation for subtitles
class HlsWebvttRephaser : public ModuleS {
	public:
		HlsWebvttRephaser(KHost* host, HlsWebvttRephaserConfig *cfg)
			: m_host(host), url(cfg->url), segmentDurationInMs(cfg->segmentDurationInMs) {
			auto meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = variantPlaylistFn;
			outputVariantPlaylist = addOutput();
			outputVariantPlaylist->setMetadata(meta);

			outputSegment = addOutput();
		}

		void processOne(Data data) override {
			if (sourceInfo.firstPtsIn90k == -1) {
				sourceInfo = getSourceInfo(); //TODO: do we need to recompute it from time to time? e.g. after stopping streams at night?
			}
			updatePhaseInWebvttSample(data);
			genVariantPlaylist();
			segNum++;
		}

	private:
		struct SourceInfo {
			int64_t firstPtsIn90k = -1; // this is the first PTS in HLS corresponding to WebVTT timing zero.
			int64_t programDateTimeIn180k = -1; // absolute time from the source: may differe significantly from UTC times
		};

		SourceInfo getSourceInfo() {
			try {
				// retrieve HLS segments timestamps
				std::vector<int64_t> hlsSegTimes;
				Data lastHlsSegment;
				HlsDemuxConfig hlsCfg;
				hlsCfg.url = url;
				auto hls = loadModule("HlsDemuxer", m_host, &hlsCfg);
				ConnectOutput(hls->getOutput(0/*arbitrary*/), [&](Data data) {
					hlsSegTimes.push_back(data->get<PresentationTime>().time);
					lastHlsSegment = data;
				});
				hls->process();

				if (hlsSegTimes.empty())
					return { -1, 0 }; // retry later

				// compute first PTS of last segment
				int64_t firstPtsOfLastSegIn90k = INT64_MAX;
				TsDemuxerConfig tsCfg;
				tsCfg.pids = { TsDemuxerConfig::ANY_VIDEO() };
				tsCfg.timestampStartsAtZero = false;
				auto ts = loadModule("TsDemuxer", m_host, &tsCfg);
				ConnectOutput(ts->getOutput(0), [&](Data data) {
					firstPtsOfLastSegIn90k = clockToTimescale(data->get<PresentationTime>().time, 90000);
				});
				ts->getInput(0)->push(lastHlsSegment);

				if (firstPtsOfLastSegIn90k == INT64_MAX) {
					m_host->log(Error, "Phase couldn't be computed. Set to zero until next iteration. Contact your vendor.");
					firstPtsOfLastSegIn90k = 0;
				}

				/* "If any Media Playlist in a Master Playlist contains an EXT-X-PROGRAM-DATE-TIME tag, then all
				    Media Playlists in that Master Playlist MUST contain EXT-X-PROGRAM-DATE-TIME tags with consistent mappings
				    of date and time to media timestamps." */
				int64_t programDateTimeIn180k = 0;
				if (hlsSegTimes[0] > timescaleToClock((int64_t)1 << 33, 90000)) {
					programDateTimeIn180k = hlsSegTimes[0];
				}

				auto const playlistDur = hlsSegTimes.back() - hlsSegTimes.front() + 8 * IClock::Rate; //Romain: + segmentDurationInMs/*FIXME: should be the same duration as the other media*/;
				//Romain: we could simply use the last PTS of the last segment?

				// fill the timeshiftBuffer
				assert(segNum == 0);
				timeshiftBufferDepthInSeg = (int)(playlistDur / timescaleToClock(segmentDurationInMs, 1000));
				while (segNum < timeshiftBufferDepthInSeg + 1) { //Romain: was -1
					auto const segName = format("subs_%s.vtt", segNum);
					segEntries.push_back(segName); // we push the entry in the playlist but not the file
					segNum++;
				}

				auto firstPtsIn90k = firstPtsOfLastSegIn90k - clockToTimescale(playlistDur, 90000) + rescale(segmentDurationInMs, 1000, 90000)/*Romain*/;
				while (firstPtsIn90k < 0) firstPtsIn90k += ((int64_t)1 << 33);

				return { firstPtsIn90k, programDateTimeIn180k };
			} catch (std::exception const& e) {
				m_host->log(Error, (std::string("error caught while computing phase between media and subtitles: ") + e.what()).c_str());
				return { -1, 0 }; // retry later
			}
		}

		void updatePhaseInWebvttSample(Data data) {
			auto const phase = sourceInfo.firstPtsIn90k == -1 ? 0 : sourceInfo.firstPtsIn90k;

			std::string line, output;
			std::stringstream ss((const char*)data->data().ptr);
			while(std::getline(ss, line)) {
				auto addLine = [&]() {
					output += line;
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
			if (timeshiftBufferDepthInSeg > 0) {
				int toRemove = segEntries.size() - timeshiftBufferDepthInSeg;
				while (toRemove-- > 0)
					segEntries.pop_front();
			}

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
			variantPl << "#EXT-X-MEDIA-SEQUENCE:" << (timeshiftBufferDepthInSeg ? segNum - timeshiftBufferDepthInSeg : 0) << "\n";
			variantPl << "\n";

			int entryNum = segNum - timeshiftBufferDepthInSeg;
			for (auto &fn : segEntries) {
				if (sourceInfo.programDateTimeIn180k > 0)
					variantPl << "#EXT-X-PROGRAM-DATE-TIME:" << formatDate(sourceInfo.programDateTimeIn180k / IClock::Rate + entryNum * segmentDurationInMs / 1000) << "\n";
				variantPl << "#EXTINF:" << segmentDurationInMs/1000.0 << ",\n";
				variantPl << fn << "\n";
				entryNum++;
			}

			variantPl << "\n";

			auto const variantPlStr = variantPl.str();
			auto out = outputVariantPlaylist->allocData<DataRaw>(variantPlStr.size());
			auto metadata = make_shared<MetadataFile>(PLAYLIST);
			metadata->filename = safe_cast<const MetadataFile>(outputVariantPlaylist->getMetadata())->filename;
			metadata->filesize = variantPlStr.size();
			out->setMetadata(metadata);
			memcpy(out->buffer->data().ptr, variantPlStr.data(), variantPlStr.size());
			outputVariantPlaylist->post(out);
		}

		KHost * const m_host;
		OutputDefault *outputSegment, *outputVariantPlaylist;
		const std::string url;
		const int segmentDurationInMs;
		//Romain: a client MUST use the relative position of each segment on the Playlist timeline
		// Romain: at the moment the A/V sn are in advance and subs are late. Related?
		// TODO: should be the same as for audio and video
		int timeshiftBufferDepthInSeg = 0;
		int segNum = timeshiftBufferDepthInSeg;
		std::list<std::string> segEntries;
		SourceInfo sourceInfo;
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (HlsWebvttRephaserConfig*)va;
	enforce(host, "HlsWebvttRephaser: host can't be NULL");
	enforce(cfg, "HlsWebvttRephaser: config can't be NULL");
	return createModule<HlsWebvttRephaser>(host, cfg).release();
}

auto const registered = Factory::registerModule("HlsWebvttRephaser", &createObject);

}