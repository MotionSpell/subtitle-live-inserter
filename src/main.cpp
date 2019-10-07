#include <iostream>
#include <stdexcept>
#include "lib_utils/format.hpp"
#include "lib_utils/log.hpp" // g_Log
#include "lib_utils/tools.hpp"
#include "lib_utils/profiler.hpp"
#include "lib_pipeline/pipeline.hpp"

const char *g_appName = "subtitle-live-inserter";

std::unique_ptr<Pipelines::Pipeline> buildPipeline(const char *subListFilename);
static Pipelines::Pipeline *g_Pipeline = nullptr;

void safeMain(int argc, const char* argv[]) {
	try {
		if (argc != 2) {
			std::cout << "Usage: " << g_appName << " sub_list_filename" << std::endl;
			exit(1);
		}

		auto pipeline = buildPipeline(argv[1]);
		g_Pipeline = pipeline.get();

		Tools::Profiler profilerProcessing(format("%s - processing time", g_appName));
		pipeline->start();
		pipeline->waitForEndOfStream();
	} catch (std::exception const& e) {
		g_Log->log(Error, format("Caught error: %s", e.what()).c_str());
		throw;
	}
}

void safeStop() {
	if (g_Pipeline) {
		g_Pipeline->exitSync();
		g_Pipeline = nullptr;
	}
}
