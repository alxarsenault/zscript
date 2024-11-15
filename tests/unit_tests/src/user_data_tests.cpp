
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

#define ZS_MEMORY_TEST_PRINT_OUTPUT 0

#if ZS_MEMORY_TEST_PRINT_OUTPUT
#define ZS_MEMORY_TEST_PRINT zb::print
#else
#define ZS_MEMORY_TEST_PRINT(...)
#endif // ZS_MEMORY_TEST_PRINT_OUTPUT.

namespace {
static std::unordered_map<void*, size_t> addr_map;

const zs::allocate_t test_allocate = [](zs::engine* eng, zs::raw_pointer_t user_ptr, void* ptr, size_t size,
                                         size_t old_size, zs::alloc_info_t info) -> void* {
  if (!size) {
    zbase_assert(ptr, "invalid pointer");
    addr_map.erase(ptr);
    ::free(ptr);
    ZS_MEMORY_TEST_PRINT("free", ptr);
    return nullptr;
  }

  if (ptr) {
    addr_map.erase(ptr);
    ptr = ::realloc(ptr, size);
    addr_map[ptr] = size;
    ZS_MEMORY_TEST_PRINT("realloc", ptr, size);
    return ptr;
  }

  zbase_assert(!ptr, "pointer should be nullptr");
  ptr = ::malloc(size);
  addr_map[ptr] = size;
  ZS_MEMORY_TEST_PRINT("malloc", ptr, size);
  return ptr;
};
} // namespace
struct my_struct {
  inline my_struct(const std::string& name)
      : _name(name) {}

  inline ~my_struct() {
    //    zb::print("~mystruct");
  }

  std::string get_bingo(zs::vm_ref vm, const std::string& s);

  std::string get_bingo2(const std::string& s) const;
  std::string get_bingo3() const;

  std::string _name;
};

TEST_CASE("zs::user_data") {
  {
    zs::engine eng(test_allocate);
    zs::object obj = zs::object::create_user_data<my_struct>(&eng, "123456789101112131415116171819");
    zs::object name_getter = zs::object::create_native_closure(&eng, [](zs::vm_ref vm) -> zs::int_t {
      zs::int_t nags = vm.stack_size();

      if (nags != 1) {
        zb::print("INVALID CALL");
        return -1;
      }

      zs::object& o = vm->stack_get(0);
      //      zb::print("name_getter", o.get_type(), o.convert_to_string());

      if (o.is_user_data()) {
        my_struct& ms = o._udata->data_ref<my_struct>();
        vm.push_string(ms._name);
      }
      else {
        vm.push_string("Banana");
      }

      return 1;
    });

    zs::object& tbl = obj._udata->get_delegate();
    tbl = zs::object::create_table(&eng);

    tbl._table->set(zs::_ss("A"), zs::_s(&eng, "bacon"));
    tbl._table->set(zs::_ss("B"), name_getter);

    REQUIRE(obj.is_user_data());
    REQUIRE(obj._udata->has_delegate());

    {
      my_struct& ms = obj._udata->data_ref<my_struct>();
      REQUIRE(ms._name == "123456789101112131415116171819");
    }

    {
      my_struct* ms = obj._udata->data<my_struct>();
      REQUIRE(ms->_name == "123456789101112131415116171819");
    }

    {

      static constexpr std::string_view code = R"""(
var a = k_mine.B();
return a;
)""";

      zs::vm vm(&eng);
      zs::object closure;

      if (auto err = vm->compile_buffer(code, "test", closure)) {
        REQUIRE(false);
        return;
      }

      REQUIRE(closure.is_closure());

      zs::object value;

      vm->get_root()._table->set(zs::_ss("k_mine"), obj);

      zs::int_t n_params = 1;
      vm.push_root();

      if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, value)) {

        REQUIRE(false);
        return;
      }

      //      zb::print(__func__, value.to_debug_string());
      //    REQUIRE(value.is_function());
      //    REQUIRE(value == "123456789101112131415116171819");

      // Call directly works.
      {
        zs::object ret_value;
        vm->push(obj);
        REQUIRE(!vm->call(name_getter, 1, vm.stack_size() - 1, ret_value));
        REQUIRE(ret_value == "123456789101112131415116171819");
      }
    }
  }

  ZS_MEMORY_TEST_PRINT("addr_map", addr_map);
}

std::string my_struct::get_bingo(zs::vm_ref vm, const std::string& s) { return s + "-" + _name; }

std::string my_struct::get_bingo2(const std::string& s) const { return s + "+" + _name; }
std::string my_struct::get_bingo3() const { return _name; }

TEST_CASE("zs::user_data2") {
  zs::engine eng;
  {

    zs::vm vm(&eng);
    zs::object obj = zs::object::create_user_data<my_struct>(&eng, "123456789101112131415116171819");
    obj.set_user_data_uid(zs::_s(eng, "banana-split"));

    zs::var uid;
    REQUIRE(!obj.get_user_data_uid(uid));
    REQUIRE(uid.is_string());
    REQUIRE(uid == "banana-split");
    REQUIRE(obj.has_user_data_uid(zs::_s(eng, "banana-split")));

    zs::object name_getter = zs::object::create_native_closure(&eng, [](zs::vm_ref vm) -> zs::int_t {
      zs::int_t nags = vm.stack_size();

      if (nags != 1) {
        zb::print("INVALID CALL");
        return -1;
      }

      zs::object& o = vm->stack_get(0);

      if (!o.is_user_data()) {
        return -1;
      }
      my_struct& ms = o._udata->data_ref<my_struct>();
      vm.push_string(ms._name);
      return 1;
    });

    zs::var fct = zs::var::create_native_closure_function(&eng, [](zs::vm_ref vm, zs::int_t p1) {
      zs::var& __this = vm[0];
      REQUIRE(__this.is_user_data());
      REQUIRE(__this.has_user_data_uid(zs::_s(vm, "banana-split")));
      REQUIRE(p1 == 55);
      return p1;
    });

    REQUIRE(obj.is_user_data());

    {

      zs::int_t np = vm.stack_size();
      vm.push(obj);

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(name_getter, n_params, vm.stack_size() - n_params, rval));

      REQUIRE(rval == "123456789101112131415116171819");
      vm->pop();
    }

    {
      zs::int_t np = vm.stack_size();
      vm.push(obj);
      vm.push_integer(55);

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(fct, n_params, vm.stack_size() - n_params, rval));

      REQUIRE(rval == 55);
    }

    {
      zs::var mfct
          = zs::var::create_native_closure_function(&eng, &my_struct::get_bingo, zs::_s(eng, "banana-split"));
      zs::int_t np = vm.stack_size();
      vm.push(obj);
      vm.push_string("yes");

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(mfct, n_params, vm.stack_size() - n_params, rval));
      REQUIRE(rval == "yes-123456789101112131415116171819");

      vm->pop(vm.stack_size());
    }
    {
      zs::var mfct = zs::var::create_native_closure_function(
          &eng, &my_struct::get_bingo2, zs::_s(eng, "banana-split"));
      zs::int_t np = vm.stack_size();
      vm.push(obj);
      vm.push_string("yes");

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(mfct, n_params, vm.stack_size() - n_params, rval));
      REQUIRE(rval == "yes+123456789101112131415116171819");

      vm->pop(vm.stack_size());
    }

    {
      zs::var mfct = zs::var::create_native_closure_function(
          &eng, &my_struct::get_bingo3, zs::_s(eng, "banana-split"));
      zs::int_t np = vm.stack_size();
      vm.push(obj);

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(mfct, n_params, vm.stack_size() - n_params, rval));
      REQUIRE(rval == "123456789101112131415116171819");

      vm->pop(vm.stack_size());
    }
  }
}

struct my_user_data_type {

  my_user_data_type() = default;

  inline my_user_data_type(const std::string& name)
      : _name(name) {}

  inline ~my_user_data_type() {
    //    zb::print("~mystruct");
  }

  std::string get_bingo(zs::vm_ref vm, const std::string& s);

  std::string get_bingo2(const std::string& s) const;
  std::string get_bingo3() const;

  std::string _name;
};

std::string my_user_data_type::get_bingo(zs::vm_ref vm, const std::string& s) { return s + "-" + _name; }
std::string my_user_data_type::get_bingo2(const std::string& s) const { return s + "+" + _name; }
std::string my_user_data_type::get_bingo3() const { return _name; }

TEST_CASE("zs::user_data3") {
  zs::engine eng;
  {
    zs::vm vm(&eng);
    zs::object obj = zs::object::create_user_data<my_user_data_type>(&eng, "123");
    obj.set_user_data_uid(zs::_s(eng, "banana-split"));

    zs::object obj2 = zs::object::create_user_data<my_user_data_type>(&eng, "bingo");
    obj2.set_user_data_uid(zs::_s(eng, "banana-split"));

    //    zs::var utypeid;
    //    REQUIRE(!obj.get_user_data_typeid(utypeid));

    {
      zs::var mfct = zs::var::create_native_closure_function(
          &eng, +[](zs::vm_ref vm, const my_user_data_type& my_type) {
            //            zb::print(my_type._name);
            REQUIRE(my_type._name == "123");
            return "bingo";
          });

      zs::int_t np = vm.stack_size();
      vm.push_root();
      vm.push(obj);

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(mfct, n_params, vm.stack_size() - n_params, rval));
      //      REQUIRE(rval == "yes-123");

      vm->pop(vm.stack_size());
    }

    {
      zs::var mfct = zs::var::create_native_closure_function(
          &eng, +[](zs::vm_ref vm, const my_user_data_type& my_type, const my_user_data_type& my_type2) {
            //            zb::print(my_type._name, my_type2._name);
            REQUIRE(my_type._name == "123");
            REQUIRE(my_type2._name == "bingo");
            return "bingo";
          });

      zs::int_t np = vm.stack_size();
      vm.push_root();
      vm.push(obj);
      vm.push(obj2);

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(mfct, n_params, vm.stack_size() - n_params, rval));
      //      REQUIRE(rval == "yes-123");

      vm->pop(vm.stack_size());
    }

    {
      zs::var mfct = zs::var::create_native_closure_function(
          &eng, &my_user_data_type::get_bingo, zs::_s(eng, "banana-split"));
      zs::int_t np = vm.stack_size();
      vm.push(obj);
      vm.push_string("yes");

      zs::int_t n_params = vm.stack_size() - np;
      zs::var rval;
      REQUIRE(!vm->call(mfct, n_params, vm.stack_size() - n_params, rval));
      REQUIRE(rval == "yes-123");

      vm->pop(vm.stack_size());
    }
  }
}
