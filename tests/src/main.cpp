// #define ZTESTS_MAIN
#include "unit_tests.h"
#include <zscript/zscript.h>
#include <zbase/sys/assert.h>
#include <uv.h>

__attribute__((constructor)) static void print_zscript_version() { zb::print(zs::version()); }

namespace utest {
std::string s_current_test_name = "john";
} // namespace utest.

struct EventListener : Catch::EventListenerBase {
  using EventListenerBase::EventListenerBase;

  void testCaseStarting(const Catch::TestCaseInfo& testInfo) override {
    utest::s_current_test_name = testInfo.name;
  }
};

CATCH_REGISTER_LISTENER(EventListener)

int main(int argc, char* argv[]) {

  //  zb::print("meta_method", zb::enum_count<zs::meta_method>());
  //  zb::print("object_type", zb::enum_count<zs::object_type>());
  //  zb::print("exposed_object_type", zb::enum_count<zs::exposed_object_type>());
  //  zb::print("extension_type", zb::enum_count<zs::extension_type>());

  uv_setup_args(argc, argv);
  zb::assert_handler.set([]() { return true; });
  return Catch::Session().run(argc, argv);
}
