#include "zjson_lexer.h"
#include <zscript/base/strings/charconv.h>
#include <zscript/base/container/constexpr_map.h>
#include <zscript/base/strings/parse_utils.h>

ZBASE_PRAGMA_PUSH_NO_MISSING_SWITCH_WARNING()

namespace zs {

struct json_lexer::helper {

  static constexpr const auto k_symbol_map = zb::eternal::hash_map<zb::eternal::string, zs::json_token_type>({
      { "none", zs::json_token_type::tok_none },
      { "null", zs::json_token_type::tok_null },
      { "true", zs::json_token_type::tok_true },
      { "false", zs::json_token_type::tok_false },
  });

  inline static void skip_single_line_comment(json_lexer* l) {
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

  inline static void skip_multi_line_comment(json_lexer* l) {
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

  inline static void skip_block_comment(json_lexer* l) {
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

          ZS_TODO("Fix this")
          //          size_t sz = std::distance(beg, l->_stream.ptr() - 3);
          //          zb::print("DKLJDKJDKL---", std::string_view(beg, sz));

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

  inline static void parse_identifier(json_lexer* l) {

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

  inline static void parse_string(json_lexer* l, uint32_t end_char) {
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

  inline static void parse_multi_line_string(json_lexer* l, uint32_t end_char) {
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
    ZS_TODO("Make this faster.");

    const size_t sz = str_value.size();
    char* svalue = (char*)zb_alloca(sz + 1);
    if (!svalue) {
      return zs::error_code::conversion_error;
    }

    std::memcpy(svalue, str_value.data(), sz);
    svalue[sz] = 0;

    int64_t number;
    if (sscanf(svalue, "%lld", &number) != 1) {
      return zs::error_code::conversion_error;
    }

    v = number;
    return {};
  }

  inline static zs::error_result to_real(std::string_view str_value, float_t& v) noexcept {
    ZS_TODO("Make this faster.");

    double dv;

    zb::from_chars_result res = zb::from_chars(str_value, dv);
    if (!res) {
      return zs::error_code::conversion_error;
    }

    v = dv;
    return {};
  }

  static inline zs::error_result parse_number(json_lexer* l) {
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

      l->_int_value = zb::hex_to_int(svalue.substr(2));
      l->next();
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
      l->next();
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
      l->next();
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

      //      // TODO: This never happens.
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
};

json_lexer::json_lexer(zs::engine* eng)
    : engine_holder(eng)
    , _escaped_string(zs::allocator<char>(eng)) {}

json_lexer::~json_lexer() {}

void json_lexer::init(std::string_view code) {
  _stream = zb::utf8_span_stream(code);
  _current_token = tok_none;
  _last_token = tok_none;
  _current_line = 1;
  _current_column = 1;
  _last_token_line = 1;
}

zs::object json_lexer::get_value() const {
  switch (_current_token) {
  case tok_none:
    return zs::none();
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

  return {};
}

zs::object json_lexer::get_debug_value() const {
  switch (_current_token) {
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

json_token_type json_lexer::lex() {

  if (!_stream.is_valid()) {
    return tok_eof;
  }

  _last_token_line = _current_line;

  while (_stream.is_valid()) {
    uint32_t c = *_stream;

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
      continue;

    case '`': {
      const size_t sz = _stream.rsize();
      if (sz >= 3 && _stream[1] == '`' && _stream[2] == '`') {
        helper::skip_block_comment(this);
        continue;
      }

      set_token(tok_lex_error);
      return _current_token;
    }

    case '{':
      return set_token_and_next(tok_lcrlbracket);

    case '}':
      return set_token_and_next(tok_rcrlbracket);

    case '[':
      return set_token_and_next(tok_lsqrbracket);

    case ']':
      return set_token_and_next(tok_rsqrbracket);

    case ',':
      return set_token_and_next(tok_comma);

    case '/':
      if (_stream.is_next_valid()) {
        if (_stream.get_next() == '/') {
          helper::skip_single_line_comment(this);
          continue;
        }
      }
      set_token(tok_lex_error);
      return _current_token;

    case ':':
      return set_token_and_next(tok_colon);

    case '.':
      if (_stream.is_next_valid() and zb::is_digit(_stream.get_next())) {
        helper::parse_number(this);
        return _current_token;
      }

      return set_token_and_next(tok_dot);

    case '-':
      return set_token_and_next(tok_minus);

    // String begin.
    case '"': {
      // Check for triple double quotes
      // Example: """content"""
      if (_stream.rsize() >= 3 && _stream[1] == '"' && _stream[2] == '"') {
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
      if (_stream.rsize() >= 3 && _stream[1] == '\'' && _stream[2] == '\'') {
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

      // Next char should be the closing single quote.
      if (!_stream.is_next_valid() || _stream.get_next() != '\'') {
        set_token(tok_lex_error);
        return _current_token;
      }

      // Skipping the char value.
      next();

      // Get the char value.
      _int_value = value;
      set_token(tok_integer_value);

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

        return _current_token;
      }

      return tok_lex_error;
    }
  }

  return tok_eof;
}
} // namespace zs.
ZBASE_PRAGMA_POP()
