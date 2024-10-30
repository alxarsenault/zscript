
#include <ztests/ztests.h>
#include <zscript/zscript.h>
#include <zbase/utility/print.h>
#include <zbase/container/byte.h>
#include <zbase/strings/parse_utils.h>
#include "lang/zparser.h"

// TEST_CASE("zs::parser") {
//   zs::engine eng;
//
//   zs::parser parser(&eng);
//
//   zs::object_ptr output;
//   zs::error_result err = parser.compile("var a = 32; a = 44;", "test", output
//   ); REQUIRE(!err);
//
//   zs::generic_ast_node_walker<zs::ast_node_type> walker(parser._root);
//
//   walker.buildDotFormat();
//
//   zb::print(walker.getDotFormat());
// }

// TEST_CASE("zs::parser") {
//   zs::engine eng;
//
//   zs::parser parser(&eng);
//
//   zs::object_ptr output;
//   zs::error_result err = parser.compile("var a = 32; var b = \"bingo\"; var c
//   = a + (b - 89) * (22 / 23); b = 66 + a;", "test",  output ); REQUIRE(!err);
//
//   zs::generic_ast_node_walker<zs::ast_node_type> walker(parser._root);
//
//   walker.buildDotFormat();
//
//   zb::print(walker.getDotFormat());
// }

TEST_CASE("zs::parser") {
  zs::engine eng;

  zs::parser parser(&eng);

  zs::object output;
  zs::error_result err = parser.parse(R"""(
var a = {
  k1 = 1,
  k2 = 23 + 24,
  k3 = "bacon"
};

var b = a.k1.p;
a.k1 = 22;
)""",
      "test", output);
  REQUIRE(!err);

  //  zb::print("SSISIIS", parser._stack.size());
  //  zs::ast_node_walker walker(parser.root());

  //  zs::string f = walker.serialize(&eng);
  //  zb::print(f);

  //  for(const auto& n : parser._stack) {
  //    zs::ast_node_walker walker(n);
  //
  //    zs::string f = walker.serialize(&eng);
  //    zb::print(f);
  //  }
}
