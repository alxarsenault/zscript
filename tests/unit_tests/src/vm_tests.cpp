
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>

TEST_CASE("zs::vm") {

  //  zs::object obj;
  //  obj.
  zs::virtual_machine* m = zs::create_virtual_machine(1024);
  zs::close_virtual_machine(m);

  zs::vm vm;

  bool did_call = false;

  REQUIRE(!vm.new_closure([&](zs::vm_ref v) {
    did_call = true;
    return 3;
  }));
}
