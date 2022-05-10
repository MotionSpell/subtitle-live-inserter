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

const char *g_appName = "subtitle-live-inserter";

std::unique_ptr<Pipelines::Pipeline> buildPipeline(Config&);
static Pipelines::Pipeline *g_Pipeline = nullptr;

namespace {
Config parseCommandLine(int argc, char const* argv[]) {
	Config cfg;

	CmdLineOptions opt;
	opt.add("g", "general-delay", &cfg.delayInSec, "General delay in seconds (signed).");
	opt.add("s", "subtitle-delay", &cfg.subtitleForwardTimeInSec, "Subtitle delay in seconds (signed).");
	opt.add("f", "file-playlist", &cfg.subListFn, "File path of the ever-growing playlist. If not set then synthetic content is generated.");
	opt.add("u", "output-format", &cfg.outputFormat, "Output format: \"dash\" (default) or \"hls\".");
	opt.add("o", "output-manifest-filename", &cfg.manifestFn, "Manifest filename. If not specified the input filename is copied.");
	opt.add("a", "base-url-av", &cfg.baseUrlAV, "Explicit URL where the source A/V content can be played back by end-users. Useful when the source URL used by the tool is different from the one end-users access.");
	opt.add("b", "base-url-sub", &cfg.baseUrlSub, "Explicit URL where the subtitle content can be played back by end-users. Useful when the content is accessible from a different URL than where we post.");
	opt.add("n", "sub-displayed-name", &cfg.displayedName, "Default=\"subtitle\". Explicit URL where the subtitle content can be played back by end-users. Useful when the content is accessible from a different URL than where we post.");
	opt.add("o", "output-manifest-filename", &cfg.manifestFn, "Manifest filename. If not specified the input filename is copied.");
	opt.add("t", "timeshift-buffer", &cfg.timeshiftBufferDepthInSec, "Default=-1 (same as source). Infinite=0. Value in seconds otherwise (DASH only).");
	opt.add("p", "post", &cfg.postUrl, "Path or URL where the content is posted. If not set the content is generated locally.");
	opt.addFlag("r", "rectify", &cfg.rectify, "Add empty samples when input content is not available on time. Default off.");
	opt.addFlag("h", "help", &cfg.help, "Print usage and exit.");
	opt.addFlag("i", "shell", &cfg.shell, "Enable the interactive shell.");

	auto urls = opt.parse(argc, argv);

	if(cfg.help) {
		opt.printHelp();
		return cfg;
	}

	if (cfg.outputFormat != "dash" && cfg.outputFormat != "hls")
		throw std::runtime_error("invalid output format, shall be \"dash\" or \"hls\"");

	if (urls.size() != 1) {
		opt.printHelp();
		if (urls.empty())
			throw std::runtime_error("No MPD URLs detected. Exit.");
		else
			throw std::runtime_error("Several MPD URLs detected. Exit.");
	}
	cfg.url = urls[0];

	std::cerr << "Detected options:\n"
	    "\turl                     =\"" << cfg.url << "\"\n"
	    "\toutput format           =\"" << cfg.outputFormat << "\"\n"
	    "\trectify                 =" << cfg.rectify << "\n"
	    "\tdelayInSec              =" << cfg.delayInSec << "\n"
	    "\tsubtitleForwardTimeInSec=" << cfg.subtitleForwardTimeInSec << "\n"
	    "\tsubListFn               =\"" << cfg.subListFn << "\"\n"
	    "\tmanifestFn              =\"" << cfg.manifestFn << "\"\n"
	    "\tbaseUrlAV               =\"" << cfg.baseUrlAV << "\"\n"
	    "\tbaseUrlSub              =\"" << cfg.baseUrlSub << "\"\n"
	    "\ttimeshiftBufferDepth    =\"" << cfg.timeshiftBufferDepthInSec << "\"\n"
	    "\tpost                    =\"" << cfg.postUrl << "\"\n";

	return cfg;
}

void setAction(Config *cfg, std::string parameters) {
	std::vector<std::string> vParams;
	{
		std::istringstream isParam(parameters);
		for (std::string param; std::getline(isParam, param, ' '); )
			vParams.push_back(param);

		if (vParams.size() != 2) {
			std::string err = "Expected 1 command + 1 parameter for command \"set\", got (" + std::to_string(vParams.size()) + "): \"" + parameters + "\"";
			throw std::runtime_error(err.c_str());
		}
	}

	auto &command = vParams[0];
	if (command == "delay") {
		int delayInSec = 0;
		std::istringstream isParam(vParams[1]);
		isParam >> delayInSec;
		std::cout << "executing: set " << command << " " << delayInSec;
		cfg->updateDelayInSec(delayInSec);
		cfg->delayInSec = delayInSec;
		std::cout << "done" << std::endl;
	} else if (command == "subfwd") {
		int subtitleForwardTimeInSec = 0;
		std::istringstream isParam(vParams[1]);
		isParam >> subtitleForwardTimeInSec;
		std::cout << "executing: set " << command << " " << subtitleForwardTimeInSec;
		cfg->subtitleForwardTimeInSec = subtitleForwardTimeInSec;
		std::cout << "... stopping the current pipeline: please update your player ..." << std::endl;
		g_Pipeline->exitSync();
		std::cout << "exited" << std::endl;
	} else {
		std::string err = "set: unknown command \"" + parameters + "\"";
		throw std::runtime_error(err.c_str());
	}
}
}

void safeMain(int argc, const char* argv[]) {
	auto cfg = parseCommandLine(argc, argv);
	if(cfg.help)
		return;

	bool exit = false;

	if (cfg.shell) {
		auto shell = std::shared_ptr<Shell>(new Shell, [&](Shell *s) {
			if (g_Pipeline)
				g_Pipeline->exitSync();
			delete s;
			exit = true;
		});
		shell->addAction("set", std::bind(setAction, &cfg, std::placeholders::_1));
		std::thread shellThread(&Shell::run, shell.get());
	}

	while (!exit) {
		auto pipeline = buildPipeline(cfg);
		g_Pipeline = pipeline.get();

		{
			Tools::Profiler profilerProcessing(format("%s - processing time", g_appName));
			pipeline->start();
			pipeline->waitForEndOfStream();
		}
	}
}
