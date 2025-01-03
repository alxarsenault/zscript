#include <catch2.h>
#include <zscript/base/strings/stack_string.h>

TEST_CASE("zb::stack_string") {
  zb::stack_string<32> str = "Alexandre";
  REQUIRE(str == "Alexandre");
  REQUIRE(str == std::string("Alexandre"));
  REQUIRE(str == std::string_view("Alexandre"));
}
