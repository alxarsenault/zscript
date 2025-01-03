#include "unit_tests.h"
#include "utility/zvm_call.h"
#include <zscript/utility/object_function_wrapper.h>

using namespace utest;

UTEST_CASE("object_function_wrapper") {
  zs::vm vm;
  zs::engine* eng = vm.get_engine();

  static constexpr zs::int_t p1 = 55;
  static constexpr zs::float_t p2 = 89.56;
  static constexpr std::string_view p3 = "john";

  zs::var func
      = zs::object_function_wrapper::create(eng, [](zs::vm_ref vm, zs::int_t p, float k, std::string_view s) {
          REQUIRE(p == p1);
          REQUIRE(k == (float)p2);
          REQUIRE(s == p3);
          return s;
        });

  REQUIRE(func.as_native_closure().get_closure_type() == zs::native_closure_object::closure_type::obj);

  // virtual_machine::call(const object& closure, std::initializer_list<const object> params,
  //                       object& ret_value)
  {
    zs::var rval;
    REQUIRE(!vm->call(func, { vm->global(), p1, p2, zs::_ss(p3) }, rval));
    REQUIRE(rval == p3);
    REQUIRE(vm.stack_size() == 0);
  }

  // virtual_machine::call(const object& closure, int_t n_params, int_t stack_base, object& ret_value,
  //                       bool stack_base_relative = true);
  {
    zs::int_t params_base = vm.stack_size();
    vm.push_global();
    vm.push_integer(p1);
    vm.push_float(p2);
    vm.push_string(p3);
    zs::int_t n_params = vm.stack_size() - params_base;
    REQUIRE(n_params == 4);

    zs::var rval;
    REQUIRE(!vm->call(func, n_params, vm.stack_size() - n_params, rval));
    vm->pop(4);
    REQUIRE(vm.stack_size() == 0);
    REQUIRE(rval == p3);
  }

  // virtual_machine::call_from_top(const object& closure, int_t n_params, object& ret_value);
  {
    zs::int_t params_base = vm.stack_size();
    vm.push_global();
    vm.push_integer(p1);
    vm.push_float(p2);
    vm.push_string(p3);
    zs::int_t n_params = vm.stack_size() - params_base;

    REQUIRE(n_params == 4);

    zs::var rval;
    REQUIRE(!zs::call_from_top(vm, func, n_params, rval));
    vm->pop(4);
    REQUIRE(vm.stack_size() == 0);
    REQUIRE(rval == p3);
  }
}

UTEST_CASE("object_function_wrapper") {
  zs::vm vm;
  zs::engine* eng = vm.get_engine();

  {
    int captured_value = 3333;
    zs::var fct
        = zs::object_function_wrapper::create(eng, [&](zs::vm_ref vm, zs::int_t val, const std::string& s) {
            REQUIRE(val == 123);
            REQUIRE(s == "bacon");
            return 32 + captured_value;
          });

    zs::var rval;
    REQUIRE(!vm->call(fct, { vm->global(), 123, zs::_ss("bacon") }, rval));
    REQUIRE(rval == captured_value + 32);
  }

  {
    int captured_value = 3333;
    zs::var fct = zs::object_function_wrapper::create(eng,
        std::function<zs::int_t(zs::int_t, const std::string&)>([&](zs::int_t val, const std::string& s) {
          REQUIRE(val == 123);
          REQUIRE(s == "bacon");
          return 32 + captured_value;
        }));

    zs::var rval;
    REQUIRE(!vm->call(fct, { vm->global(), 123, zs::_ss("bacon") }, rval));
    REQUIRE(rval == captured_value + 32);
  }

  {
    zs::var fct = zs::object_function_wrapper::create(
        eng, [](const std::vector<std::string>& vec) -> std::vector<std::string> {
          std::vector<std::string> vec_out = vec;
          vec_out.push_back("D");
          vec_out.push_back("E");
          return vec_out;
        });

    zs::var rval;
    REQUIRE(
        !vm->call(fct, { vm->global(), zs::_a(eng, { zs::_ss("A"), zs::_ss("B"), zs::_ss("C") }) }, rval));
    REQUIRE(rval.is_array());
    REQUIRE(rval == zs::_a(eng, { zs::_ss("A"), zs::_ss("B"), zs::_ss("C"), zs::_ss("D"), zs::_ss("E") }));
  }

  //

  REQUIRE(vm.stack_size() == 0);
}

UTEST_CASE("KKLKKK") {

  zs::engine eng;
  {

    {

      {
        zs::vm vm(&eng);

        int ksd = 3333;
        zs::var fct = zs::object_function_wrapper::create(
            &eng, [&](zs::vm_ref vm, zs::int_t val, const std::string& s) {
              REQUIRE(val == 123);
              REQUIRE(s == "bacon");
              return 32 + ksd;
            });

        vm.push_global();
        vm.push_integer(123);
        vm.push_string("bacon");

        zs::var rval;
        REQUIRE(!vm->call(fct, 3, vm.stack_size() - 3, rval));

        REQUIRE(rval == ksd + 32);
      }

      {
        zs::vm vm(&eng);

        int ksd = 3333;

        zs::var fct = zs::object_function_wrapper::create(
            &eng, [&](zs::vm_ref vm, zs::int_t val, const std::string& s) {
              REQUIRE(val == 123);
              REQUIRE(s == "bacon");
              return 32 + ksd;
            });

        vm.push_global();
        vm.push_integer(123);
        vm.push_string("bacon");

        zs::var rval;
        REQUIRE(!vm->call(fct, 3, vm.stack_size() - 3, rval));

        REQUIRE(rval == ksd + 32);
      }

      {
        zs::vm vm(&eng);

        int ksd = 3333;

        zs::var fct = zs::object_function_wrapper::create(&eng,
            std::function<zs::int_t(zs::int_t, const std::string&)>([=](zs::int_t val, const std::string& s) {
              REQUIRE(val == 123);
              REQUIRE(s == "bacon");
              return 32 + ksd;
            }));

        vm.push_global();
        vm.push_integer(123);
        vm.push_string("bacon");

        {
          REQUIRE(vm[-1].is_string());
          zs::string s((zs::allocator<char>(&eng)));
          REQUIRE(!vm[-1].convert_to_string(s));
          REQUIRE(s == "bacon");
        }

        {
          REQUIRE(vm[-1].is_string());
          std::string s;
          REQUIRE(!vm[-1].convert_to_string(s));
          REQUIRE(s == "bacon");
        }

        zs::var rval;
        REQUIRE(!vm->call(fct, 3, vm.stack_size() - 3, rval));

        REQUIRE(rval == ksd + 32);
      }
    }

    zs::var a = zs::object_function_wrapper::create(
        &eng, +[](zs::vm_ref vm, zs::int_t val, zs::float_t fval, const zs::var& obj) {
          //          zb::print(__FUNCTION__, val, fval, obj.get_type());
          return 89;
        });

    {
      zs::vm vm(&eng);
      vm.push_global();
      vm.push_integer(33);
      vm.push_float(12.12);

      zs::var obj = zs::var::create_array(&eng, 5);
      vm.push(obj);

      zs::var rval;
      REQUIRE(!vm->call(a, 4, vm.stack_size() - 4, rval));

      REQUIRE(rval == 89);
    }

    {
      zs::vm vm(&eng);

      zs::var b = zs::object_function_wrapper::create(
          &eng, +[](zs::vm_ref vm) {
            //            zb::print(__FUNCTION__, "B");
            return 11;
          });

      vm.push_global();

      zs::var rval;
      REQUIRE(!vm->call(b, 1, vm.stack_size() - 1, rval));

      REQUIRE(rval == 11);
    }

    {
      zs::vm vm(&eng);

      zs::var c = zs::object_function_wrapper::create(
          &eng, +[]() {
            //            zb::print(__FUNCTION__, "C");
            return 11;
          });

      vm.push_global();

      zs::var rval;
      REQUIRE(!vm->call(c, 1, vm.stack_size() - 1, rval));

      REQUIRE(rval == 11);
    }

    {
      zs::vm vm(&eng);

      zs::var d = zs::object_function_wrapper::create(
          &eng,
          +[](zs::vm_ref vm, zs::int_t p1, std::vector<std::string> vec, std::string s,
               std::map<std::string, zs::var> m, std::map<std::string, zs::var> m2) {
            //            zb::print("D", "p1", p1, "\nvec.size", vec.size(),
            //            "\nVEC", vec, "\nS", s, "\nmap", m, m2);

            return p1;
          });

      zs::int_t np = vm.stack_size();
      vm.push_global();
      vm.push_integer(55);
      zs::var obj = zs::_a(&eng, 3);
      auto& vec = *obj.get_array_internal_vector();
      vec[0] = 88;
      vec[1] = 89.21;
      vec[2] = zs::_ss("kl");
      vm.push(obj);

      vm.push_string("john");
      //      vm.push_integer(55);

      zs::var tbl = zs::_o(&eng, //
          std::initializer_list<std::pair<zs::object, zs::object>>{
              { zs::_ss("john2"), 89 }, //
              { zs::_ss("john9"), 989 }, //
              { zs::_ss("joh"), zs::_ss("pljkl") }, //
              { zs::_ss("A"), obj } //
          });

      ;

      std::array<char, 3> arr = { 1, 2, 3 };

      zs::var tbl2 = zs::var(&eng, //
          std::map<std::string, zs::var>{
              { "john", 21 }, //
              { "johnll", zs::_a(&eng, arr) }, //
              { "peter", obj } //
          });
      //
      //      zb::print("--------------------------",tbl);
      //      zb::print("--------------------------",tbl2);

      vm.push(tbl);
      vm.push(tbl2);
      //      vm.push_integer(11);
      zs::int_t n_params = vm.stack_size() - np;
      //      zb::print("LLLLL", n_params);
      zs::var rval;
      REQUIRE(!vm->call(d, n_params, vm.stack_size() - n_params, rval));

      //      zb::print(rval.to_debug_string());
      REQUIRE(rval == 55);
    }
  }
}
