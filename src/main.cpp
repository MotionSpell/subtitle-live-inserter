#include <iostream>
#include "lib_utils/format.hpp"
#include "lib_utils/log.hpp" // g_Log
#include "lib_utils/profiler.hpp"
#include "lib_pipeline/pipeline.hpp"

const char *g_appName = "subtitle-live-inserter";

std::unique_ptr<Pipelines::Pipeline> buildPipeline(const char *subListFilename, const char *mpdUrl);
static Pipelines::Pipeline *g_Pipeline = nullptr;

void safeMain(int argc, const char* argv[]) {
	if (argc != 2 && argc != 3) {
		std::cout << "Usage: " << g_appName << " mpd_url [sub_list_filename]" << std::endl;
		exit(1);
	}

	auto pipeline = buildPipeline(argv[1], argc == 3 ? argv[2] : "");
	g_Pipeline = pipeline.get();

	Tools::Profiler profilerProcessing(format("%s - processing time", g_appName));
	pipeline->start();
	pipeline->waitForEndOfStream();
}

void safeStop() {
	if (g_Pipeline) {
		g_Pipeline->exitSync();
		g_Pipeline = nullptr;
	}
}
