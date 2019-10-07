//Romain: review includes
#include "lib_pipeline/pipeline.hpp"
#include "lib_utils/system_clock.hpp"
#include "lib_utils/os.hpp"
#include "lib_utils/time.hpp"
#include <algorithm> //std::max
#include <cassert>

// modules
#include "lib_media/common/metadata.hpp"
#include "lib_media/common/pcm.hpp"
#include "lib_media/common/picture.hpp"
#include "lib_media/encode/libav_encode.hpp"
#include "lib_media/in/sound_generator.hpp"
#include "lib_media/in/video_generator.hpp"
#include "lib_media/mux/mux_mp4_config.hpp"
#include "lib_media/utils/regulator.hpp"
#include "lib_media/stream/adaptive_streaming_common.hpp" // AdaptiveStreamingCommon::getCommonPrefixAudio
#include "lib_media/out/filesystem.hpp"
#include "lib_media/out/http_sink.hpp"
#include "lib_media/transform/audio_convert.hpp"
#include "plugins/Dasher/mpeg_dash.hpp"

using namespace Modules;
using namespace Pipelines;

extern const char *g_appName;

auto const DASH_SUBDIR = "dash/";
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

std::unique_ptr<Pipeline> buildPipeline(const char *filename) {
	auto pipeline = make_unique<Pipeline>();

	const bool live = true;
	auto dasherCfg = Modules::DasherConfig { DASH_SUBDIR, format("%s.mpd", g_appName), live, g_segmentDurationInMs};
	auto dasher = pipeline->add("MPEG_DASH", &dasherCfg);

	auto sinkCfg = FileSystemSinkConfig { "." };
	auto sink = pipeline->add("FileSystemSink", &sinkCfg);
	ensureDir(DASH_SUBDIR);

	pipeline->connect(GetOutputPin(dasher, 0), sink);
	pipeline->connect(GetOutputPin(dasher, 1), sink, true);

	auto regulate = [&](OutputPin source) -> OutputPin {
		auto regulator = pipeline->addNamedModule<Regulator>("Regulator", g_SystemClock);
		pipeline->connect(source, regulator);
		return GetOutputPin(regulator);
	};

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

    int numDashInputs = 0;

    {
        auto videoGen = pipeline->addModule<In::VideoGenerator>("videogen://framecount=0&framerate=25");
        auto source = GetOutputPin(videoGen, 0);
        source = regulate(source);

        auto const resolution = Resolution(320, 180); //Romain: videogen resolution is hardcoded and not transmitted in initial metadata
        PictureFormat encoderInputPicFmt(resolution, PixelFormat::I420);
		g_Log->log(Info, "[Encoder] Found video stream");
		EncoderConfig p { EncoderConfig::Video };
		p.bitrate = (resolution.width * resolution.height * 25 * 7) / 100; //Romain: arbitrary bitrate
		p.frameRate = Fraction(25); //Romain: hardcoded from videogen ctor params
		p.GOPSize = Fraction(g_segmentDurationInMs, 1000) * p.frameRate;
		auto encoder = pipeline->add("Encoder", &p);

	    PictureFormat dstFmt(resolution, p.pixelFormat);        
		auto converter = pipeline->add("VideoConvert", &dstFmt);
        pipeline->connect(source, converter);
        pipeline->connect(converter, encoder);

        auto prefix = Stream::AdaptiveStreamingCommon::getCommonPrefixVideo(numDashInputs, resolution);
        auto muxer = mux(GetOutputPin(encoder));
        pipeline->connect(muxer, GetInputPin(dasher, numDashInputs));
        ++numDashInputs;
    }

    {
        auto audioGen = pipeline->addModule<In::SoundGenerator>();
        auto source = GetOutputPin(audioGen, 0);
        auto metadata = audioGen->getOutputMetadata(0);
        source = regulate(source);

		auto format = PcmFormat(44100, 2, Stereo, F32, Planar);
		auto cfg = AudioConvertConfig{ {0}, format, 1024};
		auto converter = pipeline->add("AudioConvert", &cfg);
        pipeline->connect(source, GetInputPin(converter));

		EncoderConfig p { EncoderConfig::Audio };
		auto encoder = pipeline->add("Encoder", &p);
        pipeline->connect(GetOutputPin(converter), GetInputPin(encoder));

        auto muxer = mux(GetOutputPin(encoder));
        pipeline->connect(muxer, GetInputPin(dasher, numDashInputs));
        ++numDashInputs;
    }

    {
        auto subSource = pipeline->add("SubtitleSource", filename);    
        auto source = GetOutputPin(subSource, 0);
        source = regulate(source);
        auto prefix = Stream::AdaptiveStreamingCommon::getCommonPrefixSubtitle(numDashInputs);

        auto muxer = mux(source);

        pipeline->connect(muxer, GetInputPin(dasher, numDashInputs));
        ++numDashInputs;
    }

	utcStartTime.startTime = fractionToClock(getUTC());

	return pipeline;
}
