#include "lib_utils/format.hpp"
#include "lib_utils/log.hpp" // g_Log
#include "lib_utils/profiler.hpp"
#include "lib_pipeline/pipeline.hpp"
#include "lib_appcommon/options.hpp"
#include "options.hpp"
#include "shell.hpp"
#include <iostream>
#include <sstream> // istringstream
#include <thread>

extern const char *g_appName;

std::unique_ptr<Pipelines::Pipeline> buildPipeline(const Config&);
static Pipelines::Pipeline *g_Pipeline = nullptr;

namespace {
Config parseCommandLine(int argc, char const* argv[]) {
	Config cfg;

	CmdLineOptions opt;
	opt.add("g", "general-delay", &cfg.delayInSec, "General delay in seconds (signed).");
	opt.add("s", "subtitle-delay", &cfg.subtitleForwardTimeInSec, "Subtitle delay in seconds (signed).");
	opt.add("f", "file-playlist", &cfg.subListFn, "File path of the ever-growing playlist. If not set then synthetic content is generated.");
	opt.add("o", "output-mpd-filename", &cfg.mpdFn, "Manifest filename. If not specified the input filename is copied.");
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
	    "\turl=\"" << cfg.url << "\"\n"
	    "\tdelayInSec=" << cfg.delayInSec << "\n"
	    "\tsubtitleForwardTimeInSec=" << cfg.subtitleForwardTimeInSec << "\n"
	    "\tsubListFn=\"" << cfg.subListFn << "\"\n"
	    "\tmpdFn=\"" << cfg.mpdFn << "\"\n"
	    "\tpost=\"" << cfg.postUrl << "\"\n";

	return cfg;
}

void getAction(std::string parameters) {
	std::istringstream isParam(parameters);

	{
		std::vector<std::string> params;
		for (std::string param; std::getline(isParam, param, ' '); )
			params.push_back(param);

		if (params.size() != 3) {
			std::string err = "Expected 1 command + 1 parameter for command \"get\", got (" + std::to_string(params.size()) + "): \"" + parameters + "\"";
			throw std::runtime_error(err.c_str());
		}
	}

	std::string command;
	isParam >> command;
	if (command == "delay") {
		int delayInSec = 0;
		isParam >> delayInSec;
		std::cout << "executing: get " << command << " " << delayInSec;
	} else if (command == "subfwd") {
		int subtitleForwardTimeInSec = 0;
		isParam >> subtitleForwardTimeInSec;
		std::cout << "executing: get " << command << " " << subtitleForwardTimeInSec;
	} else {
		std::string err = "get: unknown command \"" + parameters + "\"";
		throw std::runtime_error(err.c_str());
	}
}
}

void safeStop() {
	if (g_Pipeline) {
		g_Pipeline->exitSync();
		g_Pipeline = nullptr;
	}
}

void safeMain(int argc, const char* argv[]) {
	auto const cfg = parseCommandLine(argc, argv);
	if(cfg.help)
		return;

	auto pipeline = buildPipeline(cfg);
	g_Pipeline = pipeline.get();

	auto shell = std::shared_ptr<Shell>(new Shell, [](Shell *s) {
		safeStop();
		delete s;
	}) ;
	shell->addAction("get", getAction);
	std::thread shellThread(&Shell::run, shell.get());

	{
		Tools::Profiler profilerProcessing(format("%s - processing time", g_appName));
		pipeline->start();
		pipeline->waitForEndOfStream();
	}
}
