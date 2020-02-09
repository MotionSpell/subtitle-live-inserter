#include "lib_pipeline/pipeline.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/os.hpp"
#include "lib_utils/time.hpp"
#include "lib_utils/tools.hpp" // operator|

// modules
#include "lib_media/mux/mux_mp4_config.hpp"
#include "lib_media/utils/regulator.hpp"
#include "lib_media/out/filesystem.hpp"
#include "redash.hpp"
#include "subtitle_source.hpp"

using namespace Modules;
using namespace Pipelines;

extern const char *g_appName;
extern const uint64_t g_segmentDurationInMs = 2000;

static UtcStartTime utcStartTime;

void ensureDir(std::string path) {
	if(!dirExists(path))
		mkdir(path);
}

std::unique_ptr<Pipeline> buildPipeline(const char *url, const int generalDelayInSec, const int subtitleForwardTimeInSec, const char *filename) {
	auto pipeline = std::make_unique<Pipeline>();

	ReDashConfig rdCfg;
	rdCfg.url = url;
	rdCfg.utcStartTime = &utcStartTime;
	rdCfg.delayInSec = generalDelayInSec;
	auto redasher = pipeline->add("reDASH", &rdCfg);

	auto sinkCfg = FileSystemSinkConfig { "." };
	auto sink = pipeline->add("FileSystemSink", &sinkCfg);

	auto regulate = [&](OutputPin source) -> OutputPin {
		auto regulator = pipeline->addNamedModule<Regulator>("Regulator", g_SystemClock);
		pipeline->connect(source, regulator);
		return GetOutputPin(regulator);
	};

	pipeline->connect(redasher, sink);

	//simulate a fake live subtitle source (instead of taking a live EBU-TTD input)
	if (1)
	{
		auto mux = [&](OutputPin compressed) -> OutputPin {
			Mp4MuxConfig mp4config;
			mp4config.baseName = "s_0";
			mp4config.segmentDurationInMs = g_segmentDurationInMs;
			mp4config.segmentPolicy = FragmentedSegment;
			mp4config.fragmentPolicy = OneFragmentPerSegment;
			mp4config.compatFlags = Browsers | ExactInputDur;
			mp4config.utcStartTime = &utcStartTime;

			auto muxer = pipeline->add("GPACMuxMP4", &mp4config);
			pipeline->connect(compressed, muxer);
			return muxer;
		};

		SubtitleSourceConfig subconfig;
		subconfig.filename = filename;
		subconfig.segmentDurationInMs = g_segmentDurationInMs;
		subconfig.utcStartTime = &utcStartTime;
		auto subSource = pipeline->add("SubtitleSource", &subconfig);
		auto source = GetOutputPin(subSource, 0);
		source = regulate(source); 
		mux(source);
	}

	//diff retrieved AST from MPD with the local clock
	if (1) {
		auto const granularityInMs = g_segmentDurationInMs;
		auto const t = int64_t(getUTC() * granularityInMs);
		auto const remainderInMs = granularityInMs - (t % granularityInMs);
		std::this_thread::sleep_for(std::chrono::milliseconds(remainderInMs));
		utcStartTime.startTime = rescale(t + remainderInMs, granularityInMs, IClock::Rate) - utcStartTime.startTime;
	}

	utcStartTime.startTime += subtitleForwardTimeInSec * IClock::Rate;

	return pipeline;
}
