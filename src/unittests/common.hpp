#pragma once

#include "tests/tests.hpp"
#include "../re_dash.hpp"
#include "lib_modules/core/connection.hpp"
#include "lib_modules/utils/helper.hpp" // NullHost
#include "lib_modules/utils/loader.hpp"
#include "lib_media/common/file_puller.hpp"
#include "lib_media/common/metadata_file.hpp"
#include "lib_media/utils/recorder.hpp"

using namespace Modules;

extern const char *g_version;

namespace {

struct MemoryFileSystem : In::IFilePuller {
	MemoryFileSystem(std::vector<const char*> srcs) : srcs(srcs) {}
	void wget(const char* szUrl, std::function<void(SpanC)> callback) override {
		requestedURLs.push_back(szUrl);
		ASSERT(!srcs.empty());
		callback({(const uint8_t*)srcs[index], strlen(srcs[index])});
		index = (index + 1) % srcs.size(); /*loop on single source (manifest-only)*/
	}
	void askToExit() override {}
	std::vector<const char*> srcs;
	std::vector<std::string> requestedURLs;
	size_t index = 0;
};

struct FilePullerFactory : In::IFilePullerFactory {
	FilePullerFactory(std::vector<const char*> srcs) : srcs(srcs) {}
	std::unique_ptr<In::IFilePuller> create() override {
		auto ret = std::make_unique<MemoryFileSystem>(srcs);
		instance = ret.get();
		return ret;
	}
	std::vector<const char*> srcs;
	In::IFilePuller *instance = nullptr;
};

ReDashConfig createRDCfg() {
	ReDashConfig cfg;
	cfg.url = "http://url/for/the.mpd";
	cfg.segmentDurationInMs = 2000;
	cfg.baseUrlSub = "./";
	cfg.manifestFn = "redash.mpd";
	UtcStartTime utcStartTime;
	utcStartTime.startTime = 1789;
	cfg.utcStartTime = &utcStartTime;
	cfg.delayInSec = 0;
	cfg.timeshiftBufferDepthInSec = 17;
	return cfg;
}

void check(const std::string &moduleName, const std::string &manifest, const std::string &expected, ReDashConfig cfg = createRDCfg()) {
	FilePullerFactory filePullerFactory({ manifest.c_str() });
	cfg.filePullerFactory = &filePullerFactory;
	auto redash = loadModule(moduleName.c_str(), &NullHost, &cfg);
	auto recorder = createModule<Utils::Recorder>(&NullHost);
	ConnectOutputToInput(redash->getOutput(0), recorder->getInput(0));

	redash->process();

	Data data;
	auto ret = recorder->tryPop(data);
	ASSERT_EQUALS(true, ret);

	auto dataRaw = std::dynamic_pointer_cast<const DataRaw>(data);
	ASSERT(dataRaw);

	ret = recorder->tryPop(data);
	ASSERT_EQUALS(false, ret);

	auto meta = std::dynamic_pointer_cast<const MetadataFile>(data->getMetadata());
	ASSERT(meta);
	ASSERT_EQUALS(cfg.manifestFn, meta->filename);

	ASSERT_EQUALS(expected, std::string((const char*)data->data().ptr, data->data().len).c_str());
}

}
