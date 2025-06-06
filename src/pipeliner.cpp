#include "lib_pipeline/pipeline.hpp"
#include <lib_media/common/utc_start_time.hpp>
#include "lib_utils/format.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/os.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/time.hpp"
#include "lib_utils/tools.hpp" // operator|
#include "options.hpp"
#include <cassert>
#include <thread>
#include <ctime>

// modules
#include "mp4_mux_file_handler_dyn.hpp"
#include "lib_media/out/filesystem.hpp"
#include "lib_media/out/http_sink.hpp"
#include "plugins/RegulatorMono/regulator_mono.hpp"
#include "hls_webvtt_rephaser.hpp"
#include "re_dash.hpp"
#include "subtitle_source.hpp"

using namespace Modules;
using namespace Pipelines;

extern const char *g_appName;
std::unique_ptr<In::IFilePuller> createHttpSource();

namespace {
void ensureDir(std::string path) {
	if(!dirExists(path))
		mkdir(path);
}

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}

static UtcStartTime utcStartTime;

struct Logger : LogSink {
		void send(Level level, const char *msg) override {
			auto const now = (double)g_SystemClock->now();
			fprintf(stderr, "[%s][%.1f][%s#%p][%s]%s\n", getTime().c_str(), now,
			    g_appName, this, getLogLevelName(level), msg);
			fflush(stderr);
		}

	private:
		const char *getLogLevelName(Level level) {
			switch (level) {
			case Level::Debug:   return "debug";
			case Level::Info:    return "info";
			case Level::Warning: return "warning";
			case Level::Error:   return "error";
			default:             return "internal error";
			}
		}

		std::string getTime() {
			char szOut[255];
			const std::time_t t = std::time(nullptr);
			const std::tm tm = *std::gmtime(&t);
			auto const size = strftime(szOut, sizeof szOut, "%Y/%m/%d %H:%M:%S", &tm);
			auto timeString = std::string(szOut, size);
			snprintf(szOut, sizeof szOut, "%s", timeString.c_str());
			return szOut;
		}
};
}

std::unique_ptr<Pipeline> buildPipeline(Config &cfg) {
	static Logger logger;
	logger.m_logLevel = Info;
	auto pipeline = std::make_unique<Pipeline>(&logger);

	//never stop on exceptions: this is needed because ReDASH/ReHLS are sources and would be stopped by default
	pipeline->registerErrorCallback([](const char *) {
		return true;
	});

	// ReDash
	IFilter *redasher = nullptr;
	struct FilePullerFactory : In::IFilePullerFactory {
		std::unique_ptr<In::IFilePuller> create() override {
			return createHttpSource();
		}
	};
	ReDashConfig rdCfg;
	rdCfg.segmentDurationInMs = cfg.segmentDurationInMs;
	rdCfg.url = cfg.url;
	UtcStartTime availabilityStartTime;
	rdCfg.utcStartTime = &availabilityStartTime; // set by this module: keep it first in module declarations
	rdCfg.delayInSec = cfg.delayInSec;
	rdCfg.manifestFn = cfg.manifestFn;
	rdCfg.displayedName = cfg.displayedName;
	rdCfg.baseUrlSub = cfg.baseUrlSub;
	rdCfg.baseUrlAV = cfg.baseUrlAV;
	FilePullerFactory filePullerFactory;
	rdCfg.filePullerFactory = &filePullerFactory;
	if (cfg.outputFormat == "dash") {
		redasher = pipeline->add("reDASH", &rdCfg);
	} else {
		redasher = pipeline->add("reHLS", &rdCfg);
	}

	// Create sink
	IFilter *sink = nullptr;
	if (startsWith(cfg.postUrl, "http")) {
		HttpSinkConfig sinkCfg{};
		sinkCfg.baseURL = cfg.postUrl;
		sinkCfg.userAgent = "Elemental";
		sink = pipeline->add("HttpSink", &sinkCfg);
	} else {
		FileSystemSinkConfig sinkCfg{};
		sinkCfg.directory = cfg.postUrl;
		ensureDir(sinkCfg.directory);
		sink = pipeline->add("FileSystemSink", &sinkCfg);
	}
	pipeline->connect(redasher, sink);

	// SubtitleSource
	SubtitleSourceConfig subconfig;
	subconfig.subtitleFn = cfg.subListFn;
	subconfig.segmentDurationInMs = cfg.segmentDurationInMs;
	subconfig.rectify = cfg.rectify;
	subconfig.format = cfg.outputFormat == "dash" ? "ttml" : "webvtt";
	subconfig.utcStartTime = &utcStartTime;
	auto subSource = pipeline->add("SubtitleSource", &subconfig);
	auto source = GetOutputPin(subSource, 0);

	// Regulate
	auto regulate = [&](OutputPin source) -> OutputPin {
		RegulatorMonoConfig rmCfg;
		rmCfg.resyncAllowed = false;
		auto regulator = pipeline->add("RegulatorMono", &rmCfg);
		pipeline->connect(source, regulator);
		return GetOutputPin(regulator);
	};
	source = regulate(source);

	if (cfg.outputFormat == "dash") {
		// Muxer
		auto mux = [&](OutputPin compressed) -> OutputPin {
			Mp4MuxConfig mp4config;
			mp4config.segmentDurationInMs = cfg.segmentDurationInMs;
			mp4config.segmentPolicy = FragmentedSegment;
			mp4config.fragmentPolicy = OneFragmentPerSegment;
			mp4config.compatFlags = Browsers | ExactInputDur;
			mp4config.utcStartTime = &utcStartTime;

			Mp4MuxFileHandlerDynConfig mp4Cfg;
			mp4Cfg.mp4MuxCfg = &mp4config;
			mp4Cfg.timeshiftBufferDepthInSec = cfg.timeshiftBufferDepthInSec == -1/*auto*/ ?
			rdCfg.timeshiftBufferDepthInSec : cfg.timeshiftBufferDepthInSec;

			auto muxer = pipeline->add("Mp4MuxFileHandlerDyn", &mp4Cfg);
			pipeline->connect(compressed, muxer);
			return muxer;
		};
		source = mux(source);
	} else {
		assert(cfg.outputFormat == "hls");
		HlsWebvttRephaserConfig hwrCfg;
		hwrCfg.url = cfg.url;
		hwrCfg.segmentDurationInMs = cfg.segmentDurationInMs;
		hwrCfg.timeshiftBufferDepthInSec = cfg.timeshiftBufferDepthInSec;
		hwrCfg.delayInSec = cfg.delayInSec;
		hwrCfg.subtitleForwardTimeInSec = cfg.subtitleForwardTimeInSec;
		auto rephaser = pipeline->add("HlsWebvttRephaser", &hwrCfg);
		pipeline->connect(source, rephaser);
		source = rephaser;
		pipeline->connect(GetOutputPin(rephaser, 1), sink, true);
	}
	pipeline->connect(source, sink, true);

	if (cfg.outputFormat == "dash") {
		// Diff retrieved AST from MPD with the local clock
		auto const granularityInMs = cfg.segmentDurationInMs;
		auto const t = int64_t(getUTC() * granularityInMs);
		auto const remainderInMs = granularityInMs - (t % granularityInMs);
		std::this_thread::sleep_for(std::chrono::milliseconds(remainderInMs));
		utcStartTime.startTime = rescale(t + remainderInMs, granularityInMs, IClock::Rate);
	} else {
		utcStartTime.startTime = fractionToClock(getUTC());
	}
	utcStartTime.startTime += cfg.subtitleForwardTimeInSec * IClock::Rate;
	availabilityStartTime.startTime += cfg.subtitleForwardTimeInSec * IClock::Rate;
	cfg.updateDelayInSec = rdCfg.updateDelayInSec;

	logger.send(Info, format("AST=%s ; UTC=%s", availabilityStartTime.startTime/IClock::Rate,
	        utcStartTime.startTime/IClock::Rate).c_str());

	return pipeline;
}
