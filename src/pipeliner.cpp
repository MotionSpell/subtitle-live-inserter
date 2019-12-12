#include "lib_pipeline/pipeline.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/os.hpp"
#include "lib_utils/time.hpp"
#include "lib_utils/tools.hpp" // operator|

// modules
#include "lib_media/mux/mux_mp4_config.hpp"
#include "lib_media/utils/regulator.hpp"
#include "lib_media/out/filesystem.hpp"

using namespace Modules;
using namespace Pipelines;

extern const char *g_appName;

extern auto const g_DashSubDir = "dash/";
extern const uint64_t g_segmentDurationInMs = 2000;

struct UtcStartTime : IUtcStartTimeQuery {
	uint64_t query() override {
		return startTime;
	}
	uint64_t startTime;
};

static UtcStartTime utcStartTime;

void ensureDir(std::string path) {
	if(!dirExists(path))
		mkdir(path);
}

std::unique_ptr<Pipeline> buildPipeline(const char *url, const char *filename) {
	auto pipeline = std::make_unique<Pipeline>();

	auto redasher = pipeline->add("reDASH", url);

	auto sinkCfg = FileSystemSinkConfig { "." };
	auto sink = pipeline->add("FileSystemSink", &sinkCfg);
	ensureDir(g_DashSubDir);

	auto regulate = [&](OutputPin source) -> OutputPin {
		auto regulator = pipeline->addNamedModule<Regulator>("Regulator", g_SystemClock);
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

		auto muxer = pipeline->add("GPACMuxMP4", &mp4config);
		pipeline->connect(compressed, muxer);
		return muxer;
	};

	//FIXME: we simulate a fake live subtitle source (instead of taking a live EBU-TTD input)
    {
        auto subSource = pipeline->add("SubtitleSource", filename);    
        auto source = GetOutputPin(subSource, 0);
		source = regulate(source);
		auto muxer = mux(source);
		pipeline->connect(muxer, sink);
	}

	utcStartTime.startTime = fractionToClock(getUTC());

	return pipeline;
}
