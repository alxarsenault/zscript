#pragma once

#include <catch2.h>
#include <fstream>

#include <zscript/base/sys/file_view.h>
#include <zscript/base/utility/print.h>

#include <zscript/zscript.h>
#include "zvirtual_machine.h"
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "jit/zjit_compiler.h"
#include "object/zfunction_prototype.h"

namespace utest {

inline size_t& get_unique_number(const std::string& name) {
  static std::unordered_map<std::string, size_t> sections_map;
  return ++sections_map[name];
}

inline std::string generate_test_name(const std::string& name) {
  std::stringstream ss;
  ss << name << "." << std::setfill('0') << std::setw(3) << get_unique_number(name);
  return ss.str();
}

extern std::string s_current_test_name;

inline void empty_init_function(zs::virtual_machine& vm) {}

inline constexpr uint32_t compile_good = 16;
inline constexpr uint32_t compile_fail = 32;
inline constexpr uint32_t call_good = 64;
inline constexpr uint32_t call_fail = 128;
inline constexpr uint32_t call_skip = 256;

enum class test_type { file, code };

using enum test_type;

struct options {

  inline options() = default;

  inline options(const std::string& content)
      : content(content) {

    if (std::filesystem::path(content).extension() == ".zs") {
      type = file;
    }
  }

  template <class Lamda>
    requires std::is_constructible_v<std::function<void(zs::vm_ref)>, Lamda>
  inline options(const std::string& content, Lamda&& lamda)
      : content(content)
      , init(std::forward<Lamda>(lamda)) {

    if (std::filesystem::path(content).extension() == ".zs") {
      type = file;
    }
  }

  template <class Lamda>
    requires std::is_constructible_v<std::function<void(zs::vm_ref)>, Lamda>
  inline options(const std::string& content, Lamda&& lamda, uint32_t flags)
      : content(content)
      , init(std::forward<Lamda>(lamda))
      , flags(flags) {

    if (std::filesystem::path(content).extension() == ".zs") {
      type = file;
    }
  }

  template <class Lamda>
    requires std::is_constructible_v<std::function<void(zs::vm_ref)>, Lamda>
  inline options(const std::string& content, uint32_t flags, Lamda&& lamda)
      : content(content)
      , init(std::forward<Lamda>(lamda))
      , flags(flags) {

    if (std::filesystem::path(content).extension() == ".zs") {
      type = file;
    }
  }

  inline options(const std::string& content, uint32_t flags)
      : content(content)
      , flags(flags) {

    if (std::filesystem::path(content).extension() == ".zs") {
      type = file;
    }
  }

  inline options(uint32_t flags, const std::string& content)
      : content(content)
      , flags(flags) {

    if (std::filesystem::path(content).extension() == ".zs") {
      type = file;
    }
  }

  inline options(test_type type, const std::string& content)
      : type(type)
      , content(content) {}

  test_type type = code;
  std::string content;
  std::function<void(zs::vm_ref)> init;
  uint32_t flags = compile_good | call_good;
};

static zs::int_t check_impl(zs::vm_ref vm) {
  zs::int_t nargs = vm.stack_size();

  if (nargs < 2) {
    return -1;
  }

  if (vm[1].is_if_true()) {
    return vm.push_bool(true);
  }

  zb::print("TEST FAILED:");

  for (zs::int_t i = 2; i < nargs; i++) {
    zb::print(vm[i]);
  }

  return -1;
  //  return vm.push_bool(false);
}

struct test_case {

  inline test_case(const std::string& test_name, const options& opts)
      : vm(1024)
      , _options(opts) {

    if (opts.type == test_type::file) {
      zs::file_loader loader(vm.get_engine());

      if (auto err = loader.open(opts.content)) {
        FAIL("Could not load file '" + opts.content + "' : " + std::string(err.message()));
        return;
      }

      code = loader.content();
    }
    else {
      code = opts.content;
    }

    vm.get_engine()->add_import_directory(ZSCRIPT_MODULES_DIRECTORY);
    vm.get_engine()->add_import_directory(ZSCRIPT_TESTS_RESOURCES_DIRECTORY);

    vm->global().as_table().set(zs::_s(vm.get_engine(), "ZSCRIPT_TESTS_RESOURCES_DIRECTORY"),
        zs::_s(vm.get_engine(), ZSCRIPT_TESTS_RESOURCES_DIRECTORY));

    vm->global().as_table().set(zs::_s(vm.get_engine(), "ZSCRIPT_EXAMPLES_DIRECTORY"),
        zs::_s(vm.get_engine(), ZSCRIPT_EXAMPLES_DIRECTORY));

    vm->global().as_table().set(zs::_s(vm.get_engine(), "ZSCRIPT_TESTS_OUTPUT_DIRECTORY"),
        zs::_s(vm.get_engine(), ZSCRIPT_TESTS_OUTPUT_DIRECTORY));

    vm->global().as_table().set(zs::_s(vm.get_engine(), "ZSCRIPT_MODULES_DIRECTORY"),
        zs::_s(vm.get_engine(), ZSCRIPT_MODULES_DIRECTORY));

    vm->global().as_table().set(zs::_ss("__zcheck"), check_impl);

    if (opts.init) {
      opts.init(vm);
    }

    if (auto err = vm->compile_buffer(code, test_name, closure)) {
      error = err;

      if ((opts.flags & compile_good)) {
        FAIL_CHECK("Compile failed ");

        zb::print(vm.get_error());
        return;
      }

      all_good = true;
      return;
    }

    if ((opts.flags & compile_fail)) {
      FAIL_CHECK("Compile succeeded");
      return;
    }

    if (!closure.is_closure()) {
      FAIL_CHECK("Not a closure");
      return;
    }

    if ((opts.flags & call_skip)) {
      all_good = true;
      return;
    }

    //        zb::print(closure.as_closure().get_proto()._module_info.to_json());
    //    zs::object env = vm->create_this_table_from_root();
    zs::object env = vm->global();
    if (auto err = vm->call(closure, env, value)) {
      error = err;
      if ((opts.flags & call_good)) {
        FAIL_CHECK("Call failed\n" + vm.get_error());
        return;
      }

      //      closure.as_closure().set_bounded_this(std::move(env));
      all_good = true;
      return;
    }

    if ((opts.flags & call_fail)) {
      FAIL_CHECK("Call succeeded");
      return;
    }

    all_good = true;
    //        zb::print("ROOT:", vm->get_root());

    //        zb::print("LOCAL:", root);
  }

  zs::vm vm;
  options _options;
  zs::object closure;
  zs::object value;
  std::string code;
  zs::error_result error;
  bool all_good = false;
};

#define __ZTEST_CASE_INTERNAL(TestStructName, test_name, Options)                                        \
  CATCH_INTERNAL_START_WARNINGS_SUPPRESSION                                                              \
  CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS                                                               \
  CATCH_INTERNAL_SUPPRESS_UNUSED_VARIABLE_WARNINGS                                                       \
  namespace {                                                                                            \
    struct TestStructName {};                                                                            \
  }                                                                                                      \
  template <>                                                                                            \
  struct zs::internal::test_helper<TestStructName> : utest::test_case {                                  \
    static inline std::string& get_test_name() {                                                         \
      static std::string n = utest::generate_test_name(test_name);                                       \
      return n;                                                                                          \
    }                                                                                                    \
    inline test_helper()                                                                                 \
        : test_case(test_helper::get_test_name(), Options) {}                                            \
    void test();                                                                                         \
  };                                                                                                     \
                                                                                                         \
  const Catch::AutoReg INTERNAL_CATCH_UNIQUE_NAME(autoRegistrar)(                                        \
      Catch::makeTestInvoker(&zs::internal::test_helper<TestStructName>::test), CATCH_INTERNAL_LINEINFO, \
      "test_case_catch_sr",                                                                              \
      Catch::NameAndTags{ zs::internal::test_helper<TestStructName>::get_test_name() });                 \
  CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION                                                               \
  void zs::internal::test_helper<TestStructName>::test()

#define ZTEST_CASE(test_name, ...) \
  __ZTEST_CASE_INTERNAL(           \
      INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), test_name, utest::options(__VA_ARGS__))

#define __ZS_TEST_CASE_INTERNAL(TestStructName, test_name, Options)                                      \
  CATCH_INTERNAL_START_WARNINGS_SUPPRESSION                                                              \
  CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS                                                               \
  CATCH_INTERNAL_SUPPRESS_UNUSED_VARIABLE_WARNINGS                                                       \
  namespace {                                                                                            \
    struct TestStructName {};                                                                            \
  }                                                                                                      \
  template <>                                                                                            \
  struct zs::internal::test_helper<TestStructName> {                                                     \
    static inline std::string& get_test_name() {                                                         \
      static std::string n = utest::generate_test_name(test_name);                                       \
      return n;                                                                                          \
    }                                                                                                    \
    inline test_helper() {}                                                                              \
    void test();                                                                                         \
  };                                                                                                     \
                                                                                                         \
  const Catch::AutoReg INTERNAL_CATCH_UNIQUE_NAME(autoRegistrar)(                                        \
      Catch::makeTestInvoker(&zs::internal::test_helper<TestStructName>::test), CATCH_INTERNAL_LINEINFO, \
      "test_case_catch_sr",                                                                              \
      Catch::NameAndTags{ zs::internal::test_helper<TestStructName>::get_test_name() });                 \
  CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION                                                               \
  void zs::internal::test_helper<TestStructName>::test()

#define UTEST_CASE(test_name, ...) \
  __ZS_TEST_CASE_INTERNAL(         \
      INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), test_name, utest::options(__VA_ARGS__))

} // namespace utest.
