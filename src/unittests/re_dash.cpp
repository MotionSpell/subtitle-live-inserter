#include "common.hpp"
#include "lib_utils/format.hpp"

const char *g_appName = "subtitle-live-inserter";

namespace {

unittest("reDash: manifest from Keepixo/Anevia/Ateme") {
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

	auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:mspr="urn:microsoft:playready" xmlns:cenc="urn:mpeg:cenc:2013" profiles="urn:dvb:dash:profile:dvb-dash:2014,urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011" type="dynamic" availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" publishTime="2020-10-02T17:27:38Z" timeShiftBufferDepth="PT24H0.00S" minBufferTime="PT10.00S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period id="0" start="PT0S">
    <AdaptationSet id="0" maxWidth="1280" maxHeight="720" maxFrameRate="50/1" par="16:9" segmentAlignment="true">
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_0_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_0_$RepresentationID$-$Number$.mp4"/>
      <Representation id="video_00" mimeType="video/mp4" codecs="avc3.640020" bandwidth="3500000" width="1280" height="720" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
      </Representation>
      <Representation id="video_01" mimeType="video/mp4" codecs="avc3.64001f" bandwidth="1800000" width="960" height="540" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
      </Representation>
      <Representation id="video_02" mimeType="video/mp4" codecs="avc3.64001f" bandwidth="1000000" width="640" height="360" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_1_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_1_$RepresentationID$-$Number$.mp4"/>
      <Representation id="audio_03" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="128000" audioSamplingRate="48000" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="2" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="alternate"/>
      <SegmentTemplate timescale="10000000" duration="40000000" startNumber="1" initialization="f25cc24bca2d606b3bc0b585b3c53985_2_$RepresentationID$-init.mp4" media="f25cc24bca2d606b3bc0b585b3c53985_2_$RepresentationID$-$Number$.mp4"/>
      <Representation id="audio_04" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="96000" audioSamplingRate="24000" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>.</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    check("reDASH", mpd, expected);
}

unittest("reDash: manifest from Elemental for ARD") {
    auto mpd = R"|(
<?xml version="1.0" encoding="UTF-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-11T15:39:24Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-02-05T08:27:46Z" minBufferTime="PT22S" suggestedPresentationDelay="PT2S" timeShiftBufferDepth="PT1M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
   <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="540000" startNumber="1612513660"/>
      <Representation id="1" width="1280" height="720" bandwidth="3584000" codecs="avc1.640020" scanType="progressive">
        <SegmentTemplate duration="540000" startNumber="1612513660" media="dash_1280x720p50_3584k-$Number$.mp4" initialization="dash_1280x720p50_3584k-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="1800000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="540000" startNumber="1612513660" media="dash_960x540p50_1800k-$Number$.mp4" initialization="dash_960x540p50_1800k-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="540000" startNumber="1612513660" media="dash_640x360p50_1024k-$Number$.mp4" initialization="dash_640x360p50_1024k-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="ger" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dash_128k_aac-$Number$.mp4" initialization="dash_128k_aac-init.mp4" duration="288000" startNumber="1612513660"/>
      <Representation id="4" bandwidth="128000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
  </Period>
    )|";

	        auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-11T15:39:24Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-02-05T08:27:46Z" minBufferTime="PT22S" suggestedPresentationDelay="PT2S" timeShiftBufferDepth="PT1M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="540000" startNumber="1612513660"/>
      <Representation id="1" width="1280" height="720" bandwidth="3584000" codecs="avc1.640020" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="540000" startNumber="1612513660" media="dash_1280x720p50_3584k-$Number$.mp4" initialization="dash_1280x720p50_3584k-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="1800000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="540000" startNumber="1612513660" media="dash_960x540p50_1800k-$Number$.mp4" initialization="dash_960x540p50_1800k-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="540000" startNumber="1612513660" media="dash_640x360p50_1024k-$Number$.mp4" initialization="dash_640x360p50_1024k-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="ger" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dash_128k_aac-$Number$.mp4" initialization="dash_128k_aac-init.mp4" duration="288000" startNumber="1612513660"/>
      <Representation id="4" bandwidth="128000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <BaseURL>http://url/for/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>.</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    check("reDASH", mpd, expected);
}

unittest("reDash: manifest from Elemental for RBB (MDR)") {
    auto mpd = R"|(
<?xml version="1.0" encoding="UTF-8" ?>
<MPD xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:mspr="urn:microsoft:playready" xmlns:cenc="urn:mpeg:cenc:2013" profiles="urn:dvb:dash:profile:dvb-dash:2014,urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011" type="dynamic" availabilityStartTime="1970-01-01T00:00:00Z" minimumUpdatePeriod="PT6.000S" publishTime="2021-02-03T10:31:43Z" timeShiftBufferDepth="PT8H0.000S" minBufferTime="PT6.000S">
  <Period id="0" start="PT0S">
    <!--Video-->
    <AdaptationSet id="0" maxWidth="1280" maxHeight="720" maxFrameRate="50/1" par="16:9" segmentAlignment="true">
      <SegmentTemplate timescale="48000" duration="288000" startNumber="0" initialization="499cc3ced932a4d6108535e62f880109_0_$RepresentationID$_init.mp4" media="499cc3ced932a4d6108535e62f880109_0_$RepresentationID$-$Number$.mp4" />
      <Representation id="video_00" mimeType="video/mp4" codecs="avc3.4d401f" bandwidth="900000" width="640" height="360" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1" />
      <Representation id="video_01" mimeType="video/mp4" codecs="avc3.4d401f" bandwidth="1800000" width="960" height="540" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1" />
      <Representation id="video_02" mimeType="video/mp4" codecs="avc3.4d4020" bandwidth="3500000" width="1280" height="720" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1" />
    </AdaptationSet>
    <!--Audio-->
    <AdaptationSet id="1" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main" />
      <SegmentTemplate timescale="48000" duration="288000" startNumber="0" initialization="499cc3ced932a4d6108535e62f880109_1_$RepresentationID$_init.mp4" media="499cc3ced932a4d6108535e62f880109_1_$RepresentationID$-$Number$.mp4" />
      <Representation id="audio_03" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="128000" audioSamplingRate="48000" startWithSAP="1">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2" />
      </Representation>
    </AdaptationSet>
  </Period>
</MPD>
    )|";

	                auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:mspr="urn:microsoft:playready" xmlns:cenc="urn:mpeg:cenc:2013" profiles="urn:dvb:dash:profile:dvb-dash:2014,urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011" type="dynamic" availabilityStartTime="1970-01-01T00:00:00Z" minimumUpdatePeriod="PT6.000S" publishTime="2021-02-03T10:31:43Z" timeShiftBufferDepth="PT8H0.000S" minBufferTime="PT6.000S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period id="0" start="PT0S">
    <AdaptationSet id="0" maxWidth="1280" maxHeight="720" maxFrameRate="50/1" par="16:9" segmentAlignment="true">
      <SegmentTemplate timescale="48000" duration="288000" startNumber="0" initialization="499cc3ced932a4d6108535e62f880109_0_$RepresentationID$_init.mp4" media="499cc3ced932a4d6108535e62f880109_0_$RepresentationID$-$Number$.mp4"/>
      <Representation id="video_00" mimeType="video/mp4" codecs="avc3.4d401f" bandwidth="900000" width="640" height="360" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
      </Representation>
      <Representation id="video_01" mimeType="video/mp4" codecs="avc3.4d401f" bandwidth="1800000" width="960" height="540" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
      </Representation>
      <Representation id="video_02" mimeType="video/mp4" codecs="avc3.4d4020" bandwidth="3500000" width="1280" height="720" scanType="progressive" frameRate="50/1" sar="1:1" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1" lang="de" segmentAlignment="true">
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <SegmentTemplate timescale="48000" duration="288000" startNumber="0" initialization="499cc3ced932a4d6108535e62f880109_1_$RepresentationID$_init.mp4" media="499cc3ced932a4d6108535e62f880109_1_$RepresentationID$-$Number$.mp4"/>
      <Representation id="audio_03" mimeType="audio/mp4" codecs="mp4a.40.2" bandwidth="128000" audioSamplingRate="48000" startWithSAP="1">
        <BaseURL>http://url/for/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>.</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    check("reDASH", mpd, expected);
}

unittest("reDash: manifest from Keepixo for RBB") {
    auto mpd = R"|(
<?xml version="1.0" encoding="UTF-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-03T10:34:30Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-01-27T17:13:30Z" minBufferTime="PT6S" suggestedPresentationDelay="PT20S" timeShiftBufferDepth="PT2H0M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="540000" startNumber="1611767597"/>
      <Representation id="1" width="1280" height="720" bandwidth="4000000" codecs="avc1.640020" scanType="progressive">
        <SegmentTemplate duration="540000" startNumber="1611767597" media="dash1280x720p50-$Number$.mp4" initialization="dash1280x720p50-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="2000000" codecs="avc1.4d4020" scanType="progressive">
        <SegmentTemplate duration="540000" startNumber="1611767597" media="dash960x540p50-$Number$.mp4" initialization="dash960x540p50-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="540000" startNumber="1611767597" media="dash640x360p50-$Number$.mp4" initialization="dash640x360p50-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="de" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dashAudio-$Number$.mp4" initialization="dashAudio-init.mp4" duration="288000" startNumber="1611767597"/>
      <Representation id="4" bandwidth="96000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
  </Period>
</MPD>
    )|";

	                        auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-03T10:34:30Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-01-27T17:13:30Z" minBufferTime="PT6S" suggestedPresentationDelay="PT20S" timeShiftBufferDepth="PT2H0M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="540000" startNumber="1611767597"/>
      <Representation id="1" width="1280" height="720" bandwidth="4000000" codecs="avc1.640020" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="540000" startNumber="1611767597" media="dash1280x720p50-$Number$.mp4" initialization="dash1280x720p50-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="2000000" codecs="avc1.4d4020" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="540000" startNumber="1611767597" media="dash960x540p50-$Number$.mp4" initialization="dash960x540p50-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="540000" startNumber="1611767597" media="dash640x360p50-$Number$.mp4" initialization="dash640x360p50-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="de" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dashAudio-$Number$.mp4" initialization="dashAudio-init.mp4" duration="288000" startNumber="1611767597"/>
      <Representation id="4" bandwidth="96000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <BaseURL>http://url/for/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>.</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    check("reDASH", mpd, expected);
}

unittest("reDash: manifest from Elemental for RBB (WDR)") {
    auto mpd = R"|(
<?xml version="1.0" encoding="UTF-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-03T11:14:54Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-01-26T10:25:50Z" minBufferTime="PT22S" suggestedPresentationDelay="PT2S" timeShiftBufferDepth="PT2H0M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="360000" startNumber="1611656746"/>
      <Representation id="1" width="1280" height="720" bandwidth="3584000" codecs="avc1.640020" scanType="progressive">
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_1280x720_3584k-$Number$.mp4" initialization="dash_1280x720_3584k-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="1800000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_960x540_1800k-$Number$.mp4" initialization="dash_960x540_1800k-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_640x360_1024k-$Number$.mp4" initialization="dash_640x360_1024k-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="ger" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dash_128k_aac-$Number$.mp4" initialization="dash_128k_aac-init.mp4" duration="192000" startNumber="1611656746"/>
      <Representation id="4" bandwidth="128000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
  </Period>
</MPD>
    )|";

	                                auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-03T11:14:54Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-01-26T10:25:50Z" minBufferTime="PT22S" suggestedPresentationDelay="PT2S" timeShiftBufferDepth="PT2H0M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="360000" startNumber="1611656746"/>
      <Representation id="1" width="1280" height="720" bandwidth="3584000" codecs="avc1.640020" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_1280x720_3584k-$Number$.mp4" initialization="dash_1280x720_3584k-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="1800000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_960x540_1800k-$Number$.mp4" initialization="dash_960x540_1800k-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://url/for/</BaseURL>
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_640x360_1024k-$Number$.mp4" initialization="dash_640x360_1024k-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="ger" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dash_128k_aac-$Number$.mp4" initialization="dash_128k_aac-init.mp4" duration="192000" startNumber="1611656746"/>
      <Representation id="4" bandwidth="128000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <BaseURL>http://url/for/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>.</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    check("reDASH", mpd, expected);
}

unittest("reDash: manifest from Elemental for RBB (WDR) with A/V BaseUrl") {
    auto mpd = R"|(
<?xml version="1.0" encoding="UTF-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-03T11:14:54Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-01-26T10:25:50Z" minBufferTime="PT22S" suggestedPresentationDelay="PT2S" timeShiftBufferDepth="PT2H0M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="360000" startNumber="1611656746"/>
      <Representation id="1" width="1280" height="720" bandwidth="3584000" codecs="avc1.640020" scanType="progressive">
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_1280x720_3584k-$Number$.mp4" initialization="dash_1280x720_3584k-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="1800000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_960x540_1800k-$Number$.mp4" initialization="dash_960x540_1800k-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_640x360_1024k-$Number$.mp4" initialization="dash_640x360_1024k-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="ger" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dash_128k_aac-$Number$.mp4" initialization="dash_128k_aac-init.mp4" duration="192000" startNumber="1611656746"/>
      <Representation id="4" bandwidth="128000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
  </Period>
</MPD>
    )|";

	                                        auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="urn:mpeg:dash:schema:mpd:2011" xmlns:cenc="urn:mpeg:cenc:2013" xsi:schemaLocation="urn:mpeg:dash:schema:mpd:2011 http://standards.iso.org/ittf/PubliclyAvailableStandards/MPEG-DASH_schema_files/DASH-MPD.xsd" type="dynamic" publishTime="2021-02-03T11:14:54Z" minimumUpdatePeriod="PT30S" availabilityStartTime="2021-01-26T10:25:50Z" minBufferTime="PT22S" suggestedPresentationDelay="PT2S" timeShiftBufferDepth="PT2H0M0S" profiles="urn:hbbtv:dash:profile:isoff-live:2012,urn:mpeg:dash:profile:isoff-live:2011">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period start="PT0S" id="1">
    <AdaptationSet mimeType="video/mp4" frameRate="50/1" segmentAlignment="true" subsegmentAlignment="true" startWithSAP="1" subsegmentStartsWithSAP="1" bitstreamSwitching="false">
      <SegmentTemplate timescale="90000" duration="360000" startNumber="1611656746"/>
      <Representation id="1" width="1280" height="720" bandwidth="3584000" codecs="avc1.640020" scanType="progressive">
        <BaseURL>http://test.com/a/</BaseURL>
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_1280x720_3584k-$Number$.mp4" initialization="dash_1280x720_3584k-init.mp4"/>
      </Representation>
      <Representation id="2" width="960" height="540" bandwidth="1800000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://test.com/a/</BaseURL>
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_960x540_1800k-$Number$.mp4" initialization="dash_960x540_1800k-init.mp4"/>
      </Representation>
      <Representation id="3" width="640" height="360" bandwidth="1024000" codecs="avc1.4d401f" scanType="progressive">
        <BaseURL>http://test.com/a/</BaseURL>
        <SegmentTemplate duration="360000" startNumber="1611656746" media="dash_640x360_1024k-$Number$.mp4" initialization="dash_640x360_1024k-init.mp4"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet mimeType="audio/mp4" lang="ger" segmentAlignment="0">
      <SegmentTemplate timescale="48000" media="dash_128k_aac-$Number$.mp4" initialization="dash_128k_aac-init.mp4" duration="192000" startNumber="1611656746"/>
      <Representation id="4" bandwidth="128000" audioSamplingRate="48000" codecs="mp4a.40.2">
        <BaseURL>http://test.com/a/</BaseURL>
        <AudioChannelConfiguration schemeIdUri="urn:mpeg:dash:23003:3:audio_channel_configuration:2011" value="2"/>
      </Representation>
    </AdaptationSet>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>.</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    auto cfg = createRDCfg();
    cfg.baseUrlAV = "http://test.com/a/";
    check("reDASH", mpd, expected, cfg);
}

unittest("reDash: add version when ProgramInfo title is present") {
    auto mpd = R"|(<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S">    
  <ProgramInformation>
    <Title>TEST</Title>
  </ProgramInformation>
</MPD>)|";

	                                                auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S">
  <ProgramInformation>
    <Title>TEST - Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
</MPD>
)|", g_version);

  check("reDASH", mpd, expected);
}

unittest("reDash: add version when ProgramInfo title is absent") {
  auto mpd = R"|(<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S"/>)|";

	                                                        auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
</MPD>
)|", g_version);

  check("reDASH", mpd, expected);
}

unittest("reDash: remote postUrl") {
  auto mpd = R"|(<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S"><Period/></MPD>)|";

	                                                                auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <BaseURL>https://remote/url/</BaseURL>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    auto cfg = createRDCfg();
    cfg.baseUrlSub = "https://remote/url/";
    check("reDASH", mpd, expected, cfg);
}

unittest("reDash: empty baseUrl") {
    auto mpd = R"|(<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S"><Period/></MPD>)|";

	                                                                        auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    auto cfg = createRDCfg();
    cfg.baseUrlSub = "";
    check("reDASH", mpd, expected, cfg);
}

unittest("reDash: general delay") {
    auto mpd = R"|(<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S"><Period/></MPD>)|";

	                                                                                auto expected = format(R"|(<?xml version="1.0" encoding="utf-8"?>
<MPD availabilityStartTime="2020-10-02T17:27:40Z" minimumUpdatePeriod="PT30.00S" timeShiftBufferDepth="PT24H0.00S">
  <ProgramInformation>
    <Title>Updated with Motion Spell / GPAC Licensing subtitle-live-inserter version %s</Title>
  </ProgramInformation>
  <Period>
    <AdaptationSet id="1789" lang="de" segmentAlignment="true">
      <Accessibility schemeIdUri="urn:tva:metadata:cs:AudioPurposeCS:2007" value="2"/>
      <Role schemeIdUri="urn:mpeg:dash:role:2011" value="main"/>
      <SegmentTemplate timescale="10000000" duration="20000000" startNumber="0" initialization="s_$RepresentationID$-init.mp4" media="s_$RepresentationID$-$Number$.m4s"/>
      <Representation id="0" mimeType="application/mp4" codecs="stpp" bandwidth="9600" startWithSAP="1"/>
    </AdaptationSet>
  </Period>
</MPD>
)|", g_version);

    auto cfg = createRDCfg();
    cfg.baseUrlSub = "";
    cfg.delayInSec = 2;
    check("reDASH", mpd, expected, cfg);
}

unittest("reDash: sanity checks") {
    auto mpd = R"|(<MPD availabilityStartTime="2020-10-02T17:27:38Z" minimumUpdatePeriod="PT30.00S"><Period/></MPD>)|";
	                                                                                        auto cfg = createRDCfg();
	                                                                                        cfg.baseUrlSub = "";
	                                                                                        ASSERT_THROWN(check("reDASH", mpd, "", cfg));
}

}
