#include <ztests/ztests.h>
#include  <zbase/strings/stack_string.h>
 
TEST_CASE("zb::stack_string") {
  zb::stack_string<32> str = "Alexandre";
  REQUIRE(str == "Alexandre");
  REQUIRE(str == std::string("Alexandre"));
  REQUIRE(str == std::string_view("Alexandre"));
}
 
