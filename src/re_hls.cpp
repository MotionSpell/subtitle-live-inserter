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
extern const char *variantPlaylistSubFn;

namespace {

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}

// http://example.com/yo/tmp.mpd -> http://example.com/
std::string serverName(std::string path) {
	auto const prefixLen = startsWith(path, "https://") ? 8 : startsWith(path, "http://") ? 7 : 0 /*assume no prefix*/;
	auto const i = path.substr(prefixLen).find('/');
	if(i != path.npos)
		path = path.substr(0, prefixLen + i + 1);
	return path;
}

// http://example.com/yo/tmp.mpd -> http://example.com/yo/
std::string urlPath(std::string path) {
	auto i = path.rfind('/');
	if (i == std::string::npos)
		return "";

	return path.substr(0, i+1);
}

std::string filenameFromUrl(std::string url) {
	std::string fn = url;
	auto i = fn.rfind('/');
	if(i != fn.npos)
		fn = fn.substr(i+1, fn.npos);

	return fn;
}

// Only handles the playlists: master (adding subs), and variants (A/V only)
class ReHLS : public Module {
	public:
		ReHLS(KHost* host, ReDashConfig *cfg)
			: m_host(host), displayedName(cfg->displayedName), url(cfg->url),
			  baseUrlAV(cfg->baseUrlAV.empty() ? urlPath(url) : cfg->baseUrlAV), baseUrlSub(cfg->baseUrlSub),
			  hasBaseUrlAV(!cfg->baseUrlAV.empty()), delayInSec(cfg->delayInSec),
			  httpSrc(cfg->filePullerFactory->create()), nextAwakeTime(g_SystemClock->now()) {
			auto const m3u8MasterAsText = download(httpSrc.get(), url.c_str());
			if (m3u8MasterAsText.empty())
				throw std::runtime_error("can't get master m3u8");

			masterPlaylistFn = filenameFromUrl(cfg->manifestFn.empty() ? url : cfg->manifestFn);
			outputPlaylists = addOutput();
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
		//add #EXT-X-START:TIME-OFFSET
		void updateVariantPlaylist(const std::string &url) {
			auto const m3u8VariantAsText = download(httpSrc.get(), url.c_str());
			if (m3u8VariantAsText.empty())
				throw std::runtime_error(format("can't get variant m3u8 \"%s\"", m3u8VariantAsText).c_str());

			bool firstSegmentFound = false;
			std::string line, m3u8VariantNew;
			std::stringstream ss(std::string(m3u8VariantAsText.begin(), m3u8VariantAsText.end()));
			while(std::getline(ss, line)) {
				auto isFirstSegment = [&]() {
					if (firstSegmentFound) return false;
					if (startsWith(line, "#EXT-X-PROGRAM-DATE-TIME:")) return true;
					else if (startsWith(line, "#EXTINF:")) return true;
					else return false;
				};
				if (isFirstSegment()) {
					firstSegmentFound = true;
					m3u8VariantNew += "#EXT-X-START:TIME-OFFSET=";
					m3u8VariantNew += std::to_string(-delayInSec);
					m3u8VariantNew += "\n";
				}

				auto isSegment = [&]() {
					if (startsWith(line, "#")) return false;
					else return true;
				};
				if (delayInSec && isSegment()) {
					auto ensureAbsoluteOutputUrl = [&]() -> size_t {
						size_t skip = 0;
						if (startsWith(line, "http")) { // absolute
							//nothing to do
						} else if (startsWith(line, "/")) { // root
							m3u8VariantNew += urlPath(baseUrlAV);
							skip = 1; // skip trailing '/'
						} else { // relative
							m3u8VariantNew += baseUrlAV;
						}
						return skip;
					};

					auto const skip = ensureAbsoluteOutputUrl();
					m3u8VariantNew += line.substr(skip);
				} else {
					m3u8VariantNew += line;
				}

				m3u8VariantNew.push_back('\n');
			}

			m3u8VariantNew.push_back('\n');

			auto const author = std::string("## Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version + "\n";
			m3u8VariantNew += author;

			auto const fn = filenameFromUrl(url);
			postManifest(outputPlaylists, fn, m3u8VariantNew);
		}

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
					return line.size() >= 2 && line[0] == '#' && line[1] == '#';
				};
				auto addLine = [&](int offset) {
					m3u8MasterNew += line.substr(offset);
				};

				if(empty() || comment()) {
					addLine(0);
					m3u8MasterNew.push_back('\n');
					continue;
				}

				size_t i = 0;
				auto ensureAbsoluteOutputUrl = [&]() -> size_t {
					size_t skip = 0;
					if (startsWith(line, "http")) { // absolute
						if (delayInSec) {
							m3u8MasterNew += baseUrlSub;
							skip = urlPath(&line[i]).size();
						}
						//else: nothing to do
					} else if (startsWith(&line[i], "/")) { // root
						if (delayInSec)
							m3u8MasterNew += baseUrlSub;
						else if (hasBaseUrlAV)
							m3u8MasterNew += urlPath(baseUrlAV);
						else
							m3u8MasterNew += serverName(baseUrlAV);

						skip = 1; // skip trailing '/'
					} else { // relative
						skip = urlPath(&line[i]).size();
						m3u8MasterNew += delayInSec ? baseUrlSub : baseUrlAV;
					}
					return skip;
				};

				if (line[0] == '#') {
					//ditch existing subtitles
					if (line.find("TYPE=SUBTITLES") != std::string::npos)
						continue;

					//make sure URLs are absolute
					auto const pattern = "URI=\"";
					auto const pos = line.find(pattern);
					if (pos != std::string::npos) {
						for ( ; i < pos + strlen(pattern); ++i)
							m3u8MasterNew.push_back(line[i]);

						auto skip = ensureAbsoluteOutputUrl();
						while (skip-- > 0)
							i++;

						for ( ; i < line.size(); ++i)
							m3u8MasterNew.push_back(line[i]);
					} else {
						addLine(0);
					}

					//add subtitle to video
					if (line.find("RESOLUTION=") != std::string::npos)
						m3u8MasterNew += ",SUBTITLES=\"subtitles\"";
				} else {
					auto const url = line;
					auto skip = ensureAbsoluteOutputUrl();
					addLine(skip);

					//variant playlist: keep retro-compatibility by not processing when delayInSec == 0
					if (delayInSec != 0) {
						auto ensureAbsoluteInputUrl = [&](std::string inputUrl) {
							if (startsWith(line, "http")) { // absolute
								return inputUrl;
							} else if (startsWith(line, "/")) { // root
								return serverName(this->url) + inputUrl;
							} else { // relative
								return urlPath(this->url) + inputUrl;
							}
						};
						updateVariantPlaylist(ensureAbsoluteInputUrl(url));
					}
				}

				m3u8MasterNew.push_back('\n');
			}

			m3u8MasterNew.push_back('\n');

			auto const author = std::string("## Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version + "\n";
			m3u8MasterNew += author;

			//add a final entry to the master playlist
			{
				//remove trailing slash
				std::string baseUrl = baseUrlSub.empty() ? "" : baseUrlSub.back() != '/' ? baseUrlSub : baseUrlSub.substr(0, baseUrlSub.size() - 1);
				auto const subVariant = format("#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID=\"subtitles\",NAME=\"%s\",LANGUAGE=\"de\","
				        "AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI=\"%s/%s\"\n", displayedName, baseUrl, variantPlaylistSubFn);
				m3u8MasterNew += subVariant;
			}

			postManifest(outputPlaylists, masterPlaylistFn, m3u8MasterNew);
		}

		void postManifest(OutputDefault *output, const std::string &fn, const std::string &contents) {
			auto out = output->allocData<DataRaw>(contents.size());
			auto metadata = make_shared<MetadataFile>(PLAYLIST);
			metadata->filename = fn;
			metadata->filesize = contents.size();
			out->setMetadata(metadata);
			memcpy(out->buffer->data().ptr, contents.data(), contents.size());
			output->post(out);
		}

		KHost *m_host;
		OutputDefault *outputPlaylists;
		const std::string displayedName, url, baseUrlAV, baseUrlSub;
		const bool hasBaseUrlAV;
		const int delayInSec;
		std::string masterPlaylistFn;
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
