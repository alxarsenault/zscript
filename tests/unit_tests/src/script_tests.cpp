
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

inline bool compare_tokens(std::span<const zs::token_type> toks, std::span<const zs::token_type> t) {
  REQUIRE(toks.size() == t.size());

  for (size_t i = 0; i < toks.size(); i++) {
    REQUIRE(toks[i] == t[i]);
  }

  return true;
}

inline bool compare_tokens(std::span<const zs::token_type> toks, std::initializer_list<zs::token_type> t) {
  return compare_tokens(toks, std::span<const zs::token_type>(t));
}

TEST_CASE("token") {
  {
    zs::engine eng;
    zs::lexer lexer(&eng);
    std::string code = "{}";
    lexer.init(code);
    std::vector<zs::token_type> toks;

    zs::token_type t = ZS_TOK(eof);
    while ((t = lexer.lex()) != ZS_TOK(eof)) {
      toks.push_back(t);
    }

    REQUIRE(toks.size() == 2);
    REQUIRE(toks[0] == zs::token_type::tok_lcrlbracket);
    REQUIRE(toks[1] == zs::token_type::tok_rcrlbracket);
  }
  //
}

TEST_CASE("lexer_02") {
  using enum zs::token_type;
  std::string_view code = "{ a= 32, b = 'j'}";

  zs::engine eng;

  {
    zs::lexer lexer(&eng, code);
    std::vector<zs::token_type> toks;

    toks.resize(9);
    std::span<zs::token_type> sp(toks);
    lexer.peek(sp);

    REQUIRE(sp.size() == 9);

    REQUIRE(compare_tokens(sp,
        { ZS_TOK(lcrlbracket, identifier, eq, integer_value, comma, identifier, eq, integer_value,
            rcrlbracket) }));
  }

  {
    zs::lexer lexer(&eng, code);
    std::vector<zs::token_type> toks;

    lexer.lex();
    lexer.lex();
    toks.resize(9);
    std::span<zs::token_type> sp(toks);
    lexer.peek(sp);

    REQUIRE(sp.size() == 8);
    REQUIRE(compare_tokens(
        sp, { ZS_TOK(eq, integer_value, comma, identifier, eq, integer_value, rcrlbracket, eof) }));
  }

  {
    zs::lexer lexer(&eng, code);

    REQUIRE(!lexer.lex_compare(
        { ZS_TOK(lcrlbracket, identifier, eq, integer_value, comma, identifier, eq, integer_value) }));
    REQUIRE(lexer.lex() == tok_rcrlbracket);
  }

  {
    zs::lexer lexer(&eng, code);
    std::vector<zs::token_type> toks;

    toks.resize(7);
    std::span<zs::token_type> sp(toks);
    REQUIRE(!lexer.lex(sp));

    REQUIRE(
        compare_tokens(sp, { ZS_TOK(lcrlbracket, identifier, eq, integer_value, comma, identifier, eq) }));
  }
  //
}
TEST_CASE("vm") {

  zs::vm vm;

  bool did_call = false;

  REQUIRE(!vm.new_closure([&](zs::vm_ref v) {
    did_call = true;
    return 3;
  }));
  //
  //    REQUIRE(obj.get_type() == zs::object_type::k_native_closure);
  //

  vm[-1].as_native_closure().call(vm);
  //      REQUIRE(zs::object_native_closure_value(obj)->call(vm) == 3);
  REQUIRE(did_call);
}
