#include "ztests.h"
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

ZS_FILE_TEST("doc/doc_01.zs") {

  REQUIRE(vm.top().is_table());

  zs::object_unordered_map<zs::object>& map = *vm.top().get_table_internal_map();

  REQUIRE(map["a"].is_null());
  REQUIRE(map["b"].is_none());
  REQUIRE(map["c"].is_bool());
  REQUIRE(map["d"].is_bool());
  REQUIRE(map["e"].is_integer());
  REQUIRE(map["f"].is_float());
  REQUIRE(map["g"].is_integer());

  REQUIRE(map["h"].is_string());
  REQUIRE(map["i"].is_string());
  REQUIRE(map["j"].is_string());
  REQUIRE(map["k"].is_array());
  REQUIRE(map["l"].is_array());
  REQUIRE(map["m"].is_table());

  REQUIRE(map["c"] == false);
  REQUIRE(map["d"] == true);
  REQUIRE(map["e"] == 5);
  REQUIRE(map["f"] == 5.5);
  REQUIRE(map["g"] == u'π');
  REQUIRE(map["h"] == "Banana");
  REQUIRE(map["i"] == "Abc");
  REQUIRE(map["j"] == R"(
           Jombo Banana
        )");
}
