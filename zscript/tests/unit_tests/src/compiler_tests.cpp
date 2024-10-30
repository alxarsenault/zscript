
#include <ztests/ztests.h>
#include <zscript/zscript.h>
#include <zbase/utility/print.h>
#include "lang/ztoken.h"
#include "lang/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

#include <filesystem>
#include <span>
#include <zbase/sys/file_view.h>
// #include <fmt/color.h>

TEST_CASE("native_closure_call") {
  zs::vm vm;

  zs::object closure = zs::object::create_native_closure(vm.get_engine(), [&](zs::vm_ref v) {
    v->push(55);
    return 1;
  });

  REQUIRE(closure.is_native_closure());

  REQUIRE(vm->stack_size() == 0);

  vm->push(closure);
  REQUIRE(vm->stack_size() == 1);

  vm->push_root();
  REQUIRE(vm->stack_size() == 2);

  zs::int_t n_params = 1;
  bool returns = true;
  bool pop_callable = false;
  REQUIRE(!zs_call(vm.get_virtual_machine(), n_params, returns, pop_callable));

  REQUIRE(vm->stack_size() == 2);

  REQUIRE(vm->stack_get(-2).is_native_closure());
  REQUIRE(vm->stack_get(-1).is_integer());
  REQUIRE(vm->top() == 55);
}

TEST_CASE("native_closure_call2") {
  zs::vm vm;

  zs::object closure = zs::object::create_native_closure(vm.get_engine(), [&](zs::vm_ref v) {
    v->push(55);
    return 1;
  });

  REQUIRE(closure.is_native_closure());

  REQUIRE(vm.stack_size() == 0);

  vm->push(closure);
  REQUIRE(vm.stack_size() == 1);

  vm.push_root();
  REQUIRE(vm.stack_size() == 2);

  zs::int_t n_params = 1;
  bool returns = true;
  bool pop_callable = true;
  REQUIRE(!zs_call(vm, n_params, returns, pop_callable));

  REQUIRE(vm.stack_size() == 1);

  REQUIRE(vm->stack_get(-1).is_integer());
  REQUIRE(vm->top() == 55);
}

TEST_CASE("native_closure_call3") {
  zs::vm vm;

  zs::object closure = zs::object::create_native_closure(vm.get_engine(), [&](zs::vm_ref v) {
    REQUIRE(v->stack_size() == 2);

    if (!v->top().is_string()) {
      return -1;
    }

    zs::object pp = v->top();
    v->push(zs::object::create_concat_string(vm.get_engine(), "John ", pp.get_string_unchecked()));
    return 1;
  });

  REQUIRE(closure.is_native_closure());

  REQUIRE(vm.stack_size() == 0);

  vm->push(closure);
  REQUIRE(vm.stack_size() == 1);

  vm.push_root();
  REQUIRE(vm.stack_size() == 2);

  vm->push(zs::object::create_small_string("Peter"));

  zs::int_t n_params = 2;
  bool returns = true;
  bool pop_callable = true;
  REQUIRE(!zs_call(vm, n_params, returns, pop_callable));

  REQUIRE(vm.stack_size() == 1);

  REQUIRE(vm->stack_get(-1).is_string());
  REQUIRE(vm->top() == "John Peter");
}

TEST_CASE("native_closure_call4") {
  zs::vm vm;

  vm.new_closure([](zs::vm_ref vm) {
    // The root should be at arg0 and arg1 should be "Peter".
    REQUIRE(vm.stack_size() == 2);

    REQUIRE(vm.is_table(-2));
    REQUIRE(vm.is_string(-1));

    // Make sure that arg1 is a string.
    std::string_view arg1;
    REQUIRE(!vm.get_string(-1, arg1));
    REQUIRE(arg1 == "Peter");

    vm.push_string_concat("John ", arg1);
    return 1;
  });

  vm.push_root();
  vm.push_string("Peter");

  REQUIRE(vm.is_native_closure(-3));
  REQUIRE(vm.is_table(-2));
  REQUIRE(vm.is_string(-1));

  REQUIRE(!vm.call(2, true, true));

  // The returned value should be the only one on the stack.
  REQUIRE(vm.stack_size() == 1);
  REQUIRE(vm.is_string(-1));

  std::string_view res;
  REQUIRE(!vm.get_string(-1, res));
  REQUIRE(res == "John Peter");
}

inline std::string left_aligned_string(std::string s, size_t n, char fill_char) {
  if (s.size() < n) {
    s.insert(s.end(), n - s.size(), fill_char);
  }

  return s;
}

inline std::string right_aligned_string(std::string s, size_t n, char fill_char) {
  if (s.size() < n) {
    s.insert(s.begin(), n - s.size(), fill_char);
  }

  return s;
}

// #ifdef ZS_COMPILER_DEV
// inline void print_dev_token(const zs::dev_token_info& dtoken) {
//   //  zb::sprint("001:006 identifier   small_string ---");
//   //  zb::sprint("LINE    TOKEN        VALUE TYPE   VALUE");
//   //  zb::sprint(" LINE     TOKEN       VALUE TYPE  VALUE");
//
//   std::string tok_value = dtoken.value.convert_to_string();
//   if (tok_value.size() > 15) {
//     tok_value.resize(12);
//     tok_value.push_back('.');
//     tok_value.push_back('.');
//     tok_value.push_back('.');
//   }
//   //  else {
//   //
//   //  }
//
//   // #ifdef ZS_DEV_DEBUG
//   //   if (dtoken.value.get_type() == zs::object_type::k_none) {
//   //     zb::print<"">("│ ",
//   right_aligned_string(std::to_string(dtoken.linfo.line), 3, '0'), ":",
//   //         right_aligned_string(std::to_string(dtoken.linfo.column), 3,
//   '0'), " │ ",
//   //         left_aligned_string(zs::token_to_string(dtoken.token), 13, ' '),
//   " │ ",
//   //         left_aligned_string("", 12, ' '), " │ ",
//   left_aligned_string(tok_value, 15, ' '), " │");
//   //   }
//   //   else {
//   //     zb::print<"">("│ ",
//   right_aligned_string(std::to_string(dtoken.linfo.line), 3, '0'), ":",
//   //         right_aligned_string(std::to_string(dtoken.linfo.column), 3,
//   '0'), " │ ",
//   //         left_aligned_string(zs::token_to_string(dtoken.token), 13, ' '),
//   " │ ",
//   // left_aligned_string(zs::object_type_name(dtoken.value.get_type()), 12, '
//   '), " │ ",
//   //         left_aligned_string(tok_value, 15, ' '), " │");
//   //   }
//   // #else
//   zb::print<"">("| ", right_aligned_string(std::to_string(dtoken.linfo.line),
//   3, '0'), ":",
//       right_aligned_string(std::to_string(dtoken.linfo.column), 3, '0'), " |
//       ", left_aligned_string(zs::token_to_string(dtoken.token), 13, ' '), " |
//       ",
//       left_aligned_string(zs::get_object_type_name(dtoken.value.get_type()),
//       12, ' '), " | ", left_aligned_string(tok_value, 15, ' '), " |");
//
//   // #endif  ZS_DEV_DEBUG.
// }
//
// inline void print_compiler_dev_info(const zs::compiler& compiler) {
//   zb::print("┌─────────┬───────────────┬──────────────┬─────────────────┐");
//   zb::print("│  LINE   │     TOKEN     │  VALUE TYPE  │ VALUE           │");
//   zb::print("├─────────┼───────────────┼──────────────┼─────────────────┤");
//   for (const auto& k : compiler._dev._info) {
//     print_dev_token(k);
//   }
//   zb::print("└─────────┴───────────────┴──────────────┴─────────────────┘\n");
// }
//
// #else
// #endif // ZS_COMPILER_DEV

TEST_CASE("compiler") {
  {
    REQUIRE((1 >> 1) == 0);

    const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_01.zs";
    zs::engine eng;

    zb::file_view file;
    REQUIRE(!file.open(filepath));

    zs::object result;
    zs::jit_compiler compiler(&eng);
    auto res = compiler.compile(file.str(), filepath, result);
    REQUIRE(!res);

    //    print_compiler_dev_info(compiler);
    //    zb::print("| 001:006 | local        | none         |");

    //    return comp.compile(shared_state, content, filename);
    //    auto res = zs::compile(shared_state, file.str(), filepath);
    //    REQUIRE(!res);
  }

  {
    const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_02.zs";
    zs::engine eng;
    zb::file_view file;
    REQUIRE(!file.open(filepath));

    zs::object result;
    zs::jit_compiler compiler(&eng);
    auto res = compiler.compile(file.str(), filepath, result);
    REQUIRE(!res);

    //    print_compiler_dev_info(compiler);

    //    return comp.compile(shared_state, content, filename);
    //    auto res = zs::compile(shared_state, file.str(), filepath);
    //    REQUIRE(!res);
  }
  //

  // REQUIRE(!zs::preprocess(shared_state, file.str(), filepath));
}

// TEST_CASE("compiler") {
//   //  REQUIRE(zs::tok_none == 1);
//
//   {
//     zs::state state;
//     std::string code = "{}";

//  }
//}

// inline constexpr std::string_view z_std_file_content =
// #include "/Users/alexarse/Develop/wp/modules/zscript/std/z_std.h"
//     ;
//
// TEST_CASE("zstd") { zb::print(z_std_file_content); }
