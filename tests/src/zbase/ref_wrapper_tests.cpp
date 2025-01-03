#include <catch2.h>
#include <zscript/base/memory/ref_wrapper.h>

namespace {

TEST_CASE("zbase.ref_wrapper.01") {
  std::string a = "A";
  std::string b = "B";
  std::reference_wrapper<std::string> rw(a);
  rw = b;

  REQUIRE(a == "A");
  REQUIRE(b == "B");
  REQUIRE(rw.get() == b);

  rw.get() = "C";
  REQUIRE(b == "C");
}

TEST_CASE("zbase.ref_wrapper.02") {
  std::string a = "A";
  std::string b = "B";

  REQUIRE(a == "A");
  REQUIRE(b == "B");

  zb::ref_wrapper<std::string> rwa(a);
  zb::ref_wrapper<std::string> rwb(b);

  rwa = rwb;

  rwa.get() = "C";
  REQUIRE(a == "A");
  REQUIRE(b == "C");
}

TEST_CASE("zbase.ref_wrapper.03") {
  std::string a = "ref_wrapper";
  std::string b = "B";
  auto ref = zb::wref(a);
  auto cref = zb::wcref(a);
  std::reference_wrapper<std::string> w(a);
  std::reference_wrapper<std::string> w2(a);

  w2 = b;
  //  REQUIRE(w == w2);
}
} // namespace.
