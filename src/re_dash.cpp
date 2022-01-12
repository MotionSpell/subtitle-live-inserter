#include "re_dash.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/time.hpp" //parseDate
#include "lib_utils/sax_xml_parser.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/xml.hpp"
#include <ctime> //gmtime
#include <chrono>
#include <thread>
#include <cassert>

using namespace Modules;
using namespace Modules::In;

extern const char *g_appName;
extern const char *g_version;
std::unique_ptr<IFilePuller> createHttpSource();
int64_t parseIso8601Period(std::string input);

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

namespace {

bool isUrlAbsolute(const std::string &url) {
	const std::string prefix = "http";
	return url.substr(0, prefix.size()) == prefix;
};

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
			: m_host(host), url(cfg->url), baseUrlAV(cfg->baseUrlAV), baseUrlSub(cfg->baseUrlSub), segmentDurationInMs(cfg->segmentDurationInMs),
			  httpSrc(cfg->filePullerFactory->create()), nextAwakeTime(g_SystemClock->now()), delayInSec(cfg->delayInSec) {
			std::string urlFn = cfg->manifestFn.empty() ? url : cfg->manifestFn;
			auto i = urlFn.rfind('/');
			if(i != urlFn.npos)
				urlFn = urlFn.substr(i+1, urlFn.npos);
			auto meta = std::make_shared<MetadataFile>(PLAYLIST);
			meta->filename = urlFn;

			auto mpdAsText = download(httpSrc.get(), url.c_str());
			if (mpdAsText.empty())
				throw std::runtime_error("can't get mpd");

			auto mpd = refreshDashSession(mpdAsText);

			auto sanity = [&](const std::string &attribute) {
				if (mpd["minimumUpdatePeriod"].empty())
					throw error(format("Sanity check failed: MPD %s attributes is absent.", attribute).c_str());
			};

			sanity("availabilityStartTime");
			sanity("minimumUpdatePeriod");
			sanity("timeShiftBufferDepth");

			cfg->timeshiftBufferDepthInSec = parseIso8601Period(mpd["timeShiftBufferDepth"]);

			output = addOutput();
			output->setMetadata(meta);
			m_host->activate(true);

			cfg->updateDelayInSec = std::bind(&ReDash::updateDelayInSec, this, std::placeholders::_1);
		}

		void process() override {
			// is it time?
			if (g_SystemClock->now() < nextAwakeTime) {
				auto const sleepInMs = 200;
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepInMs));
				return;
			}

			// download the mpd
			auto mpdAsText = download(httpSrc.get(), url.c_str());
			if (mpdAsText.empty()) {
				m_host->log(Error, "can't get mpd");
				return;
			}

			// compare to previous mpd
			if (mpdAsText == lastMpdAsText)
				return;

			auto mpd = refreshDashSession(mpdAsText);

			// add AST offset to mitigate truncated file issues with Apache on Windows
			{
				std::unique_lock<std::mutex> lock(updateMutex);
				mpd["availabilityStartTime"] = formatDate(parseDate(mpd["availabilityStartTime"]) + delayInSec);
			}

			// add version of this tool
			addVersion(mpd);

			// add BaseURL whenever necessary for all but the subtitle adaptation sets
			addBaseUrl(mpd, url);

			// add our subtitles
			removeExistingSubtitleAdaptationSets(mpd);
			addSubtitleAdaptationSet(mpd);

			// publish modified mpd
			auto const modifiedMpdAsText = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + serializeXml(mpd);
			postManifest(modifiedMpdAsText);

			// save unmodified mpd
			lastMpdAsText = mpdAsText;

			auto sleepTimeInSec = std::abs(minUpdatePeriodInSec);
			auto const maxSleepTimeInSec = 2; // some contents refresh more often than advertised in the manifest
			if (sleepTimeInSec > maxSleepTimeInSec) sleepTimeInSec = maxSleepTimeInSec;
			nextAwakeTime = g_SystemClock->now() + sleepTimeInSec;
		}

	private:
		void updateDelayInSec(int delayInSec) {
			std::unique_lock<std::mutex> lock(updateMutex);
			this->delayInSec = delayInSec;
		}

		Tag refreshDashSession(const std::vector<uint8_t> &mpdAsText) {
			auto mpd = parseXml({ (const char*)mpdAsText.data(), mpdAsText.size() });
			assert(mpd.name == "MPD");

			assert(!mpd["minimumUpdatePeriod"].empty());
			minUpdatePeriodInSec = parseIso8601Period(mpd["minimumUpdatePeriod"]);

			return mpd;
		}

		void addVersion(Tag &mpd) const {
			auto const version = std::string("Updated with Motion Spell / GPAC Licensing ") + g_appName + " version " + g_version;

			for (auto& e : mpd.children)
				if (e.name == "ProgramInformation")
					for (auto& p : e.children)
						if (p.name == "Title") {
							p.content += std::string(" - ") + version;
							return;
						}

			Tag title { "Title" };
			title.content = version;
			Tag progInfo { "ProgramInformation" };
			progInfo.children.push_back(title);
			mpd.children.insert(mpd.children.begin(), progInfo);
		}

		void addBaseUrl(Tag &mpd, const std::string &url) const {
			// compute base URL
			std::string baseUrl;

			if (baseUrlAV.empty()) {
				baseUrl = url;
				auto i = baseUrl.rfind('/');
				if (i != baseUrl.npos)
					baseUrl = baseUrl.substr(0, i+1);
			} else {
				baseUrl = baseUrlAV;
			}

			// ensure all the elements from @tag are covered by an absolute BaseURL
			auto hasAbsoluteBaseUrl = [](Tag &tag) {
				for (auto &e : tag.children)
					if (e.name == "BaseURL" && isUrlAbsolute(e.content))
						return true;

				return false;
			};

			if (hasAbsoluteBaseUrl(mpd))
				return;

			for (auto& period : mpd.children) {
				if (hasAbsoluteBaseUrl(period))
					continue;

				if (period.name == "Period")
					for (auto& as : period.children)
						if (as.name == "AdaptationSet") {
							if (hasAbsoluteBaseUrl(as))
								continue;

							for (auto& rep : as.children)
								if (rep.name == "Representation") {
									if (hasAbsoluteBaseUrl(rep))
										continue;

									Tag tag{"BaseURL"};
									tag.content = baseUrl;
									rep.children.insert(rep.children.begin(), tag);
								}
						}
			}
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
					auto b = baseUrlSub.empty() ? std::string() : format("<BaseURL>%s</BaseURL>", baseUrlSub);
					auto const timescale = 10000000;
					auto const startNumber = (parseDate(mpd["availabilityStartTime"]) * 1000) / segmentDurationInMs;
					auto as = format(R"|(
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
        <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2" />
        <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main" />
        %s<SegmentTemplate timescale="%s" duration="%s" startNumber="%s" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s" />
        <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1" />
    </AdaptationSet>)|", b, timescale, rescale(segmentDurationInMs, 1000, timescale), startNumber);
					e.add(parseXml({ as.c_str(), as.size() }));
				} else
					addSubtitleAdaptationSet(e);
		}

		void postManifest(const std::string &contents) {
			auto out = output->allocData<DataRaw>(contents.size());
			auto metadata = make_shared<MetadataFile>(PLAYLIST);
			metadata->filename = safe_cast<const MetadataFile>(output->getMetadata())->filename;
			metadata->filesize = contents.size();
			out->setMetadata(metadata);
			memcpy(out->buffer->data().ptr, contents.data(), contents.size());
			output->post(out);
		}

		KHost* const m_host;
		OutputDefault* output;

		const std::string url, baseUrlAV, baseUrlSub;
		const int segmentDurationInMs;
		std::vector<uint8_t> lastMpdAsText;
		std::unique_ptr<IFilePuller> httpSrc;
		int64_t minUpdatePeriodInSec = 0;
		Fraction nextAwakeTime;

		std::mutex updateMutex;
		int delayInSec = 0; // protected by updateMutex
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (ReDashConfig*)va;
	enforce(host, "reDASH: host can't be NULL");
	enforce(cfg, "reDASH: config can't be NULL");
	return createModule<ReDash>(host, cfg).release();
}

auto const registered = Factory::registerModule("reDASH", &createObject);
}
