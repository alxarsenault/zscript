#include "lex/ztokenizer.h"
#include <zbase/strings/charconv.h>
#include <zbase/container/constexpr_map.h>
#include <zbase/strings/parse_utils.h>

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

namespace zs {

tokenizer::tokenizer(zs::engine* eng)
    : engine_holder(eng) {}

tokenizer::tokenizer(zs::engine* eng, std::string_view code)
    : tokenizer(eng) {
  init(code);
}

tokenizer::~tokenizer() {}

void tokenizer::init(std::string_view code) {
  _stream = zb::utf8_span_stream(code);
  _tok_vtype = tv_none;
  _current_token = tok_none;
  _last_token = tok_none;
  _last_token_line = 1;
  _current_line_info = { 1, 1 };
  _last_line_info = { 0, 0 };
  _value = {};
}

void tokenizer::skip_white_spaces() {
  _last_line_info = get_line_info();

  while (zb::is_one_of(*_stream, ' ', '\t')) {
    _current_line_info.column++;
    _stream.incr();
  }
}

zs::error_result tokenizer::parse_string(uint32_t end_char) {
  zb::utf8_span_stream& s = _stream;

  if (!s.is_next_valid()) {
    set_token(tok_lex_error);
    return;
  }

  next();

  const char* beg = s.ptr();

  bool escaped_found = false;
  bool escaped = false;

  while (s.is_valid()) {

    if (*s == '\n') {
      set_token(tok_lex_error);
      return;
    }

    if (!escaped && ('\\' == *s)) {
      escaped_found = true;
      escaped = true;

      next();
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

    next();
  }

  if (s.is_end()) {
    set_token(tok_lex_error);
    return zs::error_code::invalid_token;
  }

  size_t sz = std::distance(beg, s.ptr());
  std::string_view str(beg, sz);

  if (escaped_found) {
    set_token(tok_escaped_string_value);
    _tok_vtype = tv_escaped_string;
    _value = str;
    next();
    return {};
  }

  set_token(tok_string_value);
  _tok_vtype = tv_string;

  _value = str;
  next();
  return {};
}

zs::error_result tokenizer::parse_multi_line_string(uint32_t end_char) {
  zb::utf8_span_stream& s = _stream;

  if (!s.is_next_valid()) {
    set_token(tok_lex_error);
    return zs::error_code::invalid_token;
  }

  next();

  const char* beg = s.ptr();

  bool escaped_found = false;
  bool escaped = false;

  while (s.is_valid()) {

    if (!escaped && ('\\' == *s)) {
      escaped_found = true;
      escaped = true;
      next();
      continue;
    }
    else if (!escaped) {
      if (end_char == *s) {

        if (s.rsize() >= 3 && s[1] == end_char && s[2] == end_char) {
          break;
        }
        next();
      }
    }
    else if (escaped) {
      escaped = false;
    }
    next();
  }

  if (s.is_end()) {
    set_token(tok_lex_error);
    return zs::error_code::invalid_token;
  }

  size_t sz = std::distance(beg, s.ptr());

  std::string_view str(beg, sz);

  if (escaped_found) {
    set_token(tok_escaped_string_value);
    _tok_vtype = tv_escaped_string;
    _value = str;
    next(3);
    return {};
  }

  set_token(tok_string_value);
  _tok_vtype = tv_string;
  _value = str;
  next(3);
  return {};
}

zs::error_result tokenizer::parse_single_line_comment() {
  auto& s = _stream;

  // Skip `//`.
  next(2);

  if (s.is_end()) {
    set_token(tok_eof);
    return {};
  }

  while (*s != '\n' && !s.is_end()) {
    next();
  }

  if (*s == '\n') {
    s.incr();
    new_line();
  }

  if (s.is_end()) {
    set_token(tok_eof);
    return {};
  }

  return {};
}

zs::error_result tokenizer::parse_multi_line_comment() {
  auto& s = _stream;

  // Skip `/*`.
  next(2);

  if (s.is_end()) {
    set_token(tok_eof);
    return {};
  }

  while (s.is_valid()) {

    if (*s == '*' && s.is_next_valid() && s.get_next() == '/') {
      next(2);

      return;
    }

    if (*s == '\n') {
      new_line();
      ++s;
    }
    else {
      next();
    }
  }

  if (s.is_end()) {
    set_token(tok_eof);
    return {};
  }

  return {};
}

zs::error_result tokenizer::parse_number() {
  zb::utf8_span_stream& s = _stream;

  const char* beg = s.ptr();
  bool dot_found = false;
  bool e_found = false;
  bool post_e_sign_found = false;
  bool post_e_digit_found = false;

  // TODO: Fix negative.
  bool is_negative = false;

  // Check for hexadecimal number.
  if (*s == '0' and zb::is_one_of(s.get_next(), 'x', 'X')) {
    next(2);
    while (s) {
      const uint32_t t = *s;

      if (zb::is_digit(t) or zb::is_one_of(t, 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F')) {
        next();
      }
      else {
        break;
      }
    }

    const size_t sz = std::distance(beg, s.ptr());
    std::string_view svalue(beg, sz);
    set_token(tok_integer_value);
    _tok_vtype = tv_hex_number;
    _value = svalue;
    //    _int_value = zb::hex_to_int(svalue.substr(2));

    ///////////////////////////////////////////////////////////////
    /// TODO: Check this.
    //      l->next();
    return {};
  }

  // Check for binary number.
  if (*s == '0' and s.get_next() == 'b') {
    next(2);
    while (s) {
      const uint32_t t = *s;

      if (zb::is_digit(t) or zb::is_one_of(t, '0', '1')) {
        next();
      }
      else {
        break;
      }
    }

    const size_t sz = std::distance(beg, s.ptr());
    std::string_view svalue(beg, sz);
    set_token(tok_integer_value);
    _tok_vtype = tv_binary_number;
    _value = svalue;
    next();
    return {};
  }

  // Check for octal number.
  if (*s == '0' and s.get_next() == 'h') {
    next(2);
    while (s) {
      const uint32_t t = *s;

      if (zb::is_digit(t) or zb::is_one_of(t, '0', '1', '2', '3', '4', '5', '6', '7')) {
        next();
      }
      else {
        break;
      }
    }

    const size_t sz = std::distance(beg, s.ptr());
    std::string_view svalue(beg, sz);
    set_token(tok_integer_value);
    _tok_vtype = tv_octal_number;
    _value = svalue;
    next();
    return {};
  }

  while (s.is_valid()) {
    const uint32_t t = *s;

    if ('.' == t) {

      if (dot_found) {
        // Multiple dots.
        set_token(tok_lex_error);
        return zs::error_code::invalid_number;
      }

      dot_found = true;
      next();
      continue;
    }

    // TODO: Fix this.
    if ('-' == t) {
      next();
      //        is_negative = true;
      continue;
    }

    // TODO: This never happens?
    else if ('+' == *s) {
      next();
      //        is_negative = false;
      continue;
    }

    else if (zb::is_one_of(t, 'e', 'E')) {
      char c = s.get_next();

      if (s.is_next_end()) {
        set_token(tok_lex_error);
        return zs::error_code::invalid_number;
      }
      else if (('+' != c) && ('-' != c) && !zb::is_digit(c)) {
        set_token(tok_lex_error);
        return zs::error_code::invalid_number;
      }

      e_found = true;
      next();
      continue;
    }

    else if (e_found && zb::is_sign(*s) && !post_e_digit_found) {
      if (post_e_sign_found) {
        set_token(tok_lex_error);
        return zs::error_code::invalid_number;
      }

      post_e_sign_found = true;
      next();
      continue;
    }

    else if (e_found && zb::is_digit(*s)) {
      post_e_digit_found = true;
      next();
      continue;
    }

    else if (('.' != t) && !zb::is_digit(*s)) {
      break;
    }

    else {
      next();
    }
  }

  const size_t sz = std::distance(beg, s.ptr());
  std::string_view svalue(beg, sz);

  if (is_negative) {
    // TODO: Handle negative.
  }

  if (dot_found || e_found) {
    set_token(tok_float_value);
    _tok_vtype = tv_float_number;
    _value = svalue;
    return {};
  }

  set_token(tok_integer_value);
  _tok_vtype = tv_int_number;
  _value = svalue;
  return {};
}

} // namespace zs.
ZBASE_PRAGMA_POP()
