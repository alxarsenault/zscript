//
// #include <ztests/ztests.h>
// #include <zscript/zscript.h>
// #include <zbase/utility/print.h>
// #include "lang/ztoken.h"
// #include "lang/zlexer.h"
// #include "lang/zcompiler.h"
//
//
// #include "zvirtual_machine.h"
// #include "zfunction_state.h"
//
// #include <filesystem>
// #include <span>
// #include <zbase/sys/file_view.h>
//
// std::vector<zs::token_type> run_string(zs::preprocessor_lexer& plexer,
// std::string_view content) {
//  plexer.init(content);
//
//  std::vector<zs::token_type> toks;
//
//  zs::token_type t = ZS_TOK(eof);
//  while (!zb::is_one_of((t = plexer.lex()), ZS_TOK(eof), ZS_TOK(lex_error))) {
//    toks.push_back(t);
//  }
//
//  if (t == ZS_TOK(lex_error)) {
//    toks.push_back(t);
//  }
//
//  return toks;
//}
//
// std::vector<zs::token_type> run_string(
//    zs::preprocessor_lexer& plexer, std::string_view content,
//    std::vector<std::string>& identifiers) {
//  plexer.init(content);
//
//  std::vector<zs::token_type> toks;
//
//  zs::token_type t = ZS_TOK(eof);
//  while (!zb::is_one_of((t = plexer.lex()), ZS_TOK(eof), ZS_TOK(lex_error))) {
//    toks.push_back(t);
//
//    if (t == ZS_TOK(identifier)) {
//      identifiers.push_back(std::string(plexer._identifier));
//    }
//  }
//
//  if (t == ZS_TOK(lex_error)) {
//    toks.push_back(t);
//  }
//
//  return toks;
//}
//
// std::vector<zs::token_type> run_string(std::string_view content) {
//  zs::preprocessor_lexer plexer;
//  return run_string(plexer, content);
//}
//
// std::pair<std::vector<zs::token_type>, std::vector<std::string>>
// run_string_with_identifiers(
//    std::string_view content) {
//  zs::preprocessor_lexer plexer;
//  std::vector<std::string> identifiers;
//  return { run_string(plexer, content, identifiers), identifiers };
//}
//
// std::vector<zs::token_type> run_string(std::string_view content,
// std::vector<std::string>& identifiers) {
//  zs::preprocessor_lexer plexer;
//  return run_string(plexer, content, identifiers);
//}
//
// std::vector<zs::token_type> run_file(zs::preprocessor_lexer& plexer, const
// std::filesystem::path& filepath)
// {
//  zb::file_view file;
//  REQUIRE(!file.open(filepath));
//  return run_string(plexer, file.str());
//}
//
// std::vector<zs::token_type> run_file(const std::filesystem::path& filepath) {
//  zb::file_view file;
//  REQUIRE(!file.open(filepath));
//  return run_string(file.str());
//}
//
// std::pair<std::vector<zs::token_type>, std::vector<std::string>>
// run_file_with_identifiers(
//    const std::filesystem::path& filepath) {
//  zb::file_view file;
//  REQUIRE(!file.open(filepath));
//  return run_string_with_identifiers(file.str());
//}
//
// template <class... Tokens>
// inline bool compare_tokens(const std::vector<zs::token_type>& tokens,
// Tokens... req_tokens) {
//  std::vector<zs::token_type> rtoks = { req_tokens... };
//  return rtoks == tokens;
//}
//
// inline bool compare_tokens(
//    const std::vector<zs::token_type>& tokens,
//    std::initializer_list<zs::token_type> req_tokens) {
//  std::vector<zs::token_type> rtoks = { req_tokens };
//  return rtoks == tokens;
//}
//
// inline bool compare_tokens(const std::pair<std::vector<zs::token_type>,
// std::vector<std::string>>& values,
//    std::initializer_list<zs::token_type> req_tokens,
//    std::initializer_list<std::string> req_identifiers) {
//  std::vector<zs::token_type> rtoks = { req_tokens };
//  std::vector<std::string> ridentifiers = { req_identifiers };
//  return rtoks == values.first && values.second == ridentifiers;
//}
//
// inline bool compare_tokens(const std::pair<std::vector<zs::token_type>,
// std::vector<std::string>>& values,
//    const std::vector<zs::token_type>& tokens, const std::vector<std::string>&
//    identifiers) {
//  return tokens == values.first && values.second == identifiers;
//}
//
// template <class... Tokens>
// inline bool compare_n_tokens(std::vector<zs::token_type> tokens, Tokens...
// req_tokens) {
//  std::vector<zs::token_type> rtoks = { req_tokens... };
//  tokens.resize(rtoks.size());
//  return tokens == rtoks;
//}
//
// TEST_CASE("preprocessor_lexer") {
//  REQUIRE(compare_n_tokens(run_string("#macros ABC() {}"),
//  ZS_TOK(lex_error)));
//
//  REQUIRE(compare_tokens(run_string("#macro ABC() {}"),
//      ZS_TOK(macro, identifier, lbracket, rbracket, lcrlbracket,
//      rcrlbracket)));
//
//  REQUIRE(compare_tokens(run_string("#macro ABC {}"),
//      ZS_TOK(macro), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lcrlbracket), //
//      ZS_TOK(rcrlbracket)));
//
//  REQUIRE(compare_tokens(run_string("#macro ABC {{}}"),
//      ZS_TOK(macro), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lcrlbracket), //
//      ZS_TOK(lcrlbracket), //
//      ZS_TOK(rcrlbracket), //
//      ZS_TOK(rcrlbracket)));
//
//  REQUIRE(compare_tokens(run_string("#macro ABC(a, b, c) {}"),
//      ZS_TOK(macro), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(identifier), //
//      ZS_TOK(comma), //
//      ZS_TOK(identifier), //
//      ZS_TOK(comma), //
//      ZS_TOK(identifier), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(lcrlbracket), //
//      ZS_TOK(rcrlbracket)));
//
//  REQUIRE(compare_tokens(run_string("#define ABC"),
//      ZS_TOK(define), //
//      ZS_TOK(identifier)));
//
//  REQUIRE(compare_n_tokens(run_string(R""""(
//      #if true
//        print("A");
//      #elif false
//        print("B");
//      #else
//        print("C");
//      #endif
//      )""""),
//      // #if
//      ZS_TOK(preprocessor_if), //
//      ZS_TOK(true), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(string_value), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(semi_colon), //
//      // #elif
//      ZS_TOK(preprocessor_elif), //
//      ZS_TOK(false), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(string_value), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(semi_colon), //
//      // #else
//      ZS_TOK(preprocessor_else), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(string_value), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(semi_colon), //
//      ZS_TOK(preprocessor_endif)));
//
//  REQUIRE(compare_n_tokens(run_string("#if 1\nprint(\"A\");
//  #else\nprint(\"B\");\n#endif"),
//      ZS_TOK(preprocessor_if), //
//      ZS_TOK(integer_value), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(string_value), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(semi_colon), //
//      ZS_TOK(preprocessor_else), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(string_value), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(semi_colon), //
//      ZS_TOK(preprocessor_endif)));
//
//  // #macro ABC(x) {
//  //   x
//  // }
//  REQUIRE(compare_tokens(run_file(ZSCRIPT_SAMPLES_DIRECTORY "/macro_01.zs"),
//      ZS_TOK(macro), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(identifier), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(lcrlbracket), //
//      ZS_TOK(identifier), //
//      ZS_TOK(rcrlbracket)));
//
//  // #macro ABC(a, b, c) {
//  //   a, b, c
//  // }
//  REQUIRE(compare_tokens(run_file(ZSCRIPT_SAMPLES_DIRECTORY "/macro_02.zs"),
//      ZS_TOK(macro), //
//      ZS_TOK(identifier), //
//      ZS_TOK(lbracket), //
//      ZS_TOK(identifier), //
//      ZS_TOK(comma), //
//      ZS_TOK(identifier), //
//      ZS_TOK(comma), //
//      ZS_TOK(identifier), //
//      ZS_TOK(rbracket), //
//      ZS_TOK(lcrlbracket), //
//      ZS_TOK(identifier), //
//      ZS_TOK(comma), //
//      ZS_TOK(identifier), //
//      ZS_TOK(comma), //
//      ZS_TOK(identifier), //
//      ZS_TOK(rcrlbracket)));
//
//  REQUIRE(compare_tokens(run_file_with_identifiers(ZSCRIPT_SAMPLES_DIRECTORY
//  "/macro_02.zs"),
//      { //
//          ZS_TOK(macro), //
//          ZS_TOK(identifier), //
//          ZS_TOK(lbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rbracket), //
//          ZS_TOK(lcrlbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rcrlbracket) },
//      {
//          "ABC",
//          "a",
//          "b",
//          "c",
//          "a",
//          "b",
//          "c",
//      }));
//
//  // #macro ABC(a, b, c) {
//  //   a + b + c
//  // }
//  REQUIRE(compare_tokens(run_file_with_identifiers(ZSCRIPT_SAMPLES_DIRECTORY
//  "/macro_03.zs"),
//      { //
//          ZS_TOK(macro), //
//          ZS_TOK(identifier), //
//          ZS_TOK(lbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rbracket), //
//          ZS_TOK(lcrlbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(add), //
//          ZS_TOK(identifier), //
//          ZS_TOK(add), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rcrlbracket) },
//      {
//          "ABC",
//          "a",
//          "b",
//          "c",
//          "a",
//          "b",
//          "c",
//      }));
//
//  // #macro ABC(t, a, b, c) {
//  //   t[a] = b + c
//  // }
//  REQUIRE(compare_tokens(run_file_with_identifiers(ZSCRIPT_SAMPLES_DIRECTORY
//  "/macro_04.zs"),
//      { //
//          ZS_TOK(macro), //
//          ZS_TOK(identifier), //
//          ZS_TOK(lbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rbracket), //
//          ZS_TOK(lcrlbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(lsqrbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rsqrbracket), //
//          ZS_TOK(eq), //
//          ZS_TOK(identifier), //
//          ZS_TOK(add), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rcrlbracket) },
//      {
//          "ABC",
//          "t",
//          "a",
//          "b",
//          "c",
//          "t",
//          "a",
//          "b",
//          "c",
//      }));
//
//  // #macro ABC(a, b) {
//  //   a ## b
//  // }
//  REQUIRE(compare_tokens(run_file_with_identifiers(ZSCRIPT_SAMPLES_DIRECTORY
//  "/macro_05.zs"),
//      { //
//          ZS_TOK(macro), //
//          ZS_TOK(identifier), //
//          ZS_TOK(lbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(comma), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rbracket), //
//          ZS_TOK(lcrlbracket), //
//          ZS_TOK(identifier), //
//          ZS_TOK(preprocessor_double_hashtag), //
//          ZS_TOK(identifier), //
//          ZS_TOK(rcrlbracket) },
//      {
//          "ABC",
//          "a",
//          "b",
//          "a",
//          "b",
//      }));
//}
//
// TEST_CASE("preprocessor") {
//  const char* filepath = ZSCRIPT_SAMPLES_DIRECTORY "/macro_06.zs";
//  zs::engine shared_state;
//  zb::file_view file;
//  REQUIRE(!file.open(filepath));
//  REQUIRE(!zs::preprocess(shared_state, file.str(), filepath));
//}
