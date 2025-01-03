#include "zlexer.h"
#include <zscript/base/strings/charconv.h>
#include <zscript/base/container/constexpr_map.h>
#include <zscript/base/strings/parse_utils.h>

ZBASE_PRAGMA_PUSH_NO_MISSING_SWITCH_WARNING()

namespace zs {

struct lexer::helper {

  static constexpr const auto k_symbol_map = zb::eternal::hash_map<zb::eternal::string, zs::token_type>(
      { { "__FILE__", zs::token_type::tok_file }, { "__LINE__", zs::token_type::tok_line },
          { "__THIS_LINE__", zs::token_type::tok_line_str }, { "null", zs::token_type::tok_null },
          { "var", zs::token_type::tok_var }, { "int", zs::token_type::tok_int },
          { "atom", zs::token_type::tok_atom }, { "float", zs::token_type::tok_float },
          { "number", zs::token_type::tok_number }, { "bool", zs::token_type::tok_bool },
          { "table", zs::token_type::tok_table }, { "array", zs::token_type::tok_array },
          { "string", zs::token_type::tok_string }, { "char", zs::token_type::tok_char },
          { "auto", zs::token_type::tok_auto }, { "true", zs::token_type::tok_true },
          { "false", zs::token_type::tok_false }, { "return", zs::token_type::tok_return },
          { "struct", zs::token_type::tok_struct }, { "and", zs::token_type::tok_and },
          { "or", zs::token_type::tok_or }, { "if", zs::token_type::tok_if },
          { "else", zs::token_type::tok_else }, { "for", zs::token_type::tok_for },
          { "do", zs::token_type::tok_do }, { "while", zs::token_type::tok_while },
          { "switch", zs::token_type::tok_switch }, { "case", zs::token_type::tok_case },
          { "break", zs::token_type::tok_break }, { "default", zs::token_type::tok_default },
          { "continue", zs::token_type::tok_continue }, { "this", zs::token_type::tok_this },
          { "global", zs::token_type::tok_global }, { "const", zs::token_type::tok_const },
          { "static", zs::token_type::tok_static }, { "function", zs::token_type::tok_function },
          { "private", zs::token_type::tok_private }, { "typeof", zs::token_type::tok_typeof },
          { "typeid", zs::token_type::tok_typeid }, { "constructor", zs::token_type::tok_constructor },

          { "xor", zs::token_type::tok_bitwise_xor }, { "not", zs::token_type::tok_not },
          { "none", zs::token_type::tok_none } });

  inline static void skip_white_spaces(lexer* l) noexcept {
    l->_last_line_info = l->get_line_info();

    while (zb::is_one_of(*l->_stream, ' ', '\t')) {
      l->_current_column++;
      l->_stream.incr();
    }
  }

  inline static void new_line(lexer* l) noexcept {
    l->_last_line_info = l->get_line_info();
    l->_last_endl_ptr = l->_stream.ptr();
    l->_current_column = 1;
    l->_current_line++;
  }

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
      new_line(l);
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
      new_line(l);
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
        new_line(l);

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
      new_line(l);
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
        new_line(l);
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
    //    auto& s = l->_stream;

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
        new_line(l);
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
};

lexer::lexer(zs::engine* eng) noexcept
    : engine_holder(eng)
    , _escaped_string(zs::allocator<char>(eng)) {}

lexer::lexer(zs::engine* eng, std::string_view code) noexcept
    : lexer(eng) {

  init(code);
}

lexer::~lexer() noexcept {}

void lexer::init(std::string_view code) noexcept {
  _stream = zb::utf8_span_stream(code);
  _current_token = _last_token = tok_none;
  _current_line = _current_column = _last_token_line = 1;
  _last_line_info = { 0, 0 };
  _identifier = _string = {};
  _escaped_string.clear();
  _int_value = _float_value = 0;
  _is_string_view_identifier = false;
}

zs::object lexer::get_value() const noexcept {
  switch (_current_token) {
  case tok_none:
    return object(zs::none{});
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

zs::object lexer::get_debug_value() const noexcept {
  switch (_current_token) {
  case tok_none:
    return object(zs::none{});
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

  inline constexpr std::array k_meta_hashs = { //
    zb::eternal::impl::str_hash("typeof"), //
    zb::eternal::impl::str_hash("call"), //
    zb::eternal::impl::str_hash("tostring"), //
    zb::eternal::impl::str_hash("get"), //
    zb::eternal::impl::str_hash("set"), //
    zb::eternal::impl::str_hash("minus"), //
    zb::eternal::impl::str_hash("copy")
  };

  inline constexpr std::array k_meta_strings = { //
    "typeof", //
    "call", //
    "tostring", //
    "get", //
    "set", //
    "minus", //
    "copy"
  };

  inline constexpr std::array k_metas = { //
    zs::constants::k_mt_call_string, //
    zs::constants::k_mt_typeof_string, //
    zs::constants::k_mt_tostring_string, //
    zs::constants::k_mt_get_string, //
    zs::constants::k_mt_set_string, //
    zs::constants::k_mt_unary_minus_string, //
    zs::constants::k_mt_copy_string
  };
} // namespace lex_detail.

void lexer::helper::parse_operator(lexer* l) {
  zb::utf8_span_stream& s = l->_stream;

  skip_white_spaces(l);

  bool start_with_parent = *s == '(';

  if (start_with_parent) {
    // Skip `(`.
    l->next();
  }

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
  case tok_sub:
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
    break;
  case tok_rshift:
    operator_value = zs::constants::k_mt_rshift_string;
    break;
  case tok_bitwise_or:
    operator_value = zs::constants::k_mt_bw_or_string;
    break;
  case tok_bitwise_and:
    operator_value = zs::constants::k_mt_bw_and_string;
    break;
  case tok_bitwise_xor:
    operator_value = zs::constants::k_mt_bw_xor_string;
    break;
  case tok_add_eq:
    operator_value = zs::constants::k_mt_add_eq_string;
    break;
  case tok_sub_eq:
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
  case tok_bitwise_xor_eq:
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

  skip_white_spaces(l);

  if (start_with_parent and *s != ')') {
    l->set_token(tok_lex_error);
    return;
  }

  // Skip `)`.
  if (start_with_parent) {
    l->next();
  }
  l->set_identifier(operator_value, true);

  // Can't use `set_token()` here, we don't want `_last_token` to change.
  l->_current_token = tok_identifier;
  return;
}

token_type lexer::lex(bool keep_endl) noexcept {

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
      helper::new_line(this);
      set_token(tok_endl);
      ++_stream;

      if (keep_endl) {
        return _current_token;
      }
      continue;

    case '`': {
      //      const size_t sz = _stream.rsize();
      if (c1 == '`' && c2 == '`') {
        helper::skip_block_comment(this);
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

    case '{':
      return set_token_and_next(tok_lcrlbracket);

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

      return set_token_and_next(tok_question_mark);
    }

    case ',':
      return set_token_and_next(tok_comma);

    case '$':
      return set_token_and_next(tok_dollar);

    case '/': {
      if (_stream.is_next_valid()) {

        if (c1 == '/') {
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
      }

      return set_token_and_next(tok_div);
    }

    case '=': {
      //      const size_t sz = _stream.rsize();

      if (c1 == '=' && c2 == '=') {
        return set_token_and_next(tok_strict_eq, 3);
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
        return set_token_and_next(tok_if_not, 2);
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
        return set_token_and_next(tok_bitwise_xor_eq, 3);
      }

      if (c1 == '^') {
        return set_token_and_next(tok_bitwise_xor, 2);
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
        return set_token_and_next(tok_sub_eq, 2);

      case '-':
        return set_token_and_next(tok_decr, 2);
      }

      return set_token_and_next(tok_sub);
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
        // <<
        else if (c1 == '<') {
          return set_token_and_next(tok_lshift, 2);
        }
      }

      return set_token_and_next(tok_lt);
    }

    case '>': {
      //      const size_t sz = _stream.rsize();

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
            set_identifier("__to_string");
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
          break;

        case tok_int:
          switch (c) {
          case '(':
            set_identifier("__to_int");
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
          break;

        case tok_float:
          switch (c) {
          case '(':
            set_identifier("__to_float");
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
          break;

        case tok_array:

          switch (c) {
          case '<':
          case '(':
          case '.':
            // Can't use `set_token()` here, we don't want `_last_token` to change.
            return _current_token = tok_identifier;
          }
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

bool lexer::lex_compare(std::span<const token_type> buffer) noexcept {

  for (token_type t : buffer) {
    if (lex() != t or zb::is_one_of(_current_token, tok_eof, tok_lex_error)) {
      return false;
    }
  }
  return true;
}

token_type lexer::peek(bool keep_endl) const noexcept {
  lexer l(*this);
  return l.lex(keep_endl);
}

zs::error_result lexer::peek(std::span<token_type>& buffer) const noexcept {
  lexer l(*this);

  auto it = buffer.begin();
  const auto end = buffer.end();

  while (it != end and !zb::is_one_of(*it++ = l.lex(), tok_eof, tok_lex_error)) {
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

bool lexer::peek_compare(std::span<const token_type> buffer) const noexcept {
  lexer l(*this);
  return l.lex_compare(buffer);
}

bool lexer::is_right_arrow_function() const noexcept {
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

zs::error_result lexer::lex(std::span<token_type>& buffer) noexcept {
  auto it = buffer.begin();
  const auto end = buffer.end();

  while (it != end and !zb::is_one_of(*it++ = lex(), tok_eof, tok_lex_error)) {
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

zs::error_result lexer::lex(std::span<token_type>& buffer, token_type tok) noexcept {
  auto it = buffer.begin();
  const auto end = buffer.end();

  while (it != end and !zb::is_one_of(*it++ = lex(), tok_eof, tok_lex_error, tok)) {
  }

  buffer = buffer.subspan(0, std::distance(buffer.begin(), it));
  return {};
}

zs::error_result lexer::lex_rbracket(std::span<token_type>& buffer) noexcept {
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

zs::error_result lexer::lex_to_rctrlbracket() noexcept {
  int_t count = 1;
  //  const char* beg = _stream.ptr();
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

zs::error_result lexer::lex_for_auto(std::span<token_type>& buffer) noexcept {
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

zs::error_result lexer::lex_to(token_type tok) noexcept {

  while (!zb::is_one_of(_current_token, tok_eof, tok_lex_error)) {
    lex();
    if (_current_token == tok) {
      return {};
    }
  }

  return zs::error_code::not_found;
}

zs::error_result lexer::lex_to(token_type tok, size_t nmax) noexcept {
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
