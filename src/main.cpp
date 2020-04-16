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
	opt.add("g", "general-delay", &cfg.delayInSec, "General delay in seconds (signed).");
	opt.add("s", "subtitle-delay", &cfg.subtitleForwardTimeInSec, "Subtitle delay in seconds (signed).");
	opt.add("f", "file-playlist", &cfg.subListFn, "File path of the ever-growing playlist. If not set then synthetic content is generated.");
	opt.add("p", "post", &cfg.postUrl, "Path or URL where the content is posted.");
	opt.addFlag("h", "help", &cfg.help, "Print usage and exit.");

	auto urls = opt.parse(argc, argv);

	if(cfg.help) {
		opt.printHelp();
		return cfg;
	}

	if (urls.size() != 1) {
		opt.printHelp();
		if (urls.empty())
			throw std::runtime_error("No MPD URLs detected. Exit.");
		else
			throw std::runtime_error("Several MPD URLs detected. Exit.");
	}
	cfg.url = urls[0];

	std::cerr << "Detected options:\n"
		"\turl=\"" << cfg.url << "\"\n\tdelayInSec=" << cfg.delayInSec << "\n"
		"\tsubtitleForwardTimeInSec=" << cfg.subtitleForwardTimeInSec << "\n"
		"\tsubListFn=\"" << cfg.subListFn << "\"\n"
		"\tpost=\"" << cfg.postUrl << "\"\n";

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
