#include "lib_modules/utils/helper.hpp"
#include "lib_modules/utils/factory.hpp"
#include "lib_utils/tools.hpp" //enforce

using namespace Modules;

namespace {

class HlsWebvttRephaser : public Module {
	public:
		HlsWebvttRephaser(KHost* /*host*//*, ReDashConfig *cfg*/) {}

		void process() override {
		}
};

IModule* createObject(KHost* host, void* /*va*/) {
	//auto cfg = (ReDashConfig*)va;
	enforce(host, "HlsWebvttRephaser: host can't be NULL");
	//enforce(cfg, "HlsWebvttRephaser: config can't be NULL");
	return createModule<HlsWebvttRephaser>(host/*, cfg*/).release();
}

auto const registered = Factory::registerModule("HlsWebvttRephaser", &createObject);

}