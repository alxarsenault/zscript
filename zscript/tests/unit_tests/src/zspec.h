#pragma once

#include "ztests.h"
#include "lang/ztoken.h"
#include "lang/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"
#include <fstream>

#define ZSECTION(X) zspec::section::X

// ### 1.1 Single line comment

//[***comments-001***]
//
//```zscript
//// This is a single line comment.
//```

namespace zspec {

enum class section {
  identifier,
  keywords,
  comments,
  variable_declaration,
  typed_variable_declaration,
  function_declaration,
  compare,
  fs_lib
};

template <section Section>
inline constexpr const char* section_name();

template <>
inline constexpr const char* section_name<section::identifier>() {
  return "identifier";
}

template <>
inline constexpr const char* section_name<section::keywords>() {
  return "keywords";
}

template <>
inline constexpr const char* section_name<section::comments>() {
  return "comments";
}

template <>
inline constexpr const char* section_name<section::variable_declaration>() {
  return "var-decl";
}

template <>
inline constexpr const char* section_name<section::typed_variable_declaration>() {
  return "typed-var-decl";
}

template <>
inline constexpr const char* section_name<section::function_declaration>() {
  return "function-decl";
}

template <>
inline constexpr const char* section_name<section::compare>() {
  return "compare";
}

template <>
inline constexpr const char* section_name<section::fs_lib>() {
  return "filesystem";
}

template <section Section>
inline size_t get_section_count() {
  static size_t count = 0;
  return ++count;
}

template <section Section>
inline std::pair<size_t, size_t>& get_subsection_count() {
  static std::pair<size_t, size_t> count = { 0, 0 };
  return count;
}

// template <section Section>
// inline size_t& get_subsection_index_count()
//{
//   static size_t count = 0;
//   return count;
// }

inline std::pair<size_t, size_t>& get_subsection_count(section s) {
  switch (s) {
  case section::comments:
    return get_subsection_count<section::comments>();
  case section::variable_declaration:
    return get_subsection_count<section::variable_declaration>();
  case section::function_declaration:
    return get_subsection_count<section::function_declaration>();
  case section::compare:
    return get_subsection_count<section::compare>();
  case section::typed_variable_declaration:
    return get_subsection_count<section::typed_variable_declaration>();
  case section::identifier:
    return get_subsection_count<section::identifier>();
  case section::keywords:
    return get_subsection_count<section::keywords>();
  case section::fs_lib:
    return get_subsection_count<section::fs_lib>();
  }

  zbase_error("Invalid");
  static std::pair<size_t, size_t> tmp;
  return tmp;
}

template <section Section>
inline size_t begin_subsection() {
  auto& k = get_subsection_count<Section>();
  k.second = 0;
  return ++k.first;
}

template <section Section>
inline std::string generate_test_name() {
  std::stringstream ss;
  ss << section_name<Section>() << "-" << std::setfill('0') << std::setw(3) << get_section_count<Section>();

  return ss.str();
}

struct z_spec_stream : std::ofstream {
  inline z_spec_stream(const char* filepath) {
    this->open(filepath);
    (*this) << "# Specification\n\n";
    (*this) << "bkbkbbbkb\n";
  }
};

inline z_spec_stream& get_stream() {
  static z_spec_stream streeam(ZSCRIPT_DOC_DIRECTORY "/spec.md");
  return streeam;
}

struct z_code_test_fixture {
  inline z_code_test_fixture(section sec, std::string_view ccode, std::string_view ccode_extra,
      std::string_view spec_name, std::string_view title, std::string_view description, int require_compile,
      int require_call)
      : vm(128)
      , code(ccode) {

    std::string_view spec_num_str = spec_name.substr(spec_name.size() - 3);

    const int spec_num = std::stoi(std::string(spec_num_str));
    //    spec_name.remove_suffix(4);

    const int sindex = (int)sec + 1;

    // ### 1.1 Single line comment

    //[***comments-001***]
    //
    //```zscript
    //// This is a single line comment.
    //```

    //    zb::stream_print(get_stream(),
    //      "\n### ",
    //      sindex,
    //      ".",
    //      spec_num,
    //      " ",
    //      title,
    //      "\n\n[***",
    //      spec_name,
    //      "***]\n\n");
    std::pair<size_t, size_t>& subs = zspec::get_subsection_count(sec);
    ++subs.second;
    if (subs.first) {

      zb::stream_print(get_stream(), "\n#### ", sindex, ".", subs.first, ".", subs.second, " ", title,
          "  <sub><sup>[", spec_name, "]</sup></sub>\n\n");
    }
    else {

      zb::stream_print(get_stream(), "\n#### ", sindex, ".", spec_num, " ", title, "  <sub><sup>[", spec_name,
          "]</sup></sub>\n\n");
    }
    //    zb::stream_print(get_stream(),
    //      "\n### ",
    //      sindex,
    //      ".",
    //      spec_num,
    //      " ",
    //      spec_name,
    //      "-",
    //      spec_num_str,
    //      "\n\n",
    //      title,
    //      "\n\n");

    if (!description.empty()) {
      zb::stream_print(get_stream(), description, "\n\n");
    }

    // Remove the end line character if it's starts with one.
    if (ccode.starts_with("\n")) {
      ccode = ccode.substr(1);
    }

    zb::stream_print(get_stream(), "```zscript\n", ccode, "\n```\n");

    full_code = code + "\n" + std::string(ccode_extra);

    vm.get_engine()->add_import_directory(ZSCRIPT_STANDARD_TESTS_RESOURCES_DIRECTORY);

    if (auto err = vm->compile_buffer(full_code, "test", closure)) {
      error = err;

      if (require_compile == ZGOOD) {
        zb::print(vm.get_error());
        REQUIRE(false);
      }

      return;
    }

    if (require_compile == ZBAD) {
      zb::print(vm.get_error());
      REQUIRE(false);
    }

    REQUIRE(closure.is_closure());

    zs::int_t n_params = 1;
    vm.push_root();

    // Top index = 3
    // N param = 1

    //
    if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, value)) {
      error = err;
      if (require_call == ZGOOD) {
        zb::print(vm.get_error());
        REQUIRE(false);
      }

      return;
    }

    vm.push(value);

    if (require_call == ZBAD) {
      zb::print(vm.get_error());
      REQUIRE(false);
    }
  }

  zs::vm vm;
  //  zs::virtual_machine_unique_ptr vmachine;
  //  zs::virtual_machine& vm;
  zs::object closure;
  zs::object value;
  std::string code;
  zs::error_result error;
  std::string full_code;
};
} // namespace zspec.

#define ZS_SPEC_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(                                                      \
    TestName, ClassName, code, code_extra, sec, title, desc, require_compile, require_call)                 \
  CATCH_INTERNAL_START_WARNINGS_SUPPRESSION                                                                 \
  CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS                                                                  \
  CATCH_INTERNAL_SUPPRESS_UNUSED_VARIABLE_WARNINGS                                                          \
  namespace {                                                                                               \
    struct TestName : ClassName {                                                                           \
      static inline std::string& get_test_name() {                                                          \
        static std::string n = zspec::generate_test_name<sec>();                                            \
        return n;                                                                                           \
      }                                                                                                     \
      inline TestName()                                                                                     \
          : ClassName(sec, code, code_extra, TestName::get_test_name(), title, desc, require_compile,       \
                require_call) {}                                                                            \
      void test();                                                                                          \
    };                                                                                                      \
                                                                                                            \
    const Catch::AutoReg INTERNAL_CATCH_UNIQUE_NAME(autoRegistrar)(Catch::makeTestInvoker(&TestName::test), \
        CATCH_INTERNAL_LINEINFO, #ClassName##_catch_sr, Catch::NameAndTags{ TestName::get_test_name() });   \
  }                                                                                                         \
  CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION                                                                  \
  void TestName::test()

#define ZS_SPEC_TEST_5(sec, title, desc, code, code_extra)                                         \
  ZS_SPEC_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), \
      zspec::z_code_test_fixture, code, code_extra, ZSECTION(sec), title, desc, ZGOOD, ZGOOD)

#define ZS_SPEC_TEST_6(sec, title, desc, code, code_extra, require_compile)                        \
  ZS_SPEC_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), \
      zspec::z_code_test_fixture, code, code_extra, ZSECTION(sec), title, desc, require_compile, ZGOOD)

#define ZS_SPEC_TEST_7(sec, title, desc, code, code_extra, require_compile, require_call)          \
  ZS_SPEC_TEST_INTERNAL_CATCH_TEST_CASE_METHOD2(INTERNAL_CATCH_UNIQUE_NAME(CATCH2_INTERNAL_TEST_), \
      zspec::z_code_test_fixture, code, code_extra, ZSECTION(sec), title, desc, require_compile,   \
      require_call)

#define ZS_SPEC_TEST(...) ZBASE_DEFER(ZBASE_CONCAT(ZS_SPEC_TEST_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

#define ZS_SPEC_SECTION(sec, name, description)                                                         \
  TEST_CASE(ZBASE_STRINGIFY(INTERNAL_CATCH_UNIQUE_NAME(__SPEC_SECTION))) {                              \
    zb::stream_print<"">(                                                                               \
        zspec::get_stream(), "\n## ", ((int)ZSECTION(sec) + 1), ". ", name, "\n\n", description, "\n"); \
  }

#define ZS_SPEC_SUBSECTION(sec, name, description)                                        \
  TEST_CASE(ZBASE_STRINGIFY(INTERNAL_CATCH_UNIQUE_NAME(__SPEC_SUBSECTION))) {             \
    zb::stream_print<"">(zspec::get_stream(), "\n### ", ((int)ZSECTION(sec) + 1), ".",    \
        zspec::begin_subsection<ZSECTION(sec)>(), ". ", name, "\n\n", description, "\n"); \
  }
