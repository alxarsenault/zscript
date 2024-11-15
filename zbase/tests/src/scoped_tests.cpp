#include <catch2.h>
#include <zbase/utility/scoped.h>

TEST_CASE("scoped") {
  int count = 0;

  {
    zb::scoped s([&]() { count++; });
  }
  REQUIRE(count == 1);
}
