#define ZTESTS_MAIN
#include <ztests/ztests.h>
#include <zscript.h>

__attribute__((constructor)) static void print_zscript_version() { zb::print(zs::version()); }

namespace ztest {
std::string s_current_test_name = "john";
}

struct EventListener : Catch::EventListenerBase {
  using EventListenerBase::EventListenerBase;

  void testCaseStarting(const Catch::TestCaseInfo& testInfo) override {
    ztest::s_current_test_name = testInfo.name;
  }
};

CATCH_REGISTER_LISTENER(EventListener)
