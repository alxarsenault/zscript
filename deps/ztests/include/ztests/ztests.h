#pragma once

#include <iostream>
#include <filesystem>
#include <string>

#define CATCH_AMALGAMATED_CUSTOM_MAIN 1
#define CATCH_CONFIG_NO_RANDOM_SEED_PRINT 1
#define CATCH_CONFIG_NO_USE_ASYNC 1
// #define CATCH_CONFIG_DISABLE_EXCEPTIONS

#define REQUIRE_TRUE(...) REQUIRE(__VA_ARGS__)
#define REQUIRE_EQ(A, B) REQUIRE((A) == (B))
#define REQUIRE_NE(A, B) REQUIRE((A) != (B))
#define CHECK_EQ(A, B) CHECK((A) == (B))
#define CHECK_NE(A, B) CHECK((A) != (B))

#define REQUIRE_STREQ(A, B) REQUIRE(std::string_view(A) == std::string_view(B))

#include <ztests/catch2.h>





namespace ztests {
//struct EventListener : Catch::EventListenerBase
//{
//  using EventListenerBase::EventListenerBase;
//  
//  void testCaseStarting(const Catch::TestCaseInfo&  testInfo ) override {
//    std::cout << testInfo.name << std::endl;
//  }
//  
//};


}


#ifdef ZTESTS_MAIN
#include <zbase/sys/assert.h>

int main(int argc, char* argv[]) {
  zb::assert_handler.set([]() { return true; });
  return Catch::Session().run(argc, argv);
}
#endif // ZTESTS_MAIN.




