#include <catch2.h>
#include <zbase/strings/string_literal.h>

TEST_CASE("zb::string_literal") {

  {
    constexpr auto a = zb::string_literal("a");
    constexpr auto b = zb::string_literal("b");
    constexpr auto c = zb::string_literal("c");
    constexpr auto abc = zb::string_literal_concat<a, b, c>();
    REQUIRE(abc == "abc");
  }

  {
    constexpr auto a = zb::string_literal("a");
    constexpr auto b = zb::string_literal("b");
    constexpr auto c = zb::string_literal("c");
    constexpr auto abc = zb::string_literal_join<"-", a, b, c>();
    REQUIRE(abc == "a-b-c");
  }

  constexpr auto k = zb::string_literal("abc");
  //  zb::print("ldsds", k);

  constexpr auto k1 = k.substr<0, 1>();
  REQUIRE(k1.view() == "a");

  constexpr auto k2 = zb::string_literal_concat<k, k1>();
  REQUIRE(k2.view() == "abca");

  constexpr auto k3 = zb::string_literal_join<"-", k, k1>();
  REQUIRE(k3.view() == "abc-a");

  REQUIRE(k3 == "abc-a");
}
