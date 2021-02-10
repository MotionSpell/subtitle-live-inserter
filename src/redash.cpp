#include "redash.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_media/common/file_puller.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/time.hpp" //parseDate
#include "lib_utils/sax_xml_parser.hpp"
#include "lib_utils/xml.hpp"
#include <ctime> //gmtime
#include <thread>
#include <chrono>
#include <cassert>

using namespace Modules;
using namespace Modules::In;

extern const uint64_t g_segmentDurationInMs;
std::unique_ptr<IFilePuller> createHttpSource();
int64_t parseIso8601Period(std::string input);

namespace {

Tag parseXml(span<const char> text) {
	Tag root;
	std::vector<Tag*> tagStack = { &root };

	auto onNodeStart = [&](std::string name, SmallMap<std::string, std::string> &attr) {
		Tag tag{name};

		for (auto &a : attr)
			tag.attr.push_back({a.key, a.value});

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

class ReDash : public Module {
	public:
		ReDash(KHost* host, ReDashConfig *cfg)
			: m_host(host), url(cfg->url), httpSrc(createHttpSource()), delayInSec(cfg->delayInSec) {
			std::string urlFn = cfg->mpdFn.empty() ? url : cfg->mpdFn;
			auto i = urlFn.rfind('/');
			if(i != urlFn.npos)
				urlFn = urlFn.substr(i+1, urlFn.npos);
			auto meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = urlFn;

			auto mpdAsText = download(httpSrc.get(), url.c_str());
			if (mpdAsText.empty())
				throw std::runtime_error("can't get mpd");

			auto mpd = refreshDashSession(mpdAsText);

			cfg->utcStartTime->startTime = parseDate(mpd["availabilityStartTime"]) * IClock::Rate;
			cfg->timeshiftBufferDepthInSec = parseIso8601Period(mpd["timeShiftBufferDepth"]);

			output = addOutput();
			output->setMetadata(meta);
			m_host->activate(true);
		}

		void process() override {
			// download the mpd
			auto mpdAsText = download(httpSrc.get(), url.c_str());
			if (mpdAsText.empty()) {
				m_host->log(Error, "can't get mpd");
				return;
			}

			// compare to previous mpd
			//if (mpdAsText == lastMpdAsText)
			//	return;

			auto mpd = refreshDashSession(mpdAsText);

			// add AST offset to mitigate truncated file issues with Apache on Windows
			mpd["availabilityStartTime"] = formatDate(parseDate(mpd["availabilityStartTime"]) + delayInSec);

			// add BaseURL
			std::string baseUrl = url;
			auto i = baseUrl.rfind('/');
			if (i != baseUrl.npos)
				baseUrl = baseUrl.substr(0, i+1);
			addBaseUrl(mpd, baseUrl);

			// add our subtitles
			removeExistingSubtitleAdaptationSets(mpd);
			addSubtitleAdaptationSet(mpd);

			// publish modified mpd
			auto const modifiedMpdAsText = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + serializeXml(mpd);
			postManifest(modifiedMpdAsText);

			// save unmodified mpd
			lastMpdAsText = mpdAsText;

			std::this_thread::sleep_for(std::chrono::seconds(minUpdatePeriodInSec));
		}

	private:
		Tag refreshDashSession(const std::vector<uint8_t> &mpdAsText) {
			auto mpd = parseXml({ (const char*)mpdAsText.data(), mpdAsText.size() });
			assert(mpd.name == "MPD");

			assert(!mpd["minimumUpdatePeriod"].empty());
			minUpdatePeriodInSec = parseIso8601Period(mpd["minimumUpdatePeriod"]);

			return mpd;
		}

		void addBaseUrl(Tag &mpd, std::string baseUrl) const {
			for (auto &e : mpd.children)
				if (e.name == "AdaptationSet") {
					Tag tag{"BaseURL"};
					tag.content = baseUrl;
					e.children.insert(e.children.begin(), tag);
				} else
					addBaseUrl(e, baseUrl);
		}

		void removeExistingSubtitleAdaptationSets(Tag& mpd) const {
			for (auto& period : mpd.children)
				if (period.name == "Period")
					for (size_t iAs = 0; iAs < period.children.size(); ++iAs) {
						auto& as = period.children[iAs];
						if (as.name == "AdaptationSet")
							for (auto& rep : as.children)
								if (rep.name == "Representation")
									for (auto& codec : rep.attr)
										if (codec.name == "codecs" && codec.value == "stpp") {
											period.children.erase(period.children.begin() + iAs);
											return removeExistingSubtitleAdaptationSets(mpd);
										}
					}

		}

		void addSubtitleAdaptationSet(Tag& mpd) const {
			for (auto& e : mpd.children)
				if (e.name == "Period") {
					auto as = R"|(
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
        <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2" />
        <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main" />
        <BaseURL>.</BaseURL>
        <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s" />
        <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1" />
    </AdaptationSet>)|";
					e.add(parseXml({ as, strlen(as) }));
				} else
					addSubtitleAdaptationSet(e);
		}

		void postManifest(const std::string &contents) {
			auto out = output->allocData<DataRaw>(contents.size());
			auto metadata = make_shared<MetadataFile>(PLAYLIST);
			metadata->filename = safe_cast<const MetadataFile>(output->getMetadata())->filename;;
			metadata->filesize = contents.size();
			out->setMetadata(metadata);
			memcpy(out->buffer->data().ptr, contents.data(), contents.size());
			output->post(out);
		}

		KHost* const m_host;
		OutputDefault* output;

		std::string url;
		std::vector<uint8_t> lastMpdAsText;
		std::unique_ptr<IFilePuller> httpSrc;
		int64_t minUpdatePeriodInSec = 0;
		int delayInSec = 0;
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (ReDashConfig*)va;
	enforce(host, "reDASH: host can't be NULL");
	enforce(cfg, "reDASH: config can't be NULL");
	return createModule<ReDash>(host, cfg).release();
}

auto const registered = Factory::registerModule("reDASH", &createObject);
}
