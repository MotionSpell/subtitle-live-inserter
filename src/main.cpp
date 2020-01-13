#include <iostream>
#include "lib_utils/format.hpp"
#include "lib_utils/log.hpp" // g_Log
#include "lib_utils/profiler.hpp"
#include "lib_pipeline/pipeline.hpp"

const char *g_appName = "subtitle-live-inserter";

std::unique_ptr<Pipelines::Pipeline> buildPipeline(const char *subListFilename, const int delayInSec, const char *mpdUrl);
static Pipelines::Pipeline *g_Pipeline = nullptr;

void safeMain(int argc, const char* argv[]) {
	if (argc < 2 || argc > 4) {
		std::cout << "Usage: " << g_appName << " mpd_url [delay_in_seconds] [sub_list_filename]" << std::endl;
		exit(1);
	}

	auto url = argv[1];
	auto delayInSec = argc > 2 ? atoi(argv[2]) : 0;
	auto subListFn = argc > 3 ? argv[3] : "";
	std::cerr << "Detected options:\n\turl=\"" << url << "\"\n\tdelayInSec=" << delayInSec << "\n\tsubListFn=\"" << subListFn << "\"\n";

	auto pipeline = buildPipeline(url, delayInSec, subListFn);
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
