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

class ReDash : public Module {
	public:
		ReDash(KHost* host, const char* url)
			: m_host(host), url(url) {
			output = addOutput();
			auto meta = std::make_shared<MetadataPktSubtitle>();
			meta->timeScale = { IClock::Rate, 1 };
			output->setMetadata(meta);
	        m_host->activate(true);
		}

		void process() override {
		}

	private:
		KHost* const m_host;
        std::string url;
		OutputDefault* output;
};

IModule* createObject(KHost* host, void* va) {
	auto url = (const char*)va;
	enforce(host, "reDASH: host can't be NULL");
	enforce(url, "reDASH: url can't be NULL");
	return createModule<ReDash>(host, url).release();
}

auto const registered = Factory::registerModule("reDASH", &createObject);
}
