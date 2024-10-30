#include "ztests.h"
#include "lang/ztoken.h"
#include "lang/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

static inline void get_01(zs::virtual_machine& vm) {

  zs::object& root = vm.get_root();
  zs::table_object* root_tbl = root._table;

  root_tbl->set(zs::_ss("check"), zs::_nc(vm, [](zs::vm_ref vm) -> zs::int_t {
    //    const zs::int_t count = vm.top();

    zs::bool_t value = false;
    if (vm->stack_get(1).convert_to_bool(value) || !value) {
      vm.push_bool(false);

      zb::print("FAILED");
      return 1;
    }

    vm.push_bool(true);
    return 1;
  }));
}

ZS_CODE_TEST("delegates::get_01", R"""(
  var my_table = { k1 = 56};

  var my_delegate = {
    a = 8,
    b = 78,
     

    

    operator(typeof) = "my_table"
  };

  // Set 'my_delegate' as delegate.
  set_delegate(my_table, my_delegate);

  check(true);

  my_table.a = 33;

  // Should crash?
  my_table.d = 54;
  var t = {
    k1 = my_table.a,
    k2 = my_table.b,
    k3 = my_table.k1,
    //k4 = my_table.d,

    k4 = typeof(my_table)
  };

  return t;
)""",
    get_01) {
  REQUIRE(vm.top().is_table());

  zs::table_object* tbl = vm.top()._table;

  //  print_s

  //  zb::print(tbl->get_map()[zs::_ss("k4")].to_debug_string());
  REQUIRE(tbl->get_map()[zs::_ss("k1")] == 33);
  REQUIRE(tbl->get_map()[zs::_ss("k4")] == "my_table");
}

#if ZS_TEST_PRINT_METHODS
ZS_CODE_TEST("delegates::get_04", R"""(
// Should this be allowed?
::ppp = 90;

var my_table = {
  k1 = 56,

  fct = function() {
    print("Abc");
    //::print("Abc");
  }
};

print("A", my_table, my_table.k1);

my_table.fct();
return my_table;
)""") {
  zb::print(vm.get_root().convert_to_string());
}
#endif // ZS_TEST_PRINT_METHODS
