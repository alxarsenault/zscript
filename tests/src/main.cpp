#include "unit_tests.h"
#include <zscript/base/sys/assert.h>

__attribute__((constructor)) static void print_zscript_version() { zb::print(zs::version()); }

namespace utest {
std::string s_current_test_name = "unknown";
} // namespace utest.

struct EventListener : Catch::EventListenerBase {
  using EventListenerBase::EventListenerBase;

  void testCaseStarting(const Catch::TestCaseInfo& testInfo) override {
    utest::s_current_test_name = testInfo.name;
  }
};

CATCH_REGISTER_LISTENER(EventListener)

int main(int argc, char* argv[]) {
  zb::assert_handler.set([]() { return true; });
  return Catch::Session().run(argc, argv);
}
