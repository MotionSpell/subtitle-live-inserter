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
extern const char *variantPlaylistFn;

namespace {

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}

// Only handles the master playlist
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
					m3u8MasterNew += line;
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
					m3u8MasterNew += baseURL;

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
					if (line.find("RESOLUTION=") != std::string::npos)
						m3u8MasterNew += ",SUBTITLES=\"subtitles\"";
				} else {
					ensureAbsoluteUrl();
					addLine();
				}

				m3u8MasterNew.push_back('\n');
			}

			m3u8MasterNew.push_back('\n');

			auto const author = std::string("## Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version + "\n";
			m3u8MasterNew += author;

			//add a final entry to the master playlist
			auto const subVariant = format("#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID=\"subtitles\",NAME=\"subtitles\",LANGUAGE=\"de\",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI=\"%s/%s\"\n", baseUrl, variantPlaylistFn);
			m3u8MasterNew += subVariant;

			postManifest(outputMaster, m3u8MasterNew);
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

		KHost *m_host;
		OutputDefault *outputMaster;
		const std::string url, baseUrl;
		const int segmentDurationInMs;
		std::unique_ptr<IFilePuller> httpSrc;
		Fraction nextAwakeTime;
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (ReDashConfig*)va;
	enforce(host, "reHLS: host can't be NULL");
	enforce(cfg, "reHLS: config can't be NULL");
	return createModule<ReHLS>(host, cfg).release();
}

auto const registered = Factory::registerModule("reHLS", &createObject);
}
