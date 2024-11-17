
#include "unit_tests.h"

TEST_CASE("zs::closure_object_01") {

  zs::vm vm;

  auto a = zs::var::create_array(vm.get_engine(), { 1, 2, 3, 4, 5 });
  auto t = zs::var::create_table(vm.get_engine(), { { zs::_ss("a"), 1 }, { 32, 2 } });

  //  zb::print(a, t);
  const auto& b = a;
  REQUIRE(b.as_array()[0] == 1);

  REQUIRE(a.as_array()[2] == 3);

  char ds[] = { 'a', '\0' };
  const char* k = "a";
  REQUIRE(t._table->contains("a"));
  REQUIRE(t._table->contains(k));
  REQUIRE(t._table->contains(ds));
  REQUIRE(t._table->contains(std::string_view("a")));
  REQUIRE(t._table->contains(zs::_ss("a")));
  REQUIRE(t._table->contains(32));

  REQUIRE(t.as_table().get("a"));
  REQUIRE(*t.as_table().get("a") == 1);

  REQUIRE(t.as_table().erase("a"));
  REQUIRE(!t.as_table().contains("a"));

  //  REQUIRE(t.table().erase(32));

  REQUIRE(*t.as_table().get(32) == 2);
  REQUIRE(!t.as_table().get("a"));

  REQUIRE(!t.as_table().set("john", 23));
  REQUIRE(!t.as_table().set("john", 233));
  REQUIRE(t.as_table().contains("john"));
}
