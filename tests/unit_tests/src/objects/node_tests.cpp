
#include "ztests.h"

TEST_CASE("zs::node") {
  zs::engine eng;

  zs::object obj = zs::object::create_node(&eng, "John");
  REQUIRE(obj.is_node());

  zs::node_object& node = obj.as_node();
  REQUIRE(node.name() == "John");

  node.children().push_back(zs::object::create_node(&eng, "Peter"));
}
