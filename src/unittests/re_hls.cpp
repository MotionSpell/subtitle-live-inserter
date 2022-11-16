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
http://url/for/index_v.m3u8

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio-aacl-main-96",NAME="VF",LANGUAGE="fre",AUTOSELECT=YES,DEFAULT=YES,CHANNELS="2",URI="http://url/for/index_a.m3u8"

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version);

    check("reHLS", m3u8, expected);
}

unittest("reHLS: manifest from Elemental (absolute URLs)") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4285248,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4285248,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2259400,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2259400,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=670450,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_578.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=670450,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_578.m3u8)|";

	auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4285248,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4285248,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2259400,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2259400,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=670450,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_578.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=670450,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000,SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_578.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version);

    check("reHLS", m3u8, expected);
}

unittest("reHLS: manifest from Elemental (relative URLs)") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_578.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_578.m3u8)|";

	        auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025-b/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025-b/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025-b/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025/wdrlz_muensterland/master_578.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://url/hls/live/2018025-b/wdrlz_muensterland/master_578.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version);

    check("reHLS", m3u8, expected);
}

unittest("reHLS: manifest from Elemental (relative URLs wih source BaseUrl)") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000
/hls/live/2018025/wdrlz_muensterland/master_578.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000
/hls/live/2018025-b/wdrlz_muensterland/master_578.m3u8)|";

	                auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=5349696,AVERAGE-BANDWIDTH=3990800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025-b/wdrlz_muensterland/master_3628.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2794000,AVERAGE-BANDWIDTH=2120800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=960x540,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025-b/wdrlz_muensterland/master_1928.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1526800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025-b/wdrlz_muensterland/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025/wdrlz_muensterland/master_578.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=804100,AVERAGE-BANDWIDTH=635800,CODECS="avc1.66.30,mp4a.40.2",RESOLUTION=480x270,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://test.com/a/hls/live/2018025-b/wdrlz_muensterland/master_578.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="http//test2.com/b/index_sub.m3u8"
)|", g_appName, g_version);

	auto cfg = createRDCfg();
	cfg.baseUrlAV = "http://test.com/a/";
	cfg.baseUrlSub = "http//test2.com/b/";
    check("reHLS", m3u8, expected, cfg);
}

unittest("reHLS: replace existing subtitle") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subs"
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subs"
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-MEDIA:TYPE=SUBTITLES,NAME="Untertitel",DEFAULT=YES,AUTOSELECT=YES,FORCED=NO,LANGUAGE="ger",GROUP-ID="subs",URI="https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_subs.m3u8")|";

	                        auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subs",SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628/wdr_msl4_fs247ww/master_1028.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1229800,AVERAGE-BANDWIDTH=1130800,CODECS="avc1.4d401f,mp4a.40.2",RESOLUTION=640x360,FRAME-RATE=50.000,SUBTITLES="subs",SUBTITLES="subtitles"
https://wdrfsww247.akamaized.net/hls/live/2009628-b/wdr_msl4_fs247ww/master_1028.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version);

    check("reHLS", m3u8, expected);
}


unittest("reHLS: combined baseUrls") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
master_3328.m3u8
)|";

	auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://A.com/a1/a2/master_3328.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="http://B.com/b1/b2/index_sub.m3u8"
)|", g_appName, g_version);

    auto cfg = createRDCfg();
	cfg.url = "http://S.com/s1/s2/live.m3u8";
    cfg.baseUrlAV = "http://A.com/a1/a2/";
    cfg.baseUrlSub = "http://B.com/b1/b2/";
    check("reHLS", m3u8, expected, cfg);
}

unittest("reHLS: subtitle naming") {
	auto m3u8 = R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
master_3328.m3u8
)|";

	auto expected = format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://A.com/a1/a2/master_3328.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="toto",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="http://B.com/b1/b2/index_sub.m3u8"
)|", g_appName, g_version);

	auto cfg = createRDCfg();
	cfg.displayedName = "toto";
	cfg.url = "http://S.com/s1/s2/live.m3u8";
	cfg.baseUrlAV = "http://A.com/a1/a2/";
	cfg.baseUrlSub = "http://B.com/b1/b2/";
	check("reHLS", m3u8, expected, cfg);
}

unittest("reHLS: global delay (BaseURLs set)") {
	auto cfg = createRDCfg();
	cfg.delayInSec = 2;
	cfg.displayedName = "toto";
	cfg.url = "http://S.com/s1/s2/live.m3u8";
	cfg.manifestFn = "titi";
	cfg.baseUrlAV = "http://A.com/a1/a2/";
	cfg.baseUrlSub = "http://B.com/b1/b2/";

	auto master = /*master*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
master_3328.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
http://xxx.com/m3u8/master_3329.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
/master_3330.m3u8
)|";
	auto inputs = std::vector<const char*> ({
		master, master,
		/*variant master_3328.m3u8*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXTINF:11.360,
sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts
)|",	/*variant master_3329.m3u8*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXTINF:11.360,
sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts
)|",	/*variant master_3330.m3u8*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXTINF:11.360,
sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts
)|" });

	auto expected = std::vector<std::string> ({
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:11.360,
http://A.com/a1/a2/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://A.com/a1/a2/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:11.360,
http://A.com/a1/a2/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://A.com/a1/a2/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:11.360,
http://A.com/a1/a2/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://A.com/a1/a2/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://B.com/b1/b2/master_3328.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://B.com/b1/b2/master_3329.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
http://B.com/b1/b2/master_3330.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="toto",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="http://B.com/b1/b2/index_sub.m3u8"
)|", g_appName, g_version),
	});

	FilePullerFactory filePullerFactory(inputs);
	cfg.filePullerFactory = &filePullerFactory;
	auto redash = loadModule("reHLS", &NullHost, &cfg);
	auto recorder = createModule<Utils::Recorder>(&NullHost);
	ConnectOutputToInput(redash->getOutput(0), recorder->getInput(0));
	redash->process();

	Data data;
	bool ret;
	std::vector<std::string> expectedFilenames = { "master_3328.m3u8", "master_3329.m3u8", "master_3330.m3u8", cfg.manifestFn };
	for (size_t i=0; i<expectedFilenames.size(); ++i) {
		ret = recorder->tryPop(data);
		ASSERT_EQUALS(true, ret);

		auto dataRaw = std::dynamic_pointer_cast<const DataRaw>(data);
		ASSERT(dataRaw);

		auto meta = std::dynamic_pointer_cast<const MetadataFile>(data->getMetadata());
		ASSERT(meta);
		ASSERT_EQUALS(expectedFilenames[i], meta->filename);

		ASSERT_EQUALS(expected[i], std::string((const char*)data->data().ptr, data->data().len).c_str());
	}

	ret = recorder->tryPop(data);
	ASSERT_EQUALS(false, ret);
}

unittest("reHLS: global delay (BaseURLs not set)") {
	auto cfg = createRDCfg();
	cfg.delayInSec = 2;
	cfg.displayedName = "toto";
	cfg.url = "http://S.com/s1/s2/live.m3u8";
	cfg.manifestFn = "titi";

	auto master = /*master*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
prefix1/master_3328.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
http://xxx.com/prefix2/master_3329.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000
/master_3330.m3u8
)|";
	auto inputs = std::vector<const char*> ({
		master, master,
		/*variant master_3328.m3u8*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXTINF:11.360,
sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts
)|",	/*variant master_3329.m3u8*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXTINF:11.360,
sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts
)|",	/*variant master_3330.m3u8*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXTINF:11.360,
sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts
)|" });

	auto expected = std::vector<std::string> ({
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:11.360,
http://S.com/s1/s2/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://S.com/s1/s2/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3328(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:11.360,
http://S.com/s1/s2/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://S.com/s1/s2/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3329(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-PLAYLIST-TYPE:VOD
#EXT-X-TARGETDURATION:14
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:11.360,
http://S.com/s1/s2/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(1)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://S.com/s1/s2/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(2)/video/107/282/158282701_mp4_h264_aac_hq.ts
#EXTINF:11.320,
http://xxx.com/segments/sec3330(3ae40f708f79ca9471f52b86da76a3a8)/frag(3)/video/107/282/158282701_mp4_h264_aac_hq.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
./prefix1/master_3328.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
./master_3329.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4224000,AVERAGE-BANDWIDTH=3660800,CODECS="avc1.640020,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=50.000,SUBTITLES="subtitles"
./master_3330.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="toto",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version),
	});

	FilePullerFactory filePullerFactory(inputs);
	cfg.filePullerFactory = &filePullerFactory;
	auto redash = loadModule("reHLS", &NullHost, &cfg);
	auto recorder = createModule<Utils::Recorder>(&NullHost);
	ConnectOutputToInput(redash->getOutput(0), recorder->getInput(0));
	redash->process();

	Data data;
	bool ret;
	std::vector<std::string> expectedFilenames = { "master_3328.m3u8", "master_3329.m3u8", "master_3330.m3u8", cfg.manifestFn };
	for (size_t i=0; i<expectedFilenames.size(); ++i) {
		ret = recorder->tryPop(data);
		ASSERT_EQUALS(true, ret);

		auto dataRaw = std::dynamic_pointer_cast<const DataRaw>(data);
		ASSERT(dataRaw);

		auto meta = std::dynamic_pointer_cast<const MetadataFile>(data->getMetadata());
		ASSERT(meta);
		ASSERT_EQUALS(expectedFilenames[i], meta->filename);

		ASSERT_EQUALS(expected[i], std::string((const char*)data->data().ptr, data->data().len).c_str());
	}

	ret = recorder->tryPop(data);
	ASSERT_EQUALS(false, ret);

	ASSERT(filePullerFactory.instance);
	std::vector<std::string> expectedRequestedURLs = {{"http://S.com/s1/s2/live.m3u8", "http://S.com/s1/s2/live.m3u8", "http://S.com/s1/s2/prefix1/master_3328.m3u8", "http://xxx.com/prefix2/master_3329.m3u8", "http://S.com//master_3330.m3u8"}};
	ASSERT_EQUALS(expectedRequestedURLs, ((MemoryFileSystem*)(filePullerFactory.instance))->requestedURLs);
}

unittest("reHLS: global delay (separate audio, baseUrlAV only)") {
	auto cfg = createRDCfg();
	cfg.delayInSec = 2;
	cfg.url = "http://127.0.0.1:8889/live.m3u8";
	cfg.baseUrlAV = "http://127.0.0.1:9000/";
	cfg.manifestFn = "titi";

	auto master = /*master*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio1",NAME="1",AUTOSELECT=YES,URI="live_1.m3u8",CHANNELS="1"
#EXT-X-STREAM-INF:BANDWIDTH=100000,CODECS="avc1.64001F,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=25,AUDIO="audio1"
live_2.m3u8
)|";
	auto inputs = std::vector<const char*> ({
		master, master,
		/*variant live_1.m3u8*/
		R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXTINF:1.99692,
audio_dash_track1_576.ts
)|",	/*variant live_2.m3u8*/
		R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXTINF:2,
video_dash_track2_576.ts
)|" });

	auto expected = std::vector<std::string> ({
		format(R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:1.99692,
http://127.0.0.1:9000/audio_dash_track1_576.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:2,
http://127.0.0.1:9000/video_dash_track2_576.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio1",NAME="1",AUTOSELECT=YES,URI="./live_1.m3u8",CHANNELS="1"
#EXT-X-STREAM-INF:BANDWIDTH=100000,CODECS="avc1.64001F,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=25,AUDIO="audio1",SUBTITLES="subtitles"
./live_2.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="./index_sub.m3u8"
)|", g_appName, g_version),
	});

	FilePullerFactory filePullerFactory(inputs);
	cfg.filePullerFactory = &filePullerFactory;
	auto redash = loadModule("reHLS", &NullHost, &cfg);
	auto recorder = createModule<Utils::Recorder>(&NullHost);
	ConnectOutputToInput(redash->getOutput(0), recorder->getInput(0));
	redash->process();

	Data data;
	bool ret;
	std::vector<std::string> expectedFilenames = { "live_1.m3u8", "live_2.m3u8", cfg.manifestFn };
	for (size_t i=0; i<expectedFilenames.size(); ++i) {
		ret = recorder->tryPop(data);
		ASSERT_EQUALS(true, ret);

		auto dataRaw = std::dynamic_pointer_cast<const DataRaw>(data);
		ASSERT(dataRaw);

		auto meta = std::dynamic_pointer_cast<const MetadataFile>(data->getMetadata());
		ASSERT(meta);
		ASSERT_EQUALS(expectedFilenames[i], meta->filename);

		ASSERT_EQUALS(expected[i], std::string((const char*)data->data().ptr, data->data().len).c_str());
	}

	ret = recorder->tryPop(data);
	ASSERT_EQUALS(false, ret);

	ASSERT(filePullerFactory.instance);
	std::vector<std::string> expectedRequestedURLs = {{"http://127.0.0.1:8889/live.m3u8", "http://127.0.0.1:8889/live.m3u8", "http://127.0.0.1:8889/live_1.m3u8", "http://127.0.0.1:8889/live_2.m3u8"}};
	ASSERT_EQUALS(expectedRequestedURLs, ((MemoryFileSystem*)(filePullerFactory.instance))->requestedURLs);
}

unittest("reHLS: global delay (separate audio, baseURLs set)") {
	auto cfg = createRDCfg();
	cfg.delayInSec = 2;
	cfg.url = "http://127.0.0.1:8889/live.m3u8";
	cfg.baseUrlAV = "http://127.0.0.1:9000/AV/";
	cfg.baseUrlSub = "http://127.0.0.1:9001/SUB/";
	cfg.manifestFn = "titi";

	auto master = /*master*/
		R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio1",NAME="1",AUTOSELECT=YES,URI="live_1.m3u8",CHANNELS="1"
#EXT-X-STREAM-INF:BANDWIDTH=100000,CODECS="avc1.64001F,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=25,AUDIO="audio1"
live_2.m3u8
)|";
	auto inputs = std::vector<const char*> ({
		master, master,
		/*variant live_1.m3u8*/
		R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXTINF:1.99692,
audio_dash_track1_576.ts
)|",	/*variant live_2.m3u8*/
		R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXTINF:2,
video_dash_track2_576.ts
)|" });

	auto expected = std::vector<std::string> ({
		format(R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:1.99692,
http://127.0.0.1:9000/AV/audio_dash_track1_576.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXT-X-VERSION:3
#EXT-X-MEDIA-SEQUENCE:576
#EXT-X-INDEPENDENT-SEGMENTS
#EXT-X-START:TIME-OFFSET=%s
#EXTINF:2,
http://127.0.0.1:9000/AV/video_dash_track2_576.ts

## Updated with Motion Spell / GPAC Licensing %s version %s
)|", -cfg.delayInSec, g_appName, g_version),
		format(R"|(#EXTM3U
#EXT-X-VERSION:3
#EXT-X-INDEPENDENT-SEGMENTS

#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID="audio1",NAME="1",AUTOSELECT=YES,URI="http://127.0.0.1:9001/SUB/live_1.m3u8",CHANNELS="1"
#EXT-X-STREAM-INF:BANDWIDTH=100000,CODECS="avc1.64001F,mp4a.40.2",RESOLUTION=1280x720,FRAME-RATE=25,AUDIO="audio1",SUBTITLES="subtitles"
http://127.0.0.1:9001/SUB/live_2.m3u8

## Updated with Motion Spell / GPAC Licensing %s version %s
#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID="subtitles",NAME="subtitle",LANGUAGE="de",AUTOSELECT=YES,DEFAULT=NO,FORCED=NO,URI="http://127.0.0.1:9001/SUB/index_sub.m3u8"
)|", g_appName, g_version),
	});

	FilePullerFactory filePullerFactory(inputs);
	cfg.filePullerFactory = &filePullerFactory;
	auto redash = loadModule("reHLS", &NullHost, &cfg);
	auto recorder = createModule<Utils::Recorder>(&NullHost);
	ConnectOutputToInput(redash->getOutput(0), recorder->getInput(0));
	redash->process();

	Data data;
	bool ret;
	std::vector<std::string> expectedFilenames = { "live_1.m3u8", "live_2.m3u8", cfg.manifestFn };
	for (size_t i=0; i<expectedFilenames.size(); ++i) {
		ret = recorder->tryPop(data);
		ASSERT_EQUALS(true, ret);

		auto dataRaw = std::dynamic_pointer_cast<const DataRaw>(data);
		ASSERT(dataRaw);

		auto meta = std::dynamic_pointer_cast<const MetadataFile>(data->getMetadata());
		ASSERT(meta);
		ASSERT_EQUALS(expectedFilenames[i], meta->filename);

		ASSERT_EQUALS(expected[i], std::string((const char*)data->data().ptr, data->data().len).c_str());
	}

	ret = recorder->tryPop(data);
	ASSERT_EQUALS(false, ret);

	ASSERT(filePullerFactory.instance);
	std::vector<std::string> expectedRequestedURLs = {{"http://127.0.0.1:8889/live.m3u8", "http://127.0.0.1:8889/live.m3u8", "http://127.0.0.1:8889/live_1.m3u8", "http://127.0.0.1:8889/live_2.m3u8"}};
	ASSERT_EQUALS(expectedRequestedURLs, ((MemoryFileSystem*)(filePullerFactory.instance))->requestedURLs);
}

}
