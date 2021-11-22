//Romain: #include "rehls.hpp"
#if 1 //Romain
#include "re_dash.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/tools.hpp" //enforce
#include <fstream>
#include <sstream>
#include <thread>

using namespace Modules;
using namespace Modules::In;

extern const char *g_appName;
extern const char *g_version;

namespace {

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}

//TODO: Romain: rename to re_hls.cpp and re_dash.cpp
class ReHLS : public Module {
	public:
		ReHLS(KHost* host, ReDashConfig *cfg)
			: m_host(host), url(cfg->url), baseUrl(cfg->baseUrl), segmentDurationInMs(cfg->segmentDurationInMs),
			  httpSrc(cfg->filePullerFactory->create()), nextAwakeTime(g_SystemClock->now()) {
			std::string urlFn = cfg->manifestFn.empty() ? url : cfg->manifestFn;
			auto i = urlFn.rfind('/');
			if(i != urlFn.npos)
				urlFn = urlFn.substr(i+1, urlFn.npos);

			auto meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = urlFn;
			outputMaster = addOutput();
			outputMaster->setMetadata(meta);

			meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = variantPlaylistFn;
			outputVariant = addOutput();
			outputVariant->setMetadata(meta);

			m_host->activate(true);
		}

		void process() override {
			// is it time?
			if (g_SystemClock->now() < nextAwakeTime) {
				auto const sleepInMs = 200;
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepInMs));
				return;
			}

			updateMasterPlaylist();
			createVariantPlaylist();

#if 0
			//Romain: out of scope here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (phase == -1) {}
			phase = computePhase(); //Romain: do we need to recompute it from time to time?
		}
		updatePhase();
		//updateTimings(); //?? //Ensure times are withing the segment duration? (e.g. not reset?) -> we need at least to be compatible
#endif

		auto const sleepTimeInSec = 1; //TODO: provide more accurate value
		nextAwakeTime = g_SystemClock->now() + sleepTimeInSec;
}

private:
void updateMasterPlaylist() {
	auto const m3u8MasterAsText = download(httpSrc.get(), url.c_str());
	if (m3u8MasterAsText.empty())
		throw std::runtime_error("can't get master m3u8");

	//add subtitle to video + make sure URLs are absolute
	std::string line, m3u8MasterNew;
	std::stringstream ss(std::string(m3u8MasterAsText.begin(), m3u8MasterAsText.end()));
	while(std::getline(ss, line)) {
		auto empty = [&]() {
			return line.empty();
		};
		auto comment = [&]() {
			return line.size() > 2 && line[0] == '#' && line[1] == '#';
		};
		auto addLine = [&]() {
			for (auto c : line)
				m3u8MasterNew.push_back(c);
		};

		if(empty() || comment()) {
			addLine();
			m3u8MasterNew.push_back('\n');
			continue;
		}

		auto ensureAbsoluteUrl = [&]() {
			if (startsWith(line, "http"))
				return;

			auto i = url.rfind('/');
			auto const baseURL = url.substr(0, i);

			for (auto c : baseURL)
				m3u8MasterNew.push_back(c);

			if (!startsWith(line, "/"))
				m3u8MasterNew.push_back('/');
		};

		if (line[0] == '#') {
			//make sure URLs are absolute
			auto const pattern = "URI=\"";
			auto const pos = line.find(pattern);
			if (pos != std::string::npos) {
				size_t i = 0;
				for ( ; i < pos + strlen(pattern); ++i)
					m3u8MasterNew.push_back(line[i]);

				ensureAbsoluteUrl();

				for ( ; i < line.size(); ++i)
					m3u8MasterNew.push_back(line[i]);
			} else {
				addLine();
			}

			//add subtitle to video
			if (line.find("RESOLUTION=") != std::string::npos) {
				std::string suffix(",SUBTITLES=\"subtitles\"");
				for (auto c : suffix)
					m3u8MasterNew.push_back(c);
			}
		} else {
			ensureAbsoluteUrl();
			addLine();
		}

		m3u8MasterNew.push_back('\n');
	}

	m3u8MasterNew.push_back('\n');

	auto const author = std::string("## Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version + "\n";
	for (auto c : author)
		m3u8MasterNew.push_back(c);

	//add a final entry to the master playlist
	auto const subVariant = format("#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID=\"subtitles\",NAME=\"subtitles\",LANGUAGE=\"de\",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI=\"%s/%s\"\n", baseUrl, variantPlaylistFn);
	for (auto c : subVariant)
		m3u8MasterNew.push_back(c);

	postManifest(outputMaster, m3u8MasterNew);
}

void createVariantPlaylist() {
	std::ofstream variantPl(variantPlaylistFn, std::ofstream::out);
	variantPl << "#EXTM3U\n";
	variantPl << "#EXT-X-VERSION:3\n";
	variantPl << std::string("## Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version + "\n";
	variantPl << "#EXT-X-TARGETDURATION: " << segmentDurationInMs/1000.0 << "\n";
	variantPl << "\n"; //Romain: #EXT-X-MEDIA-SEQUENCE:24098
	variantPl << "#EXT-X-PLAYLIST-TYPE:VOD\n";
	variantPl << "\n";
#if 0 //Romain
	for (auto i : entries) {
		variantPl << "#EXTINF:" << segmentDurationInMs/1000.0 << ",\n";
		variantPl << fn << "\n";
	}
#endif
	variantPl << "\n";
#if 0 //Romain
#EXTINF:5.760,
	segmentv_24106.ts
#endif
	variantPl.close();
}

void postManifest(OutputDefault *output, const std::string &contents) {
	auto out = output->allocData<DataRaw>(contents.size());
	auto metadata = make_shared<MetadataFile>(PLAYLIST);
	metadata->filename = safe_cast<const MetadataFile>(output->getMetadata())->filename;;
	metadata->filesize = contents.size();
	out->setMetadata(metadata);
	memcpy(out->buffer->data().ptr, contents.data(), contents.size());
	output->post(out);
}

#if 0 //Romain
int64_t computePhase() {
	//instanciate HLS and grab first PTS
	return 0;
}

void updatePhase() {
	// WEBVTT
	// X-TIMESTAMP-MAP=LOCAL:00:00:00.000,MPEGTS:0  <-- change MPEG-TS phase
}
#endif

KHost *m_host;
OutputDefault *outputMaster, *outputVariant;
const char * variantPlaylistFn = "index_sub.m3u8";
const std::string url, baseUrl;
const int segmentDurationInMs;
std::unique_ptr<IFilePuller> httpSrc;
Fraction nextAwakeTime;
//int64_t phase = -1; // this is the first PTS in HLS corresponding to WebVTT timing zero.
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (ReDashConfig*)va;
	enforce(host, "reHLS: host can't be NULL");
	enforce(cfg, "reHLS: config can't be NULL");
	return createModule<ReHLS>(host, cfg).release();
}

auto const registered = Factory::registerModule("reHLS", &createObject);
}

#endif

