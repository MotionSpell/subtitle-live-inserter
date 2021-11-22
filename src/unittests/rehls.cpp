#include "common.hpp"
#include "lib_utils/format.hpp"

extern const char *g_appName;

namespace {

unittest("reHLS: manifest from Quortex") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:7
## Just In Time Delivered by Quortex Solution, version 1.0.22
#EXT-X-INDEPENDENT-SEGMENTS

#EXT-X-STREAM-INF:BANDWIDTH=800000,RESOLUTION=640x360,FRAME-RATE=25,CODECS="avc1.42001e,mp4a.40.2",AUDIO="audio-aacl-main-96"
index_v.m3u8

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio-aacl-main-96",NAME="VF",LANGUAGE="fre",AUTOSELECT=YES,DEFAULT=YES,CHANNELS="2",URI="index_a.m3u8"
)|";

	auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:7
## Just In Time Delivered by Quortex Solution, version 1.0.22
#EXT-X-INDEPENDENT-SEGMENTS

#EXT-X-STREAM-INF:BANDWIDTH=800000,RESOLUTION=640x360,FRAME-RATE=25,CODECS="avc1.42001e,mp4a.40.2",AUDIO="audio-aacl-main-96",SUBTITLES="subtitles"
./index_v.m3u8

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio-aacl-main-96",NAME="VF",LANGUAGE="fre",AUTOSELECT=YES,DEFAULT=YES,CHANNELS="2",URI="./index_a.m3u8"

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitles",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version);

    check("reHLS", m3u8, expected);
}

unittest("reHLS: manifest from Elemental") {
    //Romain: TODO
}

}
