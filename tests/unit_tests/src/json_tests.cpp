
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>

#include "json/zjson_parser.h"

TEST_CASE("zs::json") {

  zs::engine eng;

  {
    zs::json_parser parser(&eng);
    zs::object table = zs::_t(eng);
    table.get_table_internal_map()->emplace(zs::_ss("yes"), zs::_ss("Banana"));
    zs::object table2 = zs::_t(eng);
    table2.get_table_internal_map()->emplace(zs::_ss("bacon"), 87.21);

    table.get_table_internal_map()->emplace(zs::_ss("alex"), table2);

    zs::object output;
    REQUIRE(!parser.parse(nullptr, R"""(
    {
      "zsa":{},
      "b":"""bingo""",
      "c": [1, 2, 3],
      "d": {
          "k1": 78,
          "k2": {
              "a": 78.679,
              "b":"bingo",
              "c": ["a", "b", "c", [1, 9], yes],
              "peter": alex,
              "john": alex.bacon,
          }
      },
      "e": false,
      "g": true,
      "h": null
    })""",
        table, output));

    //    zb::print(output.convert_to_string());

    //    for(auto it : *output.get_table_internal_map()) {
    //      zb::print(it.first.to_debug_string(), it.second.to_debug_string());
    //    }
  }
}

TEST_CASE("zs::jsonk") {

  zs::engine eng;

  {
    zs::json_parser parser(&eng);
    zs::object output;
    REQUIRE(!parser.parse(nullptr, R"""(
    {
      "zsa":{},
      "b":"""bingo""",
      "c": [1, 2, 3],
      "d": {
          "k1": 78,
          "k2": {
              "a": 78.679,
              "b":"bingo",
              "c": ["a", "b", "c", [1, 9]]
          }
      },
      "e": false,
      "g": true,
      "h": null
    })""",
        nullptr, output));

    //    zb::print(output.convert_to_string());

    //    for(auto it : *output.get_table_internal_map()) {
    //      zb::print(it.first.to_debug_string(), it.second.to_debug_string());
    //    }
  }
}
