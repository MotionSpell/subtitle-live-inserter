#include "lib_modules/utils/helper.hpp"
#include "lib_media/common/metadata.hpp"
#include "lib_media/common/attributes.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/tools.hpp" //enforce
#include "lib_utils/log_sink.hpp"
#include "lib_utils/format.hpp"
#include <fstream>

extern const uint64_t g_segmentDurationInMs;

namespace {

using namespace Modules;

class SubtitleSource : public Module {
	public:
		SubtitleSource(KHost* host, const char* filename, const uint64_t segmentDurationInMs)
			: m_host(host), filename(filename), segmentDurationInMs(segmentDurationInMs) {
			output = addOutput();
			auto meta = std::make_shared<MetadataPktSubtitle>();
			meta->timeScale = { IClock::Rate, 1 };
			output->setMetadata(meta);
	        m_host->activate(true);
		}

		void process() override {
			std::ifstream file(filename);
			if (!file.is_open())
				throw error("Can't open subtitle source file");
            file.seekg(lastFilePos, std::ios::beg);

			std::string line;
			if (std::getline(file, line)) {
                m_host->log(Warning, format("Opening \"%s\"", line).c_str());

                std::ifstream ifs(line);
                if (!ifs.is_open())
                    return;

                auto pbuf = ifs.rdbuf();
                std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
                pbuf->pubseekpos (0,ifs.in);

                auto pkt = output->allocData<DataRaw>(size);
                pkt->set(CueFlags{});
                pkt->set(DecodingTime{ timescaleToClock(numSegment * (int64_t)segmentDurationInMs, 1000) });
                pkt->setMediaTime(numSegment * segmentDurationInMs, 1000);
                pbuf->sgetn((char*)pkt->buffer->data().ptr, size);
                output->post(pkt);

                numSegment++;
                ifs.close();

                lastFilePos = file.tellg();
                printf("Romain: %d\n", (int)lastFilePos);
			}
		}

	private:
		KHost* const m_host;
        std::string filename;
		OutputDefault* output;
        int numSegment = 0;
        const uint64_t segmentDurationInMs;
        std::streampos lastFilePos;
};

IModule* createObject(KHost* host, void* va) {
	auto filename = (const char*)va;
	enforce(host, "SubtitleSource: host can't be NULL");
	enforce(filename, "SubtitleSource: filename can't be NULL");
	return createModule<SubtitleSource>(host, filename, g_segmentDurationInMs).release();
}

auto const registered = Factory::registerModule("SubtitleSource", &createObject);
}
