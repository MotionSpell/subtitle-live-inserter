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
	MemoryFileSystem(const char *src) : src(src) {}
	void wget(const char* /*szUrl*/, std::function<void(SpanC)> callback) override {
		ASSERT(src != nullptr);
		callback({(const uint8_t*)src, strlen(src)});
	}
	void askToExit() override {}
	const char *src = nullptr;
};

struct FilePullerFactory : In::IFilePullerFactory {
	FilePullerFactory(const char *src) : src(src) {}
	std::unique_ptr<In::IFilePuller> create() override {
		return std::make_unique<MemoryFileSystem>(src);
	}
	const char *src = nullptr;
};

ReDashConfig createRDCfg() {
	ReDashConfig cfg;
	cfg.url = "http://url/for/the.mpd";
	cfg.segmentDurationInMs = 2000;
	cfg.baseUrlSub = ".";
	cfg.manifestFn = "redash.mpd";
	cfg.postUrl = "/root/output/";
	UtcStartTime utcStartTime;
	utcStartTime.startTime = 1789;
	cfg.utcStartTime = &utcStartTime;
	cfg.delayInSec = 0;
	cfg.timeshiftBufferDepthInSec = 17;
	return cfg;
}

void check(const std::string &moduleName, const std::string &manifest, const std::string &expected, ReDashConfig cfg = createRDCfg()) {
	FilePullerFactory filePullerFactory(manifest.c_str());
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

	ASSERT_EQUALS(cfg.manifestFn, meta->filename.substr(meta->filename.size() - cfg.manifestFn.size(), meta->filename.size()));

	ASSERT_EQUALS(expected, std::string((const char*)data->data().ptr, data->data().len).c_str());
}

}
