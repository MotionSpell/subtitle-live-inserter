#pragma once

#include "lib_media/mux/mux_mp4_config.hpp"

struct Mp4MuxFileHandlerDynConfig {
	Modules::Mp4MuxConfig *mp4MuxCfg = nullptr;
	int64_t timeshiftBufferDepthInSec = 0;
};
