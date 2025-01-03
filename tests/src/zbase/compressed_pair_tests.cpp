#include <catch2.h>
#include <zscript/base/utility/compressed_pair.h>

TEST_CASE("compressed_pair") {
  zb::compressed_pair<int, float> a(2, 4.5);
  REQUIRE(a.first() == 2);
  REQUIRE(a.second() == 4.5);
  REQUIRE(sizeof(zb::compressed_pair<int, zb::empty_struct>) == sizeof(int));
  REQUIRE(sizeof(zb::compressed_pair<zb::empty_struct, int>) == sizeof(int));
}
