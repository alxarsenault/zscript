
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

#include <filesystem>
#include <span>

#include <zbase/sys/file_view.h>

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
//
//   zb::print<"">("| ", right_aligned_string(std::to_string(dtoken.linfo.line),
//   3, '0'), ":",
//       right_aligned_string(std::to_string(dtoken.linfo.column), 3, '0'), " |
//       ", left_aligned_string(zs::token_to_string(dtoken.token), 13, ' '), " |
//       ",
//       left_aligned_string(zs::get_object_type_name(dtoken.value.get_type()),
//       12, ' '), " | ", left_aligned_string(tok_value, 15, ' '), " |");
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
// #endif // ZS_COMPILER_DEV

TEST_CASE("dsdsdsdsdsdsdsdsd") { 
  const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_03.zs";
  zs::vm vm;
  zs::engine* eng = vm.get_engine();
  
  zb::file_view file;
  REQUIRE(!file.open(filepath));
 
  zs::object fpo;
  zs::jit_compiler compiler(eng);
  if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
    zb::print(compiler.get_error() );
    REQUIRE(false);
  }
 
 
  REQUIRE(fpo.is_function_prototype());
  fpo.as_proto().debug_print();
  
zs::object  closure = zs::object::create_closure(eng, fpo, vm->get_root());
  
  zs::object result;
  if (auto err =  vm->call(closure, {vm->get_root()}, result)) {
    zb::print(vm.get_error() );
    REQUIRE(false);
  }
 
  zb::print(result);
  //  compiler._lexer.
  //  compiler.print_handler();

  //    print_compiler_dev_info(compiler);
  //    zb::print("| 001:006 | local        | none         |");

  //    return comp.compile(shared_state, content, filename);
  //    auto res = zs::compile(shared_state, file.str(), filepath);
  //    REQUIRE(!res);
}
