#include <ztests/ztests.h>
#include <zbase/strings/charconv.h>
#include <zbase/memory/memory.h>
 
TEST_CASE("charconv", "[core]") {
  char buffer[512] = {};
  {
    double value = 0;
    std::string_view s = "234.23 k";

    auto res = zb::from_chars(s.data(), s.data() + s.size(), value);
    REQUIRE(res);
    REQUIRE(res.value() == s.data() + 6);
    REQUIRE(value == 234.23);
  }
  {
    int value = 0;
    std::string_view s = "234 peter";

    auto res = zb::from_chars(s, value);
    REQUIRE(res);
    REQUIRE(res.value() == s.data() + 3);
    REQUIRE(value == 234);
  }

  {
    zb::mem_zero(buffer, 512);
    REQUIRE(zb::to_chars(buffer, 512, 78.78f));
    REQUIRE(std::string_view(buffer) == "78.78");
  }
  {

    zb::mem_zero(buffer, 512);
    REQUIRE(zb::to_chars(buffer, 512, 78.78));
    REQUIRE(std::string_view(buffer) == "78.78");
  }

  {

    zb::mem_zero(buffer, 512);
    REQUIRE(zb::to_chars(buffer, 512, 123));
    REQUIRE(std::string_view(buffer) == "123");
  }

  {
    int value = 0;
    REQUIRE(zb::from_chars("7896", value));
    REQUIRE(value == 7896);
  }

  {
    std::string s;
    REQUIRE(zb::to_chars(s, 32));
    REQUIRE(s == "32");
  }
}
