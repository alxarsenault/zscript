#include <catch2.h>
#include <zscript/base/container/vector.h>
#include <zscript/base/crypto/hash.h>

TEST_CASE("zb::vector") {
  zb::vector<int> vec = { 1, 2, 3, 4, 5 };
  const zb::vector<int>& cvec = vec;
  {
    auto it = vec.find(4);
    REQUIRE(it != vec.end());
    REQUIRE(*it == 4);
  }
  {
    auto it = cvec.find(4);
    REQUIRE(it != cvec.end());
    REQUIRE(*it == 4);
  }

  {
    auto it = vec.find(90);
    REQUIRE(it == vec.end());
  }

  {
    int* p = vec.pfind(4);
    REQUIRE(p);
    REQUIRE(*p == 4);
  }

  {
    const int* p = cvec.pfind(4);
    REQUIRE(p);
    REQUIRE(*p == 4);
  }

  std::span<int> sp = vec.subspan(1, 2);
  REQUIRE(sp.size() == 2);
  REQUIRE(sp[0] == 2);
  REQUIRE(sp[1] == 3);
}

TEST_CASE("zb::rapid_hash") {
  REQUIRE(zb::hash(std::string_view("bingo")) == 3709701525439390827);
  REQUIRE(zb::rapid_hash(std::string_view("bingo")) == 17783093858914975540UL);
  REQUIRE(zb::rapid_hash(std::string_view("bango")) == 16131472548179735328UL);
  REQUIRE(zb::rapid_hash(23) == 11821756975296769119UL);
  REQUIRE(zb::rapid_hash(24) == 16154279494620515966UL);
}
