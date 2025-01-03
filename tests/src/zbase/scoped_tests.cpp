#include <catch2.h>
#include <zscript/base/utility/scoped.h>

TEST_CASE("scoped") {
  int count = 0;

  {
    zb::scoped s([&]() { count++; });
  }
  REQUIRE(count == 1);
}
