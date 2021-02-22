#include "lib_pipeline/pipeline.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/os.hpp"
#include "lib_utils/time.hpp"
#include "lib_utils/tools.hpp" // operator|
#include "options.hpp"
#include <thread>

// modules
#include "mp4_mux_file_handler_dyn.hpp"
#include "lib_media/out/filesystem.hpp"
#include "lib_media/out/http_sink.hpp"
#include "plugins/RegulatorMono/regulator_mono.hpp"
#include "redash.hpp"
#include "subtitle_source.hpp"

using namespace Modules;
using namespace Pipelines;

extern const char *g_appName;
extern const uint64_t g_segmentDurationInMs = 2000;

static UtcStartTime utcStartTime;
std::unique_ptr<In::IFilePuller> createHttpSource();

namespace {
void ensureDir(std::string path) {
	if(!dirExists(path))
		mkdir(path);
}

bool startsWith(std::string s, std::string prefix) {
	return s.substr(0, prefix.size()) == prefix;
}
}

std::unique_ptr<Pipeline> buildPipeline(const Config &cfg) {
	auto pipeline = std::make_unique<Pipeline>();

	struct FilePullerFactory : In::IFilePullerFactory {
		std::unique_ptr<In::IFilePuller> create() override {
			return createHttpSource();
		}
	};

	ReDashConfig rdCfg;
	rdCfg.url = cfg.url;
	rdCfg.utcStartTime = &utcStartTime;
	rdCfg.delayInSec = cfg.delayInSec;
	rdCfg.mpdFn = cfg.mpdFn;
	rdCfg.postUrl = cfg.postUrl;
	FilePullerFactory filePullerFactory;
	rdCfg.filePullerFactory = &filePullerFactory;
	auto redasher = pipeline->add("reDASH", &rdCfg);

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

	auto regulate = [&](OutputPin source) -> OutputPin {
		RegulatorMonoConfig rmCfg;
		auto regulator = pipeline->add("RegulatorMono", &rmCfg);
		pipeline->connect(source, regulator);
		return GetOutputPin(regulator);
	};

	pipeline->connect(redasher, sink);

	auto mux = [&](OutputPin compressed) -> OutputPin {
		Mp4MuxConfig mp4config;
		mp4config.segmentDurationInMs = g_segmentDurationInMs;
		mp4config.segmentPolicy = FragmentedSegment;
		mp4config.fragmentPolicy = OneFragmentPerSegment;
		mp4config.compatFlags = Browsers | ExactInputDur;
		mp4config.utcStartTime = &utcStartTime;

		Mp4MuxFileHandlerDynConfig cfg;
		cfg.mp4MuxCfg = &mp4config;
		cfg.timeshiftBufferDepthInSec = rdCfg.timeshiftBufferDepthInSec;

		auto muxer = pipeline->add("Mp4MuxFileHandlerDyn", &cfg);
		pipeline->connect(compressed, muxer);
		return muxer;
	};

	SubtitleSourceConfig subconfig;
	subconfig.filename = cfg.subListFn;
	subconfig.segmentDurationInMs = g_segmentDurationInMs;
	subconfig.utcStartTime = &utcStartTime;
	auto subSource = pipeline->add("SubtitleSource", &subconfig);
	auto source = GetOutputPin(subSource, 0);
	source = regulate(source);
	auto muxer = mux(source);
	pipeline->connect(muxer, sink, true);

	//diff retrieved AST from MPD with the local clock
	auto const granularityInMs = g_segmentDurationInMs;
	auto const t = int64_t(getUTC() * granularityInMs);
	auto const remainderInMs = granularityInMs - (t % granularityInMs);
	std::this_thread::sleep_for(std::chrono::milliseconds(remainderInMs));
	utcStartTime.startTime = rescale(t + remainderInMs, granularityInMs, IClock::Rate) - utcStartTime.startTime;
	utcStartTime.startTime += cfg.subtitleForwardTimeInSec * IClock::Rate;

	return pipeline;
}
