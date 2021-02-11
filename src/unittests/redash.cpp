#include "tests/tests.hpp"
#include "lib_modules/utils/loader.hpp"
#include "lib_modules/utils/helper.hpp" // NullHost
#include "lib_media/common/file_puller.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_media/utils/recorder.hpp"
#include "lib_modules/core/connection.hpp"
#include "../redash.hpp"

using namespace Modules;

namespace {
struct MemoryFileSystem : In::IFilePuller {
    MemoryFileSystem(const char *src) : src(src) {}
	void wget(const char* /*szUrl*/, std::function<void(SpanC)> callback) override {
        ASSERT(src != nullptr);
		callback({(const uint8_t*)src, strlen(src)});
	}
	const char *src = nullptr;
};

struct FilePullerFactory : In::IFilePullerFactory {
    FilePullerFactory(const char *src) : src(src) {}
    std::unique_ptr<In::IFilePuller> create() override {
        return std::make_unique<MemoryFileSystem>(src);
    }
	const char *src = nullptr;
};

unittest("Redash: manifest from Keepixo/Anevia/Ateme") {
    auto mpd = R"|(
<?xml version="1.0" encoding="UTF-8" ?>
<MPD xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:mspr="urn:microsoft:playready" xmlns:cenc="urn:mpeg:cenc:2013" profiles="urn:dvb:dash:profile:dvb-dash:2014,urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011" type="dynamic" availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" publishTime="2020-10-02T17:27:38Z" timeShiftBufferDepth="PT24H0.00S" minBufferTime="PT10.00S">
  <Period id="0" start="PT0S">
    <!--Video-->
    <AdaptationSet id="0" maxWidth="1280" maxHeight="720" maxFrameRate="50/1" par="16:9" segmentAlignment="true">
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_0_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_0_$RepresentationID$-$Number$.mp4" />
      <Representation id="video_00" mimeType="video/mp4" codecs="avc3.640020" bandwidth="3500000" width="1280" height="720" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1" />
      <Representation id="video_01" mimeType="video/mp4" codecs="avc3.64001f" bandwidth="1800000" width="960" height="540" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1" />
      <Representation id="video_02" mimeType="video/mp4" codecs="avc3.64001f" bandwidth="1000000" width="640" height="360" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1" />
    </AdaptationSet>
    <!--Audio-->
    <AdaptationSet id="1" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main" />
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_1_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_1_$RepresentationID$-$Number$.mp4" />
      <Representation id="audio_03" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="128000" audioSamplingRate="48000" startWithSAP="1">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2" />
      </Representation>
    </AdaptationSet>
    <!--Audio-->
    <AdaptationSet id="2" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="alternate" />
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_2_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_2_$RepresentationID$-$Number$.mp4" />
      <Representation id="audio_04" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="96000" audioSamplingRate="24000" startWithSAP="1">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2" />
      </Representation>
    </AdaptationSet>
  </Period>
</MPD>)|";

    std::string expected = R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:mspr="urn:microsoft:playready" xmlns:cenc="urn:mpeg:cenc:2013" profiles="urn:dvb:dash:profile:dvb-dash:2014,urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011" type="dynamic" availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" publishTime="2020-10-02T17:27:38Z" timeShiftBufferDepth="PT24H0.00S" minBufferTime="PT10.00S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version 2-master-rev34-g8f15aa5</Title>
  </ProgramInformation>
  <Period id="0" start="PT0S">
    <AdaptationSet id="0" maxWidth="1280" maxHeight="720" maxFrameRate="50/1" par="16:9" segmentAlignment="true">
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_0_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_0_$RepresentationID$-$Number$.mp4"/>
      <Representation id="video_00" mimeType="video/mp4" codecs="avc3.640020" bandwidth="3500000" width="1280" height="720" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>/home/root/</BaseURL>
      </Representation>
      <Representation id="video_01" mimeType="video/mp4" codecs="avc3.64001f" bandwidth="1800000" width="960" height="540" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>/home/root/</BaseURL>
      </Representation>
      <Representation id="video_02" mimeType="video/mp4" codecs="avc3.64001f" bandwidth="1000000" width="640" height="360" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>/home/root/</BaseURL>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_1_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_1_$RepresentationID$-$Number$.mp4"/>
      <Representation id="audio_03" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="128000" audioSamplingRate="48000" startWithSAP="1">
        <BaseURL>/home/root/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="2" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="alternate"/>
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_2_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_2_$RepresentationID$-$Number$.mp4"/>
      <Representation id="audio_04" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="96000" audioSamplingRate="24000" startWithSAP="1">
        <BaseURL>/home/root/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>http://127.0.0.1/test/</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|";

    ReDashConfig cfg;
    cfg.url = "/home/root/";
    UtcStartTime utcStartTime;
    utcStartTime.startTime = 1789;
    cfg.utcStartTime = &utcStartTime;
    cfg.delayInSec = 0;
    cfg.timeshiftBufferDepthInSec = 17;
    cfg.mpdFn = "redash.mpd";
    cfg.postUrl = "http://127.0.0.1/test/";
    FilePullerFactory filePullerFactory(mpd);
    cfg.filePullerFactory = &filePullerFactory;
    auto redash = loadModule("reDASH", &NullHost, &cfg);
    auto recorder = createModule<Utils::Recorder>(&NullHost);
	ConnectOutputToInput(redash->getOutput(0), recorder->getInput(0));

    redash->process();
    
    Data data;
    auto ret = recorder->tryPop(data);
    ASSERT_EQUALS(true, ret);

    auto dataRaw = std::dynamic_pointer_cast<const DataRaw>(data);
    ASSERT(dataRaw);

    ret = recorder->tryPop(data);
    ASSERT_EQUALS(false, ret);

    auto meta = std::dynamic_pointer_cast<const MetadataFile>(data->getMetadata());
    ASSERT(meta);

    ASSERT_EQUALS(cfg.mpdFn, meta->filename.substr(meta->filename.size() - cfg.mpdFn.size(), meta->filename.size()));

    ASSERT_EQUALS(expected, std::string((const char*)data->data().ptr, data->data().len).c_str());
}

}
