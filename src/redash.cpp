#include "redash.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_media/common/file_puller.hpp"
#include "lib_media/common/sax_xml_parser.hpp"
#include "lib_media/common/xml.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/time.hpp" //parseDate
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

	//FIXME: our parser uses a map that reorder the attributes
	auto onNodeStart = [&](std::string name, std::map<std::string, std::string> &attr) {
		Tag tag{name};

		for (auto &a : attr)
			tag.attr.push_back({a.first, a.second});

		tagStack.back()->add(tag);
		tagStack.push_back(&tagStack.back()->children.back());
	};

	auto onNodeEnd = [&](std::string) {
		tagStack.pop_back();
	};

	saxParse(text, onNodeStart, onNodeEnd);

	assert(tagStack.front()->children.size() == 1);
	return tagStack.front()->children[0];
}

class ReDash : public Module {
	public:
		ReDash(KHost* host, ReDashConfig *cfg)
		: m_host(host), url(cfg->url), httpSrc(createHttpSource()) {
			std::string urlFn = url;
			auto i = urlFn.rfind('/');
			if(i != urlFn.npos)
				urlFn = urlFn.substr(i+1, urlFn.npos);
			auto meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = urlFn;
			output = addOutput();
			output->setMetadata(meta);
			m_host->activate(true);

			auto mpd = refreshDashSession(url);

			cfg->utcStartTime->startTime = parseDate(mpd["availabilityStartTime"]) * IClock::Rate;
		}

		void process() override {
			auto mpd = refreshDashSession(url);

			//TODO: there is no default comparison operator in C++<20
			//if (mpd == lastMpd)
			//	return;

			//add BaseURL
			std::string baseUrl = url;
			auto i = baseUrl.rfind('/');
			if (i != baseUrl.npos)
				baseUrl = baseUrl.substr(0, i+1);
			addBaseUrl(mpd, baseUrl);

			//add subtitles
			addSubtitleAdaptationSet(mpd);

			auto newMpd = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + serializeXml(mpd);

			postManifest(newMpd);

			if (0) {
				m_host->log(Warning, newMpd.c_str());
				exit(0);
			}

			std::this_thread::sleep_for(std::chrono::seconds(minUpdatePeriodInSec));
		}

	private:
		Tag refreshDashSession(std::string url) {
			auto mpdAsText = download(httpSrc.get(), url.c_str());
			if (mpdAsText.empty())
				throw std::runtime_error("can't get mpd");

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
					//tag.attr.push_back({"serviceLocation", baseUrl});
					e.add(tag);
				} else
					addBaseUrl(e, baseUrl);
		}

		void addSubtitleAdaptationSet(Tag& mpd) const {
			for (auto& e : mpd.children)
				if (e.name == "Period") {
					auto as = R"|(
    <AdaptationSet id="gpac_licensing_live_sub_inserter" lang="de" segmentAlignment="true">
        <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2" />
        <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main" />
        <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s" />
        <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1" />
    </AdaptationSet>)|";
					e.add(parseXml({ as, strlen(as) }));
				}
				else
					addSubtitleAdaptationSet(e);
		}

		void postManifest(std::string &contents) {
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
		std::unique_ptr<IFilePuller> httpSrc;
		int minUpdatePeriodInSec = 0;
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (ReDashConfig*)va;
	enforce(host, "reDASH: host can't be NULL");
	enforce(cfg, "reDASH: config can't be NULL");
	return createModule<ReDash>(host, cfg).release();
}

auto const registered = Factory::registerModule("reDASH", &createObject);
}
