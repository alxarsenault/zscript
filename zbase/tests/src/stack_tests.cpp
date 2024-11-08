#include <ztests/ztests.h>
#include <zbase/container/stack.h>

TEST_CASE("zbase.stack.01") {
  zb::execution_stack<int> fstack(32);
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
    zb::stack_view<int> sview(fstack);
    REQUIRE(sview.stack_size() == 2);
    REQUIRE(sview[0] == 56);
    REQUIRE(sview[1] == 57);

    REQUIRE(sview[-2] == 56);
    REQUIRE(sview[-1] == 57);
  }
  {
    zb::stack_view<int> sview = fstack.get_stack_view();

    REQUIRE(sview.stack_size() == 2);
    REQUIRE(sview[0] == 56);
    REQUIRE(sview[1] == 57);

    REQUIRE(sview[-2] == 56);
    REQUIRE(sview[-1] == 57);
  }

  {
    zb::stack_view<int> sview = fstack.get_stack_view();
    sview[0] = 100;

    REQUIRE(sview[0] == 100);
    REQUIRE(fstack[0] == 100);

    fstack.set_stack_base(0);
    REQUIRE(sview[0] == 100);
    REQUIRE(fstack[1] == 100);
    REQUIRE(fstack[0] == 55);
  }
}

TEST_CASE("zb::execution_stack<string>") {
  zb::execution_stack<std::string> fstack(10);
  fstack.push("A");
  fstack.push("B");

  std::string b = fstack.pop_get();
  REQUIRE(b == "B");
}
