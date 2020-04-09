#include "lib_pipeline/pipeline.hpp"
#include "lib_modules/utils/loader.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/os.hpp"
#include "lib_utils/time.hpp"
#include "lib_utils/tools.hpp" // operator|
#include "options.hpp"
#include <cassert>

// modules
#include "lib_media/mux/mux_mp4_config.hpp"
#include "lib_media/utils/regulator.hpp"
#include "lib_media/out/filesystem.hpp"
#include "lib_media/out/http_sink.hpp"
#include "redash.hpp"
#include "subtitle_source.hpp"

using namespace Modules;
using namespace Pipelines;

extern const char *g_appName;
extern const uint64_t g_segmentDurationInMs = 2000;

static UtcStartTime utcStartTime;

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

	ReDashConfig rdCfg;
	rdCfg.url = cfg.url;
	rdCfg.utcStartTime = &utcStartTime;
	rdCfg.delayInSec = cfg.delayInSec;
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
		auto regulator = pipeline->addNamedModule<Regulator>("Regulator", g_SystemClock);
		pipeline->connect(source, regulator);
		return GetOutputPin(regulator);
	};

	pipeline->connect(redasher, sink);

	auto mux = [&](OutputPin compressed) -> OutputPin {
		struct Mp4MuxerFilenamed : ModuleS {
			Mp4MuxerFilenamed(KHost *host, Mp4MuxConfig *cfg) : segDurInMs(cfg->segmentDurationInMs) {
				addOutput();
				delegate = safe_cast<ModuleS>(loadModule("GPACMuxMP4", host, (void*)cfg));
				ConnectOutput(delegate->getOutput(0), [&](Data data) {
					auto out = std::make_shared<DataBaseRef>(data);
					out->copyAttributes(*data);
					auto meta = std::make_shared<MetadataFile>(*safe_cast<const MetadataFile>(data->getMetadata()));
					meta->filename = "s_0-";
					if (meta->durationIn180k == 0) {
						meta->filename += "init.mp4";
					} else {
						meta->filename += std::to_string(data->get<PresentationTime>().time / timescaleToClock(segDurInMs, 1000));
						meta->filename += ".m4s";
					}
					assert(meta->EOS); //we don't support the muxer flush mem flag
					out->setMetadata(meta);
					getOutput(0)->post(out);
				});
			}
			void processOne(Data data) override {
				delegate->getInput(0)->push(data);
			}
		private:
			std::shared_ptr<ModuleS> delegate;
			const uint64_t segDurInMs;
		};
		Mp4MuxConfig mp4config;
		mp4config.segmentDurationInMs = g_segmentDurationInMs;
		mp4config.segmentPolicy = FragmentedSegment;
		mp4config.fragmentPolicy = OneFragmentPerSegment;
		mp4config.compatFlags = Browsers | ExactInputDur;
		mp4config.utcStartTime = &utcStartTime;

		auto muxer = pipeline->addModule<Mp4MuxerFilenamed>(&mp4config);
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
