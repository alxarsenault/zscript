#pragma once

#include <ztests/ztests.h>
#include <zscript/zscript.h>

#include "lang/ztoken.h"
#include "lang/zlexer.h"
#include "lang/jit/zjit_compiler.h"
#include "object/zfunction_prototype.h"

#include "zvirtual_machine.h"

#include <zbase/sys/file_view.h>
#include <zbase/utility/print.h>
#include <fstream>

#define ZS_TEST_PRINT_METHODS 0

#define ZSTD_PATH(name) ZSCRIPT_STANDARD_TESTS_RESOURCES_DIRECTORY "/" name

#define ZS_FILE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(TestName, ClassName, filename, ...)                 \
  CATCH_INTERNAL_START_WARNINGS_SUPPRESSION                                                               \
  CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS                                                                \
  CATCH_INTERNAL_SUPPRESS_UNUSED_VARIABLE_WARNINGS                                                        \
  namespace {                                                                                             \
  struct TestName : ClassName {                                                                           \
    inline TestName()                                                                                     \
        : ClassName(ZSTD_PATH(filename)) {}                                                               \
    void test();                                                                                          \
  };                                                                                                      \
  const Catch::AutoReg INTERNAL_CATCH_UNIQUE_NAME(autoRegistrar)(Catch::makeTestInvoker(&TestName::test), \
      CATCH_INTERNAL_LINEINFO, #ClassName##_catch_sr, Catch::NameAndTags{ __VA_ARGS__ });                 \
  }                                                                                                       \
  CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION                                                                \
  void TestName::test()

#define ZS_FILE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD(filename, className, ...) \
  ZS_FILE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(                               \
      INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), className, filename, __VA_ARGS__)

#define ZS_FILE_TEST_1(filename, ...)           \
  ZS_FILE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD( \
      filename, ztest::z_file_test_fixture, filename, /*"[.]",*/ __VA_ARGS__)

#define ZS_FILE_TEST_2(filename, fct, ...)      \
  ZS_FILE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD( \
      filename, ztest::z_file_test_fixture_t<fct>, filename, __VA_ARGS__)

//
//
//

#define ZS_CODE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(TestName, ClassName, code, tname, lamda, ...)       \
  CATCH_INTERNAL_START_WARNINGS_SUPPRESSION                                                               \
  CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS                                                                \
  CATCH_INTERNAL_SUPPRESS_UNUSED_VARIABLE_WARNINGS                                                        \
  namespace {                                                                                             \
  struct TestName : ClassName {                                                                           \
    inline TestName()                                                                                     \
        : ClassName(code, tname, lamda) {}                                                                \
    void test();                                                                                          \
  };                                                                                                      \
  const Catch::AutoReg INTERNAL_CATCH_UNIQUE_NAME(autoRegistrar)(Catch::makeTestInvoker(&TestName::test), \
      CATCH_INTERNAL_LINEINFO, #ClassName##_catch_sr, Catch::NameAndTags{ tname });                       \
  }                                                                                                       \
  CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION                                                                \
  void TestName::test()

#define ZS_CODE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD(code, tname, lamda, ...)                      \
  ZS_CODE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), \
      ztest::z_code_test_fixture, code, tname, lamda, __VA_ARGS__)

#define ZS_CODE_TEST_2(tname, code, ...) \
  ZS_CODE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD(code, tname, ztest::empty_init_function, __VA_ARGS__)

#define ZS_CODE_TEST_3(tname, code, fct, ...) \
  ZS_CODE_TEST_INTERNAL_CATCH_TEST_CASE_METHOD(code, tname, fct, __VA_ARGS__)

#define ZS_FILE_TEST(...) ZBASE_DEFER(ZBASE_CONCAT(ZS_FILE_TEST_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)
#define ZS_CODE_TEST(...) ZBASE_DEFER(ZBASE_CONCAT(ZS_CODE_TEST_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

namespace ztest {

inline void empty_init_function(zs::virtual_machine& vm) {}

inline static void include_test_lib(zs::virtual_machine& vm) {
  zs::object test_lib = zs::_t(vm.get_engine());
  test_lib.as_table()["vals"] = zs::_t(vm.get_engine());

  test_lib.as_table()[zs::_s(vm.get_engine(), "check")]
      = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
          //        zb::print(vm->get_call_stack(), vm->stack().top());

          bool valid = vm[1].is_if_true();
          if (vm.stack_size() >= 3) {
            INFO(vm[2].get_string_unchecked());
            CHECK(valid);
          }
          else {
            CHECK(valid);
          }

          vm.push(valid);
          return 1;
        });

  test_lib.as_table()[zs::_s(vm.get_engine(), "check_eq")]
      = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
          //        zb::print(vm->get_call_stack(), vm->stack().top());

          bool valid = vm[1] == vm[2];
          if (vm.stack_size() > 3) {
            INFO(vm[3].get_string_unchecked());
            CHECK(valid);
          }
          else {
            CHECK(valid);
          }

          vm.push(valid);
          return 1;
        });

  test_lib.as_table()[zs::_s(vm.get_engine(), "info")]
      = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
          UNSCOPED_INFO(vm[1].get_string_unchecked());
          return 0;
        });

  zs::table_object* tbl = vm.get_imported_module_cache()._table;
  tbl->set(zs::_ss("utest"), test_lib);
}

// inline zs::object get_utest_vals(zs::engine* eng) {
//   return eng->get_imported_module_cache().as_table()["utest"].as_table()["vals"];
// }

struct z_file_test_fixture {
  inline z_file_test_fixture(const char* filepath)
      //      : vmachine(zs::create_virtual_machine(512))
      : vm(512)
      , filepath(filepath) {

    vm.get_engine()->add_import_directory(ZSCRIPT_STANDARD_TESTS_RESOURCES_DIRECTORY);

    if (auto err = vm->compile_file(filepath, "test", closure)) {
      error = err;
      zb::print(vm.get_error(), error, __FILE__, __LINE__, filepath);
      return;
    }

    REQUIRE(closure.is_closure());

    zs::int_t n_params = 1;
    vm.push_root();

    if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, ret_value)) {
      error = err;
      zb::print(vm.get_error(), error, __FILE__, __LINE__);
      REQUIRE(false); //("kk");
    }
    //    REQUIRE(!v);

    vm.push(ret_value);
  }

  inline ~z_file_test_fixture() {
    //    closure.reset();
    //    zs::close_virtual_machine(vmachine);
  }

  //  zs::virtual_machine_unique_ptr vmachine;
  //  zs::virtual_machine& vm;
  zs::vm vm;
  zs::object closure;
  zs::object ret_value;
  std::string filepath;
  zb::file_view file;
  zs::error_result error;
};

template <auto Fct>
struct z_file_test_fixture_t {
  inline z_file_test_fixture_t(const char* filepath)
      : vm(512)
      , filepath(filepath) {

    vm.get_engine()->add_import_directory(ZSCRIPT_STANDARD_TESTS_RESOURCES_DIRECTORY);

    Fct(vm);
    if (auto err = vm->compile_file(filepath, "test", closure)) {
      error = err;
      return;
    }

    REQUIRE(closure.is_closure());

    zs::int_t n_params = 1;
    vm.push_root();

    REQUIRE(!vm->call(closure, n_params, vm.stack_size() - n_params, ret_value));

    vm.push(ret_value);
  }

  inline ~z_file_test_fixture_t() {
    //    closure.reset();
    //    zs::close_virtual_machine(vmachine);
  }

  //  zs::virtual_machine_unique_ptr vmachine;
  //  zs::virtual_machine& vm;
  zs::vm vm;
  zs::object closure;
  zs::object ret_value;
  std::string filepath;
  zb::file_view file;
  zs::error_result error;
};

struct z_code_test_fixture {

  template <class Lamda>
  inline z_code_test_fixture(
      std::string_view ccode, std::string_view tname = "", Lamda&& fct = empty_init_function)
      : vm(512)
      , eng(vm.get_engine())
      , code(ccode) {

    eng->add_import_directory(ZSCRIPT_STANDARD_TESTS_RESOURCES_DIRECTORY);

    include_test_lib(vm);

    fct(vm);

    if (auto err = vm->compile_buffer(code, "test", closure)) {
      error = err;
      zb::print(vm.get_error());
      return;
    }

    REQUIRE(closure.is_closure());

    // Print debug info.
    //     closure._closure->get_function_prototype()->debug_print();

    zs::int_t n_params = 1;
    vm.push_root();

    if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, value)) {
      error = err;
      zb::print(std::source_location::current(), error, vm.get_error());
      REQUIRE(false);
    }

    vm.push(value);
  }

  inline ~z_code_test_fixture() {
    //        closure.reset();
    //    value.reset();
    //    zs::close_virtual_machine(vmachine);
  }
  //  inline zs::table_object& vals() { return get_utest_vals(vm.get_engine()).as_table(); }

  //  zs::virtual_machine_unique_ptr vmachine;
  //  zs::virtual_machine& vm;
  zs::vm vm;
  zs::engine* eng;
  zs::object closure;
  zs::object value;
  std::string code;
  zs::error_result error;
};

template <class Fct>
inline void run_code_test(std::string_view code, size_t stack_size, std::string_view source_name, Fct&& fct) {
  zs::virtual_machine* vmachine = zs::create_virtual_machine(stack_size);
  zs::virtual_machine& vm = *vmachine;

  zs::object closure;
  REQUIRE(!vm.compile_buffer(code, source_name, closure));
  REQUIRE(closure.is_closure());

  zs::int_t n_params = 1;
  vm.push_root();

  zs::object ret_value;
  REQUIRE(!vm.call(closure, n_params, vm.stack_size() - n_params, ret_value));

  fct(vm, closure);
}

template <class Fct>
inline void run_code_test(std::string_view code, Fct&& fct) {
  run_code_test(code, 128, "test", std::forward<Fct>(fct));
}

template <class Fct>
inline void run_file_test(const char* filepath, Fct&& fct) {
  zb::file_view file;
  REQUIRE(!file.open(filepath));

  run_code_test(file.str(), 128, "test", std::forward<Fct>(fct));
}

} // namespace ztest.