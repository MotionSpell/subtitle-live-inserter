#include "mp4_mux_file_handler_dyn.hpp"
#include "lib_modules/utils/helper.hpp"
#include "lib_media/common/attributes.hpp" // PresentationTime
#include "lib_media/common/metadata_file.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_modules/utils/loader.hpp"
#include "lib_utils/format.hpp"
#include "lib_utils/log_sink.hpp"
#include "lib_utils/tools.hpp" // safe_cast
#include <cassert>

using namespace Modules;

namespace {

struct Mp4MuxFileHandlerDyn : ModuleS {
		Mp4MuxFileHandlerDyn(KHost *host, Mp4MuxFileHandlerDynConfig *cfg)
			: m_host(host), output(addOutput()),
			  segDurInMs(cfg->mp4MuxCfg->segmentDurationInMs), timeshiftBufferDepth(timescaleToClock(cfg->timeshiftBufferDepthInSec, 1)) {
			delegate = safe_cast<ModuleS>(loadModule("GPACMuxMP4", m_host, (void *)cfg->mp4MuxCfg));
			ConnectOutput(delegate->getOutput(0), [&](Data data) {
				auto out = std::make_shared<DataBaseRef>(data);
				out->copyAttributes(*data);
				auto meta = std::make_shared<MetadataFile>(*safe_cast<const MetadataFile>(data->getMetadata()));
				meta->filename = "s_0-";
				if (meta->durationIn180k == 0) {
					meta->filename += "init.mp4";
				} else {
					meta->filename += std::to_string(data->get<PresentationTime>().time / timescaleToClock(segDurInMs, 1000));
					meta->filename += ".m4s";
					timeshiftSegments.push_back({data->get<PresentationTime>().time, meta->filename});
				}
				assert(meta->EOS); //we don't support the muxer flush mem flag
				m_host->log(Info, format("Segment %s created.", meta->filename).c_str());
				out->setMetadata(meta);
				getOutput(0)->post(out);

				/*delete deprecated content*/
				auto seg = timeshiftSegments.begin();
				while (seg != timeshiftSegments.end()) {
					if (data->get<PresentationTime>().time - seg->pts > timeshiftBufferDepth) {
						// send 'DELETE' command
						auto out = output->allocData<DataRaw>(0);
						auto meta = make_shared<MetadataFile>(SUBTITLE_PKT);
						meta->filesize = INT64_MAX; // "DELETE"
						meta->filename = seg->filename;
						out->setMetadata(meta);
						output->post(out);

						seg = timeshiftSegments.erase(seg);
					} else
						break; /*segment are pushed in chronological order*/
				}
			});
		}
		void processOne(Data data) override {
			delegate->getInput(0)->push(data);
		}

	private:
		std::shared_ptr<ModuleS> delegate;
		KHost* const m_host;
		OutputDefault *const output;
		const uint64_t segDurInMs;
		const int64_t timeshiftBufferDepth;

		struct PendingSegment {
			int64_t pts;
			std::string filename;
		};
		std::vector<PendingSegment> timeshiftSegments;
};

IModule* createObject(KHost* host, void* va) {
	auto cfg = (Mp4MuxFileHandlerDynConfig*)va;
	enforce(host, "Mp4MuxFileHandlerDyn: host can't be NULL");
	enforce(cfg, "Mp4MuxFileHandlerDyn: config can't be NULL");
	return createModule<Mp4MuxFileHandlerDyn>(host, cfg).release();
}

auto const registered = Factory::registerModule("Mp4MuxFileHandlerDyn", &createObject);
}