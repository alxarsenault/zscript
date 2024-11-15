
#include "ztests.h"

TEST_CASE("zs::weak_ref_01") {

  zs::engine eng;
  zs::object wref;

  {
    zs::object table2;

    {
      zs::object table = zs::var::create_table(&eng, { { zs::_ss("a"), 33 } });

      REQUIRE(table._table->get_map()[zs::_ss("a")] == 33);

      wref = table.get_weak_ref();
      REQUIRE(wref.is_weak_ref());
      //      REQUIRE(wref._weak_ref->get_type() ==
      //      zs::object_type::k_weak_ref);
      REQUIRE(wref._weak_ref->get_object().is_table());

      table2 = table;
      zs::object wref2 = table.get_weak_ref();
      zs::object wref3 = table2.get_weak_ref();

      zs::object wref3333 = wref.get_weak_ref();

      REQUIRE(wref3333.is_weak_ref());
      REQUIRE(wref3333.get_weak_ref_value().is_table());

      REQUIRE(wref2.is_weak_ref());
      REQUIRE(wref2.get_weak_ref_value().is_table());
      REQUIRE(wref3.get_weak_ref_value().is_table());

      zs::object tvalue = wref.get_weak_ref_value();
      REQUIRE(tvalue.is_table());

      REQUIRE(tvalue._table->get_map()[zs::_ss("a")] == 33);
    }
    REQUIRE(table2._table->get_map()[zs::_ss("a")] == 33);

    REQUIRE(wref.get_weak_ref_value().is_table());
  }

  REQUIRE(wref.get_weak_ref_value().is_null());
}
