#include "zlexer.h"
#include <zbase/strings/charconv.h>
#include <zbase/container/constexpr_map.h>
#include <zbase/strings/parse_utils.h>

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

namespace zs {

struct lexer::helper {

  static constexpr const auto k_symbol_map = zb::eternal::hash_map<zb::eternal::string, zs::token_type>({
      { "__FILE__", zs::token_type::tok_file },
      { "__LINE__", zs::token_type::tok_line },
      { "__THIS_LINE__", zs::token_type::tok_line_str },
      { "null", zs::token_type::tok_null },
      { "var", zs::token_type::tok_var },
      { "int", zs::token_type::tok_int },
      { "float", zs::token_type::tok_float },
      { "number", zs::token_type::tok_number },
      { "bool", zs::token_type::tok_bool },
      { "table", zs::token_type::tok_table },
      { "array", zs::token_type::tok_array },
      { "string", zs::token_type::tok_string },
      { "char", zs::token_type::tok_char },
      { "auto", zs::token_type::tok_auto },
      { "true", zs::token_type::tok_true },
      { "exttype", zs::token_type::tok_exttype },
      { "false", zs::token_type::tok_false },
      { "return", zs::token_type::tok_return },
      { "extend", zs::token_type::tok_extend },
      { "struct", zs::token_type::tok_struct },
      { "and", zs::token_type::tok_and },
      { "or", zs::token_type::tok_or },
      { "if", zs::token_type::tok_if },
      { "else", zs::token_type::tok_else },
      { "for", zs::token_type::tok_for },
      { "foreach", zs::token_type::tok_foreach },
      { "do", zs::token_type::tok_do },
      { "while", zs::token_type::tok_while },
      { "switch", zs::token_type::tok_switch },
      { "case", zs::token_type::tok_case },
      { "break", zs::token_type::tok_break },
      { "default", zs::token_type::tok_default },
      { "continue", zs::token_type::tok_continue },
      { "try", zs::token_type::tok_try },
      { "catch", zs::token_type::tok_catch },
      { "throw", zs::token_type::tok_throw },
      { "this", zs::token_type::tok_this },
      { "base", zs::token_type::tok_base },
      { "class", zs::token_type::tok_class },
      { "namespace", zs::token_type::tok_namespace },
      { "use", zs::token_type::tok_use },
      { "global", zs::token_type::tok_global },
      { "const", zs::token_type::tok_const },
      { "static", zs::token_type::tok_static },
      //      { "export", zs::token_type::tok_export },
      { "function", zs::token_type::tok_function },
      { "private", zs::token_type::tok_private },
      { "typeof", zs::token_type::tok_typeof },
      { "typeid", zs::token_type::tok_typeid },
      { "constructor", zs::token_type::tok_constructor },
      { "destructor", zs::token_type::tok_destructor },
      { "mutable", zs::token_type::tok_mutable },
      { "in", zs::token_type::tok_in },
      { "enum", zs::token_type::tok_enum },
      { "xor", zs::token_type::tok_xor },
      { "not", zs::token_type::tok_not },
      { "none", zs::token_type::tok_none },
      { "define", zs::token_type::tok_define },
  });

  //  static inline void create_symbol_map(auto& symbols) {
  //    using enum token_type;
  //    for (token_type i = next_token(token_type::tok_first_named_token); i <
  //    token_type::tok_last_named_token;
  //         i = (token_type)(((int)i) + 1)) {
  //      symbols[zs::token_to_string(i)] = i;
  //    }
  //    symbols["__FILE__"] = tok_file;
  //    symbols["__LINE__"] = tok_line;
  //
  //    //    zb::print(symbols);
  //    //    symbols["null"] = tok_null;
  //    //    symbols["local"] = tok_local;
  //    //    symbols["true"] = tok_true;
  //    //    symbols["false"] = tok_false;
  //    //    symbols["return"] = tok_return;
  //    //    symbols["extend"] = tok_extend;
  //    //    symbols["and"] = tok_and;
  //    //    symbols["or"] = tok_or;
  //    //    symbols["if"] = tok_if;
  //    //    symbols["else"] = tok_else;
  //    //    symbols["for"] = tok_for;
  //    //    symbols["do"] = tok_do;
  //    //    symbols["while"] = tok_while;
  //    //    symbols["switch"] = tok_switch;
  //    //    symbols["case"] = tok_case;
  //    //    symbols["break"] = tok_break;
  //    //    symbols["default"] = tok_default;
  //    //    symbols["continue"] = tok_continue;
  //    //    symbols["try"] = tok_try;
  //    //    symbols["catch"] = tok_catch;
  //    //    symbols["throw"] = tok_throw;
  //    //    symbols["this"] = tok_this;
  //    //    symbols["base"] = tok_base;
  //    //    symbols["class"] = tok_class;
  //    //    symbols["namespace"] = tok_namespace;
  //    //    symbols["global"] = tok_global;
  //    //    symbols["const"] = tok_const;
  //    //    symbols["static"] = tok_static;
  //    //    symbols["typeof"] = tok_typeof;
  //    //    symbols["constructor"] = tok_constructor;
  //    //    symbols["destructor"] = tok_destructor;
  //    //    symbols["in"] = tok_in;
  //    //    symbols["function"] = tok_function;
  //    //    symbols["enum"] = tok_enum;
  //    //    symbols["__FILE__"] = tok_file;
  //    //    symbols["__LINE__"] = tok_line;
  //  }

  inline static void skip_hashtag_comment(lexer* l) {
    auto& s = l->_stream;

    // Skip `#`.
    l->next();

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }

    while (*s != '\n' && !s.is_end()) {
      l->next();
    }

    if (*s == '\n') {
      s.incr();
      l->new_line();
    }

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }
  }

  inline static void skip_single_line_comment(lexer* l) {
    auto& s = l->_stream;

    // Skip `//`.
    l->next(2);

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }

    while (*s != '\n' && !s.is_end()) {
      l->next();
    }

    if (*s == '\n') {
      s.incr();
      l->new_line();
    }

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }
  }

  inline static void skip_triple_dash_doc_block_comment2(lexer* l) {
    auto& s = l->_stream;

    // Skip `/// @`.
    l->next(5);

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }

    zb_loop() {
      if (s.is_end()) {
        l->set_token(tok_eof);
        return;
      }

      if (*s == '\n') {
        s.incr();
        l->new_line();

        while (zb::is_one_of(*s, '\t', '\r', ' ')) {
          l->next();
        }

        if (s.rsize() >= 3 and s[0] == '/' and s[1] == '/' and s[2] == '/') {
          l->next(3);
          continue;
        }
        else {
          return;
        }
      }
      else {
        s.incr();
      }
    }

    if (*s == '\n') {
      s.incr();
      l->new_line();
    }
  }

  inline static void skip_triple_dash_doc_block_comment(lexer* l) {
    auto& s = l->_stream;

    const char* beg = s.ptr() + 5;

    skip_triple_dash_doc_block_comment2(l);
    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }

    const size_t sz = std::distance(beg, s.ptr());
    std::string_view svalue(beg, sz);

    zs::string& estr = l->_escaped_string;

    estr.clear();
    const char* ptr = svalue.data();
    const char* end = svalue.data() + svalue.size();

    while (ptr < end) {
      bool was_endl = *ptr == '\n';
      estr.push_back(*ptr++);

      if (was_endl and ptr < end) {
        while (*ptr++ != '/') {
        }

        ptr += 3;
      }
    }

    while (estr.ends_with('\n')) {
      estr.pop_back();
    }
  }

  inline static void skip_multi_line_comment(lexer* l) {
    auto& s = l->_stream;

    // Skip `/*`.
    l->next(2);

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }

    while (s.is_valid()) {

      if (*s == '*' && s.is_next_valid() && s.get_next() == '/') {
        l->next(2);

        return;
      }

      if (*s == '\n') {
        l->new_line();
        ++s;
      }
      else {
        l->next();
      }
    }

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }
  }

  inline static void cleanup_block_comment(lexer* l, std::string_view svalue) {
    auto& s = l->_stream;

    zs::string& estr = l->_escaped_string;

    estr.clear();

    const char* ptr = svalue.data();
    const char* end = svalue.data() + svalue.size();
    while (ptr < end) {
      bool was_endl = *ptr == '\n';
      estr.push_back(*ptr++);

      if (was_endl and ptr < end) {
        while (zb::is_one_of(*ptr, ' ', '\t')) {
          ptr++;
        }
      }
    }
  }

  inline static void skip_block_comment(lexer* l) {
    auto& s = l->_stream;

    // Skip ```
    l->next(3);

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }

    size_t count = 0;

    while (s.is_valid()) {
      if (*s == '`') {
        count++;

        if (count == 3) {
          l->next();

          //          size_t sz = std::distance(beg, l->_stream.ptr() - 3);
          //          zb::print("DKLJDKJDKL---", std::string_view(beg, sz));

          ZS_TODO("What's up here?")
          return;
        }
        l->next();
      }
      else if (*s == '\n') {
        count = 0;
        l->new_line();
        ++s;
      }
      else {
        l->next();
        count = 0;
      }
    }

    if (s.is_end()) {
      l->set_token(tok_eof);
      return;
    }
  }

  inline static void parse_identifier(lexer* l) {

    const char* beg = l->_stream.ptr();

    ZS_TODO("Unicode char c > 255 ?");
    while (l->_stream.is_valid() && (zb::is_alphanumeric_or_underscore(*l->_stream) || *l->_stream > 255)) {
      l->next();
    }

    size_t sz = std::distance(beg, l->_stream.ptr());
    std::string_view identifier(beg, sz);

    if (auto it = k_symbol_map.find(zb::eternal::string(identifier)); it != k_symbol_map.end()) {
      l->set_token(it->second);
      l->set_identifier(identifier);
      return;
    }

    l->set_identifier(identifier);
    l->set_token(tok_identifier);
  }

  static inline void cleanup_escapes(zs::string& s) {
    auto it1 = s.begin();
    auto it2 = s.begin();
    auto end = s.end();

    const size_t sz = s.size();
    size_t new_size = sz;

    while (it1 != end) {
      if (*it1 == '\\') {
        new_size--;

        if (++it1 == end) {
          break;
        }

        if (*it1 != '\\') {
          switch (*it1) {
          case 'n':
            *it1 = '\n';
            break;
          case 'r':
            *it1 = '\r';
            break;
          case 't':
            *it1 = '\t';
            break;
          case 'f':
            *it1 = '\f';
            break;
          case 'v':
            *it1 = '\v';
            break;
          case 'b':
            *it1 = '\b';
            break;
          }

          continue;
        }
      }

      *it2++ = *it1++;
    }

    if (new_size != sz) {
      s.resize(new_size);
    }
  }

  inline static void parse_string(lexer* l, uint32_t end_char) {
    zb::utf8_span_stream& s = l->_stream;

    if (!s.is_next_valid()) {
      l->set_token(tok_lex_error);
      return;
    }

    l->next();

    const char* beg = s.ptr();

    bool escaped_found = false;
    bool escaped = false;

    while (s.is_valid()) {

      if (*s == '\n') {
        l->set_token(tok_lex_error);
        return;
      }

      if (!escaped && ('\\' == *s)) {
        escaped_found = true;
        escaped = true;
        l->next();
        continue;
      }
      else if (!escaped) {
        if (end_char == *s) {
          break;
        }
      }
      else if (escaped) {
        escaped = false;
      }
      l->next();
    }

    if (s.is_end()) {
      l->set_token(tok_lex_error);
      return;
    }

    size_t sz = std::distance(beg, l->_stream.ptr());
    std::string_view str(beg, sz);

    if (escaped_found) {
      l->set_token(tok_escaped_string_value);
      l->_escaped_string = str;
      cleanup_escapes(l->_escaped_string);
      l->next();
      return;
    }

    l->set_token(tok_string_value);
    l->_string = str;
    l->next();
    return;
  }

  inline static void parse_multi_line_string(lexer* l, uint32_t end_char) {
    zb::utf8_span_stream& s = l->_stream;

    if (!s.is_next_valid()) {
      l->set_token(tok_lex_error);
      return;
    }

    l->next();

    const char* beg = s.ptr();

    bool escaped_found = false;
    bool escaped = false;

    while (s.is_valid()) {

      if (!escaped && ('\\' == *s)) {
        escaped_found = true;
        escaped = true;
        l->next();
        continue;
      }
      else if (!escaped) {
        if (end_char == *s) {

          if (s.rsize() >= 3 && s[1] == end_char && s[2] == end_char) {
            break;
          }
          l->next();
        }
      }
      else if (escaped) {
        escaped = false;
      }
      l->next();
    }

    if (s.is_end()) {
      l->set_token(tok_lex_error);
      return;
    }

    size_t sz = std::distance(beg, l->_stream.ptr());

    std::string_view str(beg, sz);

    if (escaped_found) {
      l->set_token(tok_escaped_string_value);
      l->_escaped_string = str;
      cleanup_escapes(l->_escaped_string);
      l->next(3);
      return;
    }

    l->set_token(tok_string_value);
    l->_string = str;
    l->next(3);
    return;
  }

  inline static zs::error_result to_integer(std::string_view str_value, int_t& v) noexcept {
    if (auto res = zb::from_chars(str_value, v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  inline static zs::error_result to_real(std::string_view str_value, float_t& v) noexcept {
    if (zb::from_chars_result res = zb::from_chars(str_value, v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  static inline zs::error_result parse_number(lexer* l) {
    zb::utf8_span_stream& s = l->_stream;

    const char* beg = s.ptr();
    bool dot_found = false;
    bool e_found = false;
    bool post_e_sign_found = false;
    bool post_e_digit_found = false;

    // TODO: Fix negative.
    bool is_negative = false;

    // Check for hexadecimal number.
    if (*s == '0' and zb::is_one_of(s.get_next(), 'x', 'X')) {
      l->next(2);
      while (s) {
        const uint32_t t = *s;

        if (zb::is_digit(t) or zb::is_one_of(t, 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F')) {
          l->next();
        }
        else {
          break;
        }
      }

      const size_t sz = std::distance(beg, s.ptr());
      std::string_view svalue(beg, sz);
      l->set_token(tok_integer_value);

      //      l->_int_value = zb::hex_to_int(svalue.substr(2));
      bool overflow = false;
      l->_int_value = zb::hex_to_int<int_t>(svalue.substr(2), &overflow);

      ///////////////////////////////////////////////////////////////
      /// TODO: Check this.
      //      l->next();
      return {};
    }

    // Check for binary number.
    if (*s == '0' and s.get_next() == 'b') {
      l->next(2);
      while (s) {
        const uint32_t t = *s;

        if (zb::is_digit(t) or zb::is_one_of(t, '0', '1')) {
          l->next();
        }
        else {
          break;
        }
      }

      const size_t sz = std::distance(beg, s.ptr());
      std::string_view svalue(beg, sz);
      l->set_token(tok_integer_value);
      l->_int_value = zb::binary_to_int(svalue.substr(2));
      //      l->next();
      return {};
    }

    // Check for octal number.
    if (*s == '0' and s.get_next() == 'h') {
      l->next(2);
      while (s) {
        const uint32_t t = *s;

        if (zb::is_digit(t) or zb::is_one_of(t, '0', '1', '2', '3', '4', '5', '6', '7')) {
          l->next();
        }
        else {
          break;
        }
      }

      const size_t sz = std::distance(beg, s.ptr());
      std::string_view svalue(beg, sz);
      l->set_token(tok_integer_value);

      l->_int_value = zb::octal_to_int(svalue.substr(2));
      //      l->next();
      return {};
    }

    while (s.is_valid()) {
      const uint32_t t = *s;

      if ('.' == t) {

        if (dot_found) {
          // Multiple dots.
          l->set_token(tok_lex_error);
          return zs::error_code::invalid;
        }

        dot_found = true;
        l->next();
        continue;
      }

      // TODO: Fix this.
      if ('-' == t) {
        l->next();
        //        is_negative = true;
        continue;
      }

      // TODO: This never happens?
      else if ('+' == *s) {
        l->next();
        //        is_negative = false;
        continue;
      }

      else if (zb::is_one_of(t, 'e', 'E')) {
        char c = s.get_next();

        if (s.is_next_end()) {
          l->set_token(tok_lex_error);
          return zs::error_code::invalid;
        }
        else if (('+' != c) && ('-' != c) && !zb::is_digit(c)) {
          l->set_token(tok_lex_error);
          return zs::error_code::invalid;
        }

        e_found = true;
        l->next();
        continue;
      }

      else if (e_found && zb::is_sign(*s) && !post_e_digit_found) {
        if (post_e_sign_found) {
          l->set_token(tok_lex_error);
          return zs::error_code::invalid;
        }

        post_e_sign_found = true;
        l->next();
        continue;
      }

      else if (e_found && zb::is_digit(*s)) {
        post_e_digit_found = true;
        l->next();
        continue;
      }

      else if (('.' != t) && !zb::is_digit(*s)) {
        break;
      }

      else {
        l->next();
      }
    }

    const size_t sz = std::distance(beg, s.ptr());
    std::string_view svalue(beg, sz);

    if (is_negative) {
      // TODO: Handle negative.
    }

    if (dot_found || e_found) {
      l->set_token(tok_float_value);

      if (auto err = to_real(svalue, l->_float_value)) {
        l->set_token(tok_lex_error);
        return err;
      }

      return {};
    }

    l->set_token(tok_integer_value);

    if (auto err = to_integer(svalue, l->_int_value)) {
      l->set_token(tok_lex_error);
      return err;
    }

    return {};
  }

  static inline void parse_operator(lexer* l);

  inline static token_type parse_at(lexer* l, uint32_t c1) {
    auto& s = l->_stream;
    const size_t sz = s.rsize();

    static constexpr std::string_view author = "@author";
    static constexpr std::string_view brief = "@brief";
    static constexpr std::string_view version = "@version";
    static constexpr std::string_view date = "@date";
    static constexpr std::string_view copyright = "@copyright";
    static constexpr std::string_view counter = "@counter";
    static constexpr std::string_view uuid = "@uuid";
    static constexpr std::string_view keyword = "@keyword";
    static constexpr std::string_view macro = "@macro";
    static constexpr std::string_view expr = "@expr";

#define KKKKK(sname)                                                            \
  else if (std::string_view(s.ptr(), zb::minimum(sname.size(), sz)) == sname) { \
    l->set_token(tok_##sname);                                                  \
    l->next(sname.size());                                                      \
    return l->_current_token;                                                   \
  }

    // @module
    if (sz >= 7 and std::string_view(s.ptr(), 7) == "@module") {
      l->set_token(tok_module);
      l->next(7);
      return l->_current_token;
    }
    // @str
    else if (sz >= 4 and std::string_view(s.ptr(), 4) == "@str") {
      l->set_token(tok_stringify);
      l->next(4);
      return l->_current_token;
    }
    // @include
    else if (sz >= 8 and std::string_view(s.ptr(), 8) == "@include") {
      l->set_token(tok_include);
      l->next(8);
      return l->_current_token;
    }
    // @import
    else if (sz >= 7 and std::string_view(s.ptr(), 7) == "@import") {
      l->set_token(tok_import);
      l->next(7);
      return l->_current_token;
    }
    KKKKK(author)
    KKKKK(brief)
    KKKKK(copyright)
    KKKKK(version)
    KKKKK(date)
    KKKKK(counter)
    KKKKK(uuid)
    KKKKK(keyword)
    KKKKK(macro)
    KKKKK(expr)

    if (c1 == '@') {
      l->set_token(tok_double_at);
      l->next(2);
      return l->_current_token;
    }

    l->set_token(tok_at);
    l->next();
    return l->_current_token;
  }
};

lexer::lexer(zs::engine* eng)
    : engine_holder(eng)
    , _escaped_string(zs::allocator<char>(eng)) {}

lexer::lexer(zs::engine* eng, std::string_view code)
    : lexer(eng) {

  init(code);
}

lexer::~lexer() {}

void lexer::init(std::string_view code) {
  _stream = zb::utf8_span_stream(code);
  _current_token = _last_token = tok_none;
  _current_line = _current_column = _last_token_line = 1;
  _last_line_info = { 0, 0 };
  _identifier = _string = {};
  _escaped_string.clear();
  _int_value = _float_value = 0;
  _is_string_view_identifier = _export_block_comments = false;
}

zs::object lexer::get_value() const {
  switch (_current_token) {
  case tok_none:
    return zs::object::create_none();
  case tok_null:
    return nullptr;
  case tok_char_value:
  case tok_integer_value:
    return _int_value;
  case tok_float_value:
    return _float_value;
  case tok_string_value:
    return zs::_s(_engine, _string);
  case tok_escaped_string_value:
    return zs::_s(_engine, _escaped_string);
  case tok_false:
    return zs::object(false);
  case tok_true:
    return zs::object(true);
  case tok_identifier:
    return _is_string_view_identifier ? zs::_sv(_identifier) : zs::object(_engine, _identifier);
  }

  return {};
}

zs::object lexer::get_debug_value() const {
  switch (_current_token) {
  case tok_none:
    return zs::object::create_none();
  case tok_null:
    return nullptr;
  case tok_integer_value:
    return _int_value;
  case tok_float_value:
    return _float_value;
  case tok_string_value:
    return zs::object(_engine, _string);
  case tok_escaped_string_value:
    return zs::object(_engine, _escaped_string);
  case tok_false:
    return zs::object(false);
  case tok_true:
    return zs::object(true);
  case tok_identifier:
    return _is_string_view_identifier ? zs::_sv(_identifier) : zs::object(_engine, _identifier);
  }

  return zs::object();
}

namespace lex_detail {

  inline constexpr std::array k_meta_hashs = { zb::eternal::impl::str_hash("call"), //
    zb::eternal::impl::str_hash("tostring"), //
    zb::eternal::impl::str_hash("get"), //
    zb::eternal::impl::str_hash("set"), //
    zb::eternal::impl::str_hash("next"), //
    zb::eternal::impl::str_hash("minus"), //
    zb::eternal::impl::str_hash("delete_slot"), //
    zb::eternal::impl::str_hash("copy") };

  inline constexpr std::array k_meta_strings = { "call", //
    "tostring", //
    "get", //
    "set", //
    "next", //
    "minus", //
    "delete_slot", //
    "copy" };

  inline constexpr std::array k_metas = { zs::constants::k_mt_call_string, //
    zs::constants::k_mt_tostring_string, //
    zs::constants::k_mt_get_string, //
    zs::constants::k_mt_set_string, //
    zs::constants::k_mt_next_string, //
    zs::constants::k_mt_unary_minus_string, //
    zs::constants::k_mt_delete_slot_string, //
    zs::constants::k_mt_copy_string };
} // namespace lex_detail.

void lexer::helper::parse_operator(lexer* l) {
  zb::utf8_span_stream& s = l->_stream;

  l->skip_white_spaces();

  if (*s != '(') {
    l->set_token(tok_lex_error);

    return;
  }

  // Skip `(`.
  l->next();

  token_type t = l->lex();

  std::string_view operator_value;
  switch (t) {
  case tok_typeof:
    operator_value = zs::constants::k_mt_typeof_string;
    break;
  case tok_eq_eq:
    operator_value = zs::constants::k_mt_compare_string;
    break;
  case tok_add:
    operator_value = zs::constants::k_mt_add_string;
    break;
  case tok_minus:
    operator_value = zs::constants::k_mt_sub_string;
    break;
  case tok_mul:
    operator_value = zs::constants::k_mt_mul_string;
    break;
  case tok_div:
    operator_value = zs::constants::k_mt_div_string;
    break;
  case tok_exp:
    operator_value = zs::constants::k_mt_exp_string;
    break;
  case tok_mod:
    operator_value = zs::constants::k_mt_mod_string;
    break;
  case tok_lshift:
    operator_value = zs::constants::k_mt_lshift_string;
  case tok_rshift:
    operator_value = zs::constants::k_mt_rshift_string;
    break;
  case tok_bitwise_or:
    operator_value = zs::constants::k_mt_bw_or_string;
    break;
  case tok_bitwise_and:
    operator_value = zs::constants::k_mt_bw_and_string;
    break;
  case tok_xor:
    operator_value = zs::constants::k_mt_bw_xor_string;
    break;
  case tok_add_eq:
    operator_value = zs::constants::k_mt_add_eq_string;
    break;
  case tok_minus_eq:
    operator_value = zs::constants::k_mt_sub_eq_string;
    break;
  case tok_mul_eq:
    operator_value = zs::constants::k_mt_mul_eq_string;
    break;
  case tok_div_eq:
    operator_value = zs::constants::k_mt_div_eq_string;
    break;
  case tok_exp_eq:
    operator_value = zs::constants::k_mt_exp_eq_string;
    break;
  case tok_mod_eq:
    operator_value = zs::constants::k_mt_mod_eq_string;
    break;
  case tok_lshift_eq:
    operator_value = zs::constants::k_mt_lshift_eq_string;
    break;
  case tok_rshift_eq:
    operator_value = zs::constants::k_mt_rshift_eq_string;
    break;
  case tok_bitwise_or_eq:
    operator_value = zs::constants::k_mt_bw_or_eq_string;
    break;
  case tok_bitwise_and_eq:
    operator_value = zs::constants::k_mt_bw_and_eq_string;
    break;
  case tok_xor_eq:
    operator_value = zs::constants::k_mt_bw_xor_eq_string;
    break;

    // TODO: What about mt_incr and mt_decr.
  case tok_incr:
    operator_value = zs::constants::k_mt_pre_incr_string;
    break;
  case tok_decr:
    operator_value = zs::constants::k_mt_pre_decr_string;
    break;

  case tok_compare:
    operator_value = zs::constants::k_mt_compare_string;
    break;

  case tok_identifier: {
    {
      using namespace lex_detail;

      const size_t id_hash = zb::eternal::impl::str_hash(l->_identifier);
      if (auto it = std::find_if(
              k_meta_hashs.begin(), k_meta_hashs.end(), [&](const auto& sh) { return sh == id_hash; });
          it != k_meta_hashs.end()) {
        size_t index = std::distance(k_meta_hashs.begin(), it);

        if (k_meta_strings[index] == l->_identifier) {
          operator_value = k_metas[index];
          break;
        }
      }

      zb::print("Invalid operator");
      l->set_token(tok_lex_error);
      return;
    }
  }

  default:
    zb::print("Invalid operator");
    l->set_token(tok_lex_error);
    return;
  }

  l->skip_white_spaces();

  if (*s != ')') {
    l->set_token(tok_lex_error);
    return;
  }

  // Skip `)`.
  l->next();
  l->set_identifier(operator_value, true);

  // Can't use `set_token()` here, we don't want `_last_token` to change.
  l->_current_token = tok_identifier;
  return;
}

token_type lexer::lex(bool keep_endl) {

  if (!_stream.is_valid()) {
    return tok_eof;
  }

  _last_token_line = _current_line;

  while (_stream.is_valid()) {
    uint32_t c = *_stream;
    auto [c1, c2] = _stream.safe_get_next_2();

    switch (c) {
    case '\t':
    case '\r':
    case ' ':
      next();
      continue;

    case '\n':
      new_line();
      set_token(tok_endl);
      ++_stream;

      if (keep_endl) {
        return _current_token;
      }
      continue;

    case '`': {
      const size_t sz = _stream.rsize();
      if (c1 == '`' && c2 == '`') {

        const char* beg = _stream.ptr() + 3;
        helper::skip_block_comment(this);

        if (_export_block_comments) {
          const size_t sz = std::distance(beg, _stream.ptr() - 3);
          std::string_view svalue(beg, sz);

          set_token(tok_doc_block);
          helper::cleanup_block_comment(this, zb::strip_all(svalue));
          next();
          return _current_token;
        }

        continue;
      }

      set_token(tok_lex_error);
      return _current_token;
    }

    case '#':
      if (c1 == '!') {
        helper::skip_hashtag_comment(this);
        continue;
      }

      return set_token_and_next(tok_hastag);

    case '{': {
      if (c1 == '/') {
        return set_token_and_next(tok_block_begin, 2);
      }

      return set_token_and_next(tok_lcrlbracket);
    }

    case '}':
      return set_token_and_next(tok_rcrlbracket);

    case '[':
      return set_token_and_next(tok_lsqrbracket);

    case ']':
      return set_token_and_next(tok_rsqrbracket);

    case '(':
      return set_token_and_next(tok_lbracket);

    case ')':
      return set_token_and_next(tok_rbracket);

    case ';':
      return set_token_and_next(tok_semi_colon);

    case '?': {
      if (c1 == '?' and c2 == '?') {
        return set_token_and_next(tok_triple_question_mark, 3);
      }

      if (c1 == ':') {
        return set_token_and_next(tok_double_question_mark, 2);
      }

      return set_token_and_next(tok_question_mark);
    }

    case ',':
      return set_token_and_next(tok_comma);

    case '$':
      if (c1 == '$') {
        return set_token_and_next(tok_double_dollar, 2);
      }

      return set_token_and_next(tok_dollar);

    case '@':
      return helper::parse_at(this, c1);

    case '/': {
      if (_stream.is_next_valid()) {
        if (_export_block_comments and _stream.rsize() >= 5 && c1 == '/' && c2 == '/' and _stream[3] == ' '
            and _stream[4] == '@') {
          const char* beg = _stream.ptr() + 5;
          helper::skip_triple_dash_doc_block_comment(this);

          set_token(tok_doc_block);
          return _current_token;
        }
        else if (c1 == '/') {
          helper::skip_single_line_comment(this);
          continue;
        }
        else if (c1 == '*') {
          helper::skip_multi_line_comment(this);
          continue;
        }
        else if (c1 == '=') {
          return set_token_and_next(tok_div_eq, 2);
        }
        else if (c1 == '>') {
          return set_token_and_next(tok_attribute_end, 2);
        }
        else if (c1 == '}') {
          return set_token_and_next(tok_block_end, 2);
        }
      }

      return set_token_and_next(tok_div);
    }

    case '=': {
      const size_t sz = _stream.rsize();

      if (c1 == '=' && c2 == '=') {
        return set_token_and_next(tok_three_way_compare, 3);
      }

      if (c1 == '=') {
        return set_token_and_next(tok_eq_eq, 2);
      }

      if (c1 == '>') {
        return set_token_and_next(tok_right_arrow, 2);
      }

      return set_token_and_next(tok_eq);
    }

    case ':': {
      if (c1 == ':') {
        return set_token_and_next(tok_double_colon, 2);
      }

      return set_token_and_next(tok_colon);
    }

    case '!': {

      switch (c1) {
      case '=':
        return set_token_and_next(tok_not_eq, 2);

      case '>':
        return set_token_and_next(tok_double_question_mark, 2);
      }

      return set_token_and_next(tok_not);
    }

      // %
      // %=
    case '%': {
      if (c1 == '=') {
        return set_token_and_next(tok_mod_eq, 2);
      }

      return set_token_and_next(tok_mod);
    }

      // ^
      // ^=
      // ^^
      // ^^=
    case '^': {
      if (c1 == '=') {
        return set_token_and_next(tok_exp_eq, 2);
      }

      if (c1 == '^' and c2 == '=') {
        return set_token_and_next(tok_xor_eq, 3);
      }

      if (c1 == '^') {
        return set_token_and_next(tok_xor, 2);
      }

      return set_token_and_next(tok_exp);
    }

    case '~': {
      if (c1 == '=') {
        return set_token_and_next(tok_inv_eq, 2);
      }

      return set_token_and_next(tok_inv);
    }

    case '|': {

      if (c1 == '=') {
        return set_token_and_next(tok_bitwise_or_eq, 2);
      }

      if (c1 == '|' && c2 == '|') {
        return set_token_and_next(tok_triple_or, 3);
      }

      if (c1 == '|') {
        return set_token_and_next(tok_or, 2);
      }

      return set_token_and_next(tok_bitwise_or);
    }

    case '&': {
      switch (c1) {
      case '=':
        return set_token_and_next(tok_bitwise_and_eq, 2);

      case '&':
        return set_token_and_next(tok_and, 2);
      }

      return set_token_and_next(tok_bitwise_and);
    }

    case '.': {
      if (zb::is_digit(c1)) {
        helper::parse_number(this);
        return _current_token;
      }

      if (c1 == '.' && c2 == '.') {
        return set_token_and_next(tok_triple_dots, 3);
      }

      return set_token_and_next(tok_dot);
    }

    case '+': {
      switch (c1) {
      case '=':
        return set_token_and_next(tok_add_eq, 2);

      case '+':
        return set_token_and_next(tok_incr, 2);
      }

      return set_token_and_next(tok_add);
    }

    case '-': {

      switch (c1) {
      case '=':
        return set_token_and_next(tok_minus_eq, 2);

      case '-':
        return set_token_and_next(tok_decr, 2);
      }

      return set_token_and_next(tok_minus);
    }

    // *
    // *=
    case '*': {
      if (c1 == '=') {
        return set_token_and_next(tok_mul_eq, 2);
      }

      return set_token_and_next(tok_mul);
    }

    case '<': {
      const size_t sz = _stream.rsize();

      if (sz >= 4) {
        // <-->
        if (c1 == '-' && c2 == '-' && _stream[3] == '>') {
          return set_token_and_next(tok_double_arrow, 4);
        }
        // <==>
        else if (c1 == '=' && c2 == '=' && _stream[3] == '>') {
          return set_token_and_next(tok_double_arrow_eq, 4);
        }
      }

      // <<=
      if (sz >= 3) {
        if (c1 == '<' && c2 == '=') {
          return set_token_and_next(tok_lshift_eq, 3);
        }
        else if (c1 == '=' && c2 == '>') {
          return set_token_and_next(tok_compare, 3);
        }
      }

      if (sz >= 2) {
        // <=
        if (c1 == '=') {
          return set_token_and_next(tok_lt_eq, 2);
        }
        // <-
        else if (c1 == '-') {
          return set_token_and_next(tok_left_arrow, 2);
        }
        // <<
        else if (c1 == '<') {
          return set_token_and_next(tok_lshift, 2);
        }

        // </
        else if (c1 == '/') {
          return set_token_and_next(tok_attribute_begin, 2);
        }
      }

      return set_token_and_next(tok_lt);
    }

    case '>': {
      const size_t sz = _stream.rsize();

      // >>=
      if (c1 == '>' && c2 == '=') {
        return set_token_and_next(tok_rshift_eq, 3);
      }

      // >=
      if (c1 == '=') {
        return set_token_and_next(tok_gt_eq, 2);
      }

      // >>
      if (c1 == '>') {
        return set_token_and_next(tok_rshift, 2);
      }

      return set_token_and_next(tok_gt);
    }

    // String begin.
    case '"': {

      // Check for triple double quotes
      // Example: """content"""
      if (c1 == '"' && c2 == '"') {
        next(2);
        helper::parse_multi_line_string(this, '"');
        return _current_token;
      }

      // Regular string.
      helper::parse_string(this, '"');
      return _current_token;
    }

    // Char or string begin.
    case '\'': {

      // Check for triple quotes.
      // Example: '''content'''
      if (c1 == '\'' && c2 == '\'') {
        next(2);
        helper::parse_multi_line_string(this, '\'');
        return _current_token;
      }

      // This should be a single quoted char value.
      // Example: 'x'.
      if (!_stream.is_next_valid()) {
        set_token(tok_lex_error);
        return _current_token;
      }

      // We are dealing with utf8 chars.
      next();
      uint32_t value = *_stream;

      if (value == '\\') {
        switch (_stream.get_next()) {
        case 'n':
          next();
          value = '\n';
          break;

        case 't':
          next();
          value = '\t';
          break;

        case 'r':
          next();
          value = '\r';
          break;

        case 'v':
          next();
          value = '\v';
          break;

        case '\'':
          next();
          value = '\'';
          break;

        case '\\':
          next();
          value = '\\';
          break;

        case 'U': {
          next();
          next();

          zb::utf8_span_stream& s = _stream;

          const char* beg = s.ptr();

          while (s) {
            const uint32_t t = *s;

            if (zb::is_digit(t)
                or zb::is_one_of(t, 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F')) {

              if (_stream.get_next() == '\'') {
                break;
              }
              next();
            }
            else {
              break;
            }
          }

          std::string_view svalue(beg, std::distance(beg, s.ptr() + 1));
          bool overflow = false;
          value = (uint32_t)zb::hex_to_int<int_t>(svalue, &overflow);

          break;
        }
        }
      }

      // Next char should be the closing single quote.
      if (!_stream.is_next_valid() || _stream.get_next() != '\'') {
        set_token(tok_lex_error);
        return _current_token;
      }

      // Skipping the char value.
      next();

      // Get the char value.
      _int_value = value;
      set_token(tok_char_value);

      // Skipping the closing single quote.
      next();

      return _current_token;
    }

    default:
      if (zb::is_digit(c)) {
        helper::parse_number(this);
        return _current_token;
      }

      else if (zb::is_letter_or_underscore(c) or c > 255) {
        ZS_TODO("Unicode char c > 255 ");

        helper::parse_identifier(this);

        const uint32_t c = _stream.safe_get();

        switch (_current_token) {
        case tok_string:
          switch (c) {
          case '(':
            set_identifier("__tostring");
            ZBASE_NO_BREAK;
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
          break;

        case tok_int:
          switch (c) {
          case '(':
            set_identifier("__toint");
            ZBASE_NO_BREAK;
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
          break;

        case tok_float:
          switch (c) {
          case '(':
            set_identifier("__tofloat");
            ZBASE_NO_BREAK;
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
          break;

        case tok_array: // set_identifier("array");

          // Can't use `set_token()` here, we don't want `_last_token` to change.
          //            return _current_token = tok_identifier;
          switch (c) {
          case '<':
            ZBASE_NO_BREAK;
          case '(':
            //            set_identifier("__create_array");
            ZBASE_NO_BREAK;
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }

          //        case tok_struct:
          //          switch (c) {
          //          case '(':
          //            set_identifier("create_struct");
          //            ZBASE_NO_BREAK;
          //          case '.':
          //            // Can't use `set_token()` here, we don't want `_last_token` to change.
          //            return _current_token = tok_identifier;
          //          }
          break;
        }

        if (_identifier == "operator") {
          helper::parse_operator(this);
        }

        return _current_token;
      }

      return tok_lex_error;
    }
  }

  return tok_eof;
}

bool lexer::lex_compare(std::span<const token_type> buffer) {

  for (token_type t : buffer) {
    if (lex() != t or zb::is_one_of(_current_token, tok_eof, tok_lex_error)) {
      return false;
    }
  }
  return true;
}

token_type lexer::peek(bool keep_endl) const {
  lexer l(*this);
  return l.lex(keep_endl);
}

zs::error_result lexer::peek(std::span<token_type>& buffer) const {
  lexer l(*this);

  auto it = buffer.begin();
  const auto end = buffer.end();

  while (it != end and !zb::is_one_of(*it++ = l.lex(), tok_eof, tok_lex_error)) {
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

bool lexer::peek_compare(std::span<const token_type> buffer) {
  lexer l(*this);
  return l.lex_compare(buffer);
}

bool lexer::is_right_arrow_function_call() const {
  ZS_ASSERT(_current_token == tok_lbracket);

  lexer l(*this);
  token_type tok = tok_none;
  int_t count = 1;
  while (!zb::is_one_of(tok = l.lex(), tok_eof, tok_lex_error)) {

    if (tok == tok_rbracket) {
      if (--count == 0) {

        return l.lex() == tok_right_arrow;
      }
    }
    else if (tok == tok_lbracket) {
      count++;
    }
  }

  return false;
}
bool lexer::is_template_function_call() const {
  ZS_ASSERT(_current_token == tok_lt);

  if (!zb::is_one_of(_last_token, ZS_TOK(identifier, rsqrbracket, rbracket))) {
    return false;
  }

  lexer l(*this);

  zb_loop() {
    switch (l.lex()) {
    case tok_none:
    case tok_null:
    case tok_var:
    case tok_int:
    case tok_float:
    case tok_number:
    case tok_bool:
    case tok_table:
    case tok_array:
    case tok_string:
    case tok_char:
    case tok_true:
    case tok_false:
    case tok_and:
    case tok_or:
    case tok_global:
    case tok_function:
    case tok_typeof:
    case tok_typeid:
    case tok_not:
    case tok_xor:
    case tok_integer_value:
    case tok_float_value:
    case tok_string_value:
    case tok_escaped_string_value:
    case tok_identifier:
    case tok_file:
    case tok_line:
    case tok_comma:
    case tok_dot:
    case tok_triple_dots:
    case tok_colon:
    case tok_double_colon:
    case tok_lbracket:
    case tok_rbracket:
    case tok_lsqrbracket:
    case tok_rsqrbracket:
    case tok_lcrlbracket:
    case tok_rcrlbracket:
    case tok_mod:
    case tok_add:
    case tok_minus:
    case tok_mul:
    case tok_div:
    case tok_exp:
    case tok_bitwise_or:
    case tok_bitwise_and:
    case tok_inv:
    case tok_incr:
    case tok_decr:
    case tok_at:
    case tok_double_at:
    case tok_dollar:
    case tok_double_dollar:
    case tok_endl:
      break;

    case tok_gt:
      return l.lex() == tok_lbracket;

    default:
      return false;
    }
  }

  return false;
}

zs::error_result lexer::lex(std::span<token_type>& buffer) {
  auto it = buffer.begin();
  const auto end = buffer.end();

  while (it != end and !zb::is_one_of(*it++ = lex(), tok_eof, tok_lex_error)) {
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

zs::error_result lexer::lex(std::span<token_type>& buffer, token_type tok) {
  auto it = buffer.begin();
  const auto end = buffer.end();

  while (it != end and !zb::is_one_of(*it++ = lex(), tok_eof, tok_lex_error, tok)) {
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

zs::error_result lexer::lex_rbracket(std::span<token_type>& buffer) {
  auto it = buffer.begin();
  const auto end = buffer.end();

  int_t count = 1;
  while (it != end and !zb::is_one_of(*it++ = lex(), tok_eof, tok_lex_error)) {

    if (*it == tok_rbracket) {
      if (!--count) {
        break;
      }
    }
    else if (*it == tok_lbracket) {

      count++;
    }
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

zs::error_result lexer::lex_to_rctrlbracket() {
  int_t count = 1;
  const char* beg = _stream.ptr();
  token_type tok = tok_none;
  while (!zb::is_one_of(tok = lex(), tok_eof, tok_lex_error)) {

    if (tok == tok_rcrlbracket) {
      if (!--count) {
        break;
      }
    }
    else if (tok == tok_lcrlbracket) {
      count++;
    }
  }

  if (zb::is_one_of(tok, tok_eof, tok_lex_error)) {
    return zs::errc::not_found;
  }

  return {};
}

zs::error_result lexer::lex_for_auto(std::span<token_type>& buffer) {
  auto it = buffer.begin();
  const auto end = buffer.end();

  ZS_ASSERT(_current_token == tok_lbracket);
  *it++ = _current_token;

  int_t count = 1;
  while (it != end) {
    token_type t = lex();
    *it++ = t;

    if (zb::is_one_of(t, tok_eof, tok_lex_error)) {
      break;
    }

    if (t == tok_semi_colon) {
      buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
      return zs::error_code::not_a_for_colon;
    }

    if (t == tok_rbracket) {
      if (--count == 0) {

        buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
        return {};
      }
    }
    else if (t == tok_lbracket) {
      count++;
    }
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));

  return zs::error_code::not_a_for_colon;
}

zs::error_result lexer::lex_to(token_type tok) {

  while (!zb::is_one_of(_current_token, tok_eof, tok_lex_error)) {
    lex();
    if (_current_token == tok) {
      return {};
    }
  }

  return zs::error_code::not_found;
}

zs::error_result lexer::lex_to(token_type tok, size_t nmax) {
  size_t count = 0;
  while (!zb::is_one_of(_current_token, tok_eof, tok_lex_error)) {

    lex();
    if (_current_token == tok) {
      return {};
    }

    if (count >= nmax) {
      return zs::error_code::not_found;
    }
  }

  return zs::error_code::not_found;
}
} // namespace zs.
ZBASE_PRAGMA_POP()
