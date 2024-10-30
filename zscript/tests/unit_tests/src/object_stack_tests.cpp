#include <ztests/ztests.h>
#include <zscript/zscript.h>
#include <zscript/core/object_stack.h>

#include <zbase/utility/print.h>
#include <zbase/container/stack.h>
#include <span>

TEST_CASE("zb::framed_stack") {
  zs::engine engine;
  zs::engine* eng = &engine;
  zb::execution_stack<zs::object, zs::allocator<zs::object>> fstack(32, zs::allocator<zs::object>(eng));

  fstack.push(55);
  fstack.push(56);

  REQUIRE(fstack.stack_size() == 2);

  REQUIRE(fstack[0] == 55);
  REQUIRE(fstack[1] == 56);

  fstack.push(57);

  REQUIRE(fstack.stack_size() == 3);

  REQUIRE(fstack[0] == 55);
  REQUIRE(fstack[1] == 56);
  REQUIRE(fstack[2] == 57);

  fstack.set_stack_base(1);
  REQUIRE(fstack.stack_size() == 2);

  REQUIRE(fstack[0] == 56);
  REQUIRE(fstack[1] == 57);

  //  REQUIRE(fstack[-3] == 55);
  REQUIRE(fstack[-2] == 56);
  REQUIRE(fstack[-1] == 57);

  {
    zb::stack_view<zs::object> sview(fstack);
    REQUIRE(sview.stack_size() == 2);
    REQUIRE(sview[0] == 56);
    REQUIRE(sview[1] == 57);

    REQUIRE(sview[-2] == 56);
    REQUIRE(sview[-1] == 57);
  }
  {
    zb::stack_view<zs::object> sview = fstack.get_stack_view();

    REQUIRE(sview.stack_size() == 2);
    REQUIRE(sview[0] == 56);
    REQUIRE(sview[1] == 57);

    REQUIRE(sview[-2] == 56);
    REQUIRE(sview[-1] == 57);
  }

  {
    zb::stack_view<zs::object> sview = fstack.get_stack_view();
    sview[0] = 2.2;

    REQUIRE(sview[0] == 2.2);
    REQUIRE(fstack[0] == 2.2);

    fstack.set_stack_base(0);
    REQUIRE(sview[0] == 2.2);
    REQUIRE(fstack[1] == 2.2);
    REQUIRE(fstack[0] == 55);
  }
}

TEST_CASE("zs::object_stack") {

  zs::engine engine;
  zs::engine* eng = &engine;

  zs::object_stack stack(eng, 32);
  REQUIRE(stack.stack_size() == 0);

  stack.push(55);
  stack.push(56);

  REQUIRE(stack.stack_size() == 2);

  REQUIRE(stack[1].is_integer());
}
