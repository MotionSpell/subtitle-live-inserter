#include "lib_utils/format.hpp"
#include "lib_utils/log.hpp" // g_Log
#include "lib_utils/profiler.hpp"
#include "lib_pipeline/pipeline.hpp"
#include "lib_appcommon/options.hpp"
#include "options.hpp"
#include <iostream>

const char *g_appName = "subtitle-live-inserter";

std::unique_ptr<Pipelines::Pipeline> buildPipeline(const Config&);
static Pipelines::Pipeline *g_Pipeline = nullptr;

namespace {
Config parseCommandLine(int argc, char const* argv[]) {
	Config cfg;

	CmdLineOptions opt;
	opt.add("d", "delay", &cfg.delayInSec, "General delay in seconds (signed).");
	opt.add("s", "subdelay", &cfg.subtitleForwardTimeInSec, "Subtitle delay in seconds (signed).");
	opt.add("p", "output", &cfg.subListFn, "File path of the ever-growing playlist. If not set then synthetic content is generated.");
	opt.addFlag("h", "help", &cfg.help, "Print usage and exit.");

	auto urls = opt.parse(argc, argv);
	if (urls.size() != 1)
		throw std::runtime_error("Several MPD URLs detected, use --help");
	cfg.url = urls[0];

	if(cfg.help) {
		std::cout << "Usage: " << g_appName << " mpd_url [delay_in_seconds] [subtitleForwardTimeInSec] [sub_list_filename]" << std::endl;
		opt.printHelp();
		return cfg;
	}

	std::cerr << "Detected options:\n\turl=\"" << cfg.url << "\"\n\tdelayInSec=" << cfg.delayInSec << "\n\tsubtitleForwardTimeInSec=" << cfg.subtitleForwardTimeInSec << "\n\tsubListFn=\"" << cfg.subListFn << "\"\n";

	return cfg;
}
}

void safeMain(int argc, const char* argv[]) {
	auto const cfg = parseCommandLine(argc, argv);
	if(cfg.help)
		return;

	auto pipeline = buildPipeline(cfg);
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
