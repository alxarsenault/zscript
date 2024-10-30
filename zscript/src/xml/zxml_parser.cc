ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

#define ZS_XML_PARSER_HANDLE_ERROR_STREAM(err, ...) \
  helper::handle_error(this, err, zs::strprint(_engine, __VA_ARGS__), std::source_location::current())

#define ZS_XML_PARSER_HANDLE_ERROR_STRING(err, msg) \
  helper::handle_error(this, err, msg, std::source_location::current())

namespace zs {

struct xml_parser::helper {

  static inline zs::error_result handle_error(
      xml_parser* p, zs::error_code ec, std::string_view msg, const std::source_location& loc) {
    zs::line_info linfo = p->_lexer->get_last_line_info();

    const auto& stream = p->_lexer->_stream;
    const char* begin = &(*stream._data.begin());
    const char* end = &(*stream._data.end());

    const char* it_line_begin = stream.ptr() - 1;
    while (it_line_begin > begin) {
      if (*it_line_begin == '\n') {
        ++it_line_begin;
        break;
      }

      --it_line_begin;
    }

    const char* it_line_end = stream.ptr();
    while (it_line_end < end) {
      if (*it_line_end == '\n') {
        break;
      }

      ++it_line_end;
    }

    std::string_view line_content(it_line_begin, std::distance(it_line_begin, it_line_end));

    const int column = linfo.column ? (int)linfo.column - 1 : 0;

    constexpr const char* new_line_padding = "\n       ";

    std::string_view fname = loc.function_name();

    if (fname.size() > 80) {
      p->_error_message += zs::strprint<"">(p->_engine, "\nerror: ", linfo, new_line_padding, line_content,
          new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", fname.substr(0, 80),
          "\n               ", fname.substr(80), "'", new_line_padding, "     in '", loc.file_name(), "'",
          new_line_padding, "     at line ", loc.line(), "\n", new_line_padding, "*** ", msg);
    }
    else {
      p->_error_message += zs::strprint<"">(p->_engine, "\nerror: ", linfo, new_line_padding, line_content,
          new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", loc.function_name(),
          "'", new_line_padding, "      in '", loc.file_name(), "'", new_line_padding, "     at line ",
          loc.line(), "\n", new_line_padding, "*** ", msg);
    }

    return ec;
  }

  static inline void replace_all(zs::string& str, std::string_view from, std::string_view to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != zs::string::npos) {
      str.replace(start_pos, from.length(), to);
      start_pos += to.length();
    }
  }

  static inline zs::string& cleanup_escapes(zs::string& s) {
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
    replace_all(s, "&quot;", "\"");
    replace_all(s, "&apos;", "'");
    replace_all(s, "&lt;", "<");
    replace_all(s, "&gt;", ">");
    replace_all(s, "&amp;", "&");
    return s;
  }
};

//
// MARK: xml_parser
//

xml_parser::xml_parser(zs::engine* eng)
    : engine_holder(eng)
    , _stack(zs::allocator<object>(eng))
    , _error_message(zs::allocator<char>(eng)) {}

xml_parser::~xml_parser() {}

xml_token_type xml_parser::lex(bool lazy) {
  _last_ptr = _lexer->_stream.ptr();
  _token = lazy ? _lexer->lazy_lex() : _lexer->lex();
  return _token;
}

zs::error_result xml_parser::parse(zs::virtual_machine* vm, std::string_view content, const object& table,
    object& output, zs::xml_token_type* prepended_token) {
  using enum xml_token_type;
  using enum zs::error_code;

  _vm = vm;
  _table = table.is_table() ? &table : nullptr;
  zs::xml_lexer lexer(_engine);

  _lexer = &lexer;

  _lexer->init(content);

  if (prepended_token) {
    _token = *prepended_token;
  }
  else {
    lex();
  }

  object ret_value;

  if (is(tok_lt)) {
    lex();
    if (auto err = parse_node(ret_value)) {
      return helper::handle_error(this, err, "invalid token", std::source_location::current());
    }
  }
  else {
    return helper::handle_error(this, invalid_token, "expected <", std::source_location::current());
  }

  if (_token == tok_lex_error) {
    return invalid;
  }

  output = ret_value;
  return {};
}

zs::error_result xml_parser::parse_node(zs::object& value) {
  using enum xml_token_type;
  using enum error_code;

  switch (_token) {
  case tok_identifier:
  case tok_template_begin:
    return parse_element(value);

  // <...
  case tok_question:
    return unimplemented;

    // <!...
  case tok_exclam:
    return unimplemented;

  default:
    return invalid_token;
  }

  return {};
}

zs::error_result xml_parser::parse_element(zs::object& node) {
  using enum xml_token_type;
  using enum zs::error_code;

  // Tag names are case-sensitive; the start-tag and end-tag must match exactly.
  // Tag names cannot contain any of the characters !"#$%&'()*+,/;<=>?@[\]^`{|}~,
  // nor a space character, and cannot begin with "-", ".", or a numeric digit.
  object tag_name;
  ZS_RETURN_IF_ERROR(parse_tag_or_attribute_name(tag_name));

  node = object::create_node(_engine, tag_name);

  ZS_RETURN_IF_ERROR(parse_attributes(node));

  // Determine ending type.
  switch (_token) {
  case tok_gt: {
    const char* data_begin = _lexer->_stream.ptr();

    lex();
    ZS_RETURN_IF_ERROR(parse_node_contents(node, data_begin));

    ZS_RETURN_IF_ERROR(expect(tok_node_end));

    object end_tag_name;
    ZS_RETURN_IF_ERROR(parse_tag_or_attribute_name(end_tag_name));

    if (tag_name != end_tag_name) {
      return helper::handle_error(
          this, invalid_argument, "wrong closing name", std::source_location::current());
    }

    return expect(tok_gt);
  }
  case tok_slash: {
    lex();
    return expect(tok_gt);
  }

  default:
    return helper::handle_error(this, invalid_token, "expected >", std::source_location::current());
  }

  return {};
}

zs::error_result xml_parser::parse_node_contents(zs::object& node, const char* data_begin) {
  using enum xml_token_type;
  using enum zs::error_code;
  const char* data_end = _last_ptr;

  int tcount = 0;

  bool is_first_node = true;

  while (is_not(tok_node_end, tok_eof)) {
    if (is(tok_lt)) {
      lex();

      if (int_t sz = std::distance(data_begin, data_end); sz and tcount and data_end) {

        std::string_view data(data_begin, sz);

        // Got data before first node?
        if (is_first_node) {
          is_first_node = false;

          if (data.contains("{{") and data.contains("}}")) {
            size_t idx = data.find("{{");
            node.as_node().value() = zs::_s(_engine, data);
          }
          else {
            node.as_node().value() = zs::_s(_engine, data);
          }
        }
        else {
          if (data.contains("{{") and data.contains("}}")) {
            size_t idx = data.find("{{");
            //          zb::print("===============", idx);
            node.as_node().children().push_back(zs::_s(_engine, data));
          }
          else {
            node.as_node().children().push_back(zs::_s(_engine, data));
          }
        }

        //        zs::string escaped_string(std::string_view(data_begin, sz), zs::string_allocator(_engine));
        //        node.as_node().children().push_back(zs::_s(_engine,
        //        helper::cleanup_escapes(escaped_string)));
      }

      // Child node.
      object child_node;
      ZS_RETURN_IF_ERROR(parse_element(child_node));
      node.as_node().children().push_back(child_node);
      tcount = 0;
      data_begin = _last_ptr;
    }
    else {
      lex(true);
      tcount++;
    }

    data_end = _last_ptr;
  }

  if (_token == tok_lex_error) {
    return helper::handle_error(this, invalid_token, "</", std::source_location::current());
  }

  if (int_t sz = std::distance(data_begin, data_end); sz and tcount and data_end) {
    std::string_view data(data_begin, sz);
    size_t l_idx = data.find("{{");
    size_t r_idx = data.find("}}");

    if (l_idx != std::string_view::npos and r_idx != std::string_view::npos and l_idx < r_idx) {

      object value;
      std::string_view sss = data.substr(l_idx + 2, r_idx - (l_idx + 2));
      object key = zs::_s(_engine, sss);

      if (_table) {
        if (_vm) {
          ZS_RETURN_IF_ERROR(_vm->get(*_table, key, value));
        }
        else {
          ZS_RETURN_IF_ERROR(_table->as_table().get(key, value));
        }
      }
      else {
        return inaccessible;
      }

      if (is_first_node) {
        is_first_node = false;
        node.as_node().value() = std::move(value);
      }
      else {
        node.as_node().children().push_back(value);
      }
    }
    else {
      if (is_first_node) {
        is_first_node = false;
        node.as_node().value() = zs::_s(_engine, data);
      }
      else {
        node.as_node().children().push_back(zs::_s(_engine, data));
      }
    }
    //    zs::string escaped_string(std::string_view(data_begin, sz), zs::string_allocator(_engine));
    //    node.as_node().children().push_back(zs::_s(_engine, helper::cleanup_escapes(escaped_string)));
  }

  return {};
}

zs::error_result xml_parser::parse_attributes(zs::object& node) {
  using enum xml_token_type;
  using enum zs::error_code;

  // Attribute name (anything but space \n \r \t / < > = ? ! \0)
  while (is_not(tok_lt, tok_gt, tok_eq, tok_question, tok_exclam, tok_slash)) {

    zs::object attribute_name;
    ZS_RETURN_IF_ERROR(parse_tag_or_attribute_name(attribute_name));
    ZS_RETURN_IF_ERROR(expect(tok_eq));

    zs::object attribute_value;

    switch (_token) {
    case tok_string_value:
      attribute_value = zs::_s(_engine, _lexer->get_string_value());
      break;

    case tok_escaped_string_value:
      attribute_value = zs::_s(_engine, _lexer->get_escaped_string_value());
      break;

    case tok_integer_value:
      attribute_value = _lexer->get_int_value();
      break;

    case tok_float_value:
      attribute_value = _lexer->get_float_value();
      break;

    case tok_false:
      attribute_value = false;
      break;

    case tok_true:
      attribute_value = true;
      break;

    case tok_null:
      attribute_value = nullptr;
      break;

    default:
      return invalid_token;
    }

    node.as_node().attributes().emplace_back(attribute_name, attribute_value);
    lex();
  }

  return {};
}

zs::error_result xml_parser::parse_tag_or_attribute_name(zs::object& name) {
  using enum xml_token_type;
  using enum zs::error_code;

  if (is(tok_identifier)) {
    name = _lexer->get_identifier();
    lex();
    return {};
  }

  ZS_RETURN_IF_ERROR(expect(tok_template_begin));

  zs::object key;
  ZS_RETURN_IF_ERROR(expect_get(tok_identifier, key));

  object value;

  if (_table) {
    if (_vm) {
      ZS_RETURN_IF_ERROR(_vm->get(*_table, key, value));
    }
    else {
      ZS_RETURN_IF_ERROR(_table->as_table().get(key, value));
    }
  }
  else {
    return inaccessible;
  }

  while (is(tok_dot)) {
    lex();
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, key));

    if (_vm) {
      ZS_RETURN_IF_ERROR(_vm->get(value, key, value));
    }
    else if (value.is_table()) {
      ZS_RETURN_IF_ERROR(value.as_table().get(key, value));
    }
    else {
      return inaccessible;
    }
  }

  ZS_RETURN_IF_ERROR(expect(tok_template_end));

  if (!value.is_string()) {
    return helper::handle_error(this, invalid_name, "Invalid tag name", std::source_location::current());
  }

  name = value;
  return {};
}

zs::error_result xml_parser::parse_bom() { return {}; }

zs::error_code xml_parser::expect(xml_token_type tok) noexcept {
  using enum zs::error_code;

  if (is_not(tok)) {
    _error_message
        += zs::strprint<"">(_engine, "invalid token ", zb::quoted<"'">(zs::xml_token_to_string(_token)),
            ", expected ", zb::quoted<"'">(zs::xml_token_to_string(tok)), _lexer->get_line_info());
    return invalid_token;
  }

  lex();
  return success;
}

zs::error_code xml_parser::expect_get(xml_token_type tok, object& ret) {
  using enum xml_token_type;
  using enum zs::error_code;

  if (is_not(tok)) {
    _error_message
        += zs::strprint<"">(_engine, "invalid token ", zb::quoted<"'">(zs::xml_token_to_string(_token)),
            ", expected ", zb::quoted<"'">(zs::xml_token_to_string(tok)), _lexer->get_line_info(),
            std::source_location::current());

    return invalid_token;
  }

  ret = _lexer->get_value();
  lex();
  return {};
}

} // namespace zs.

ZBASE_PRAGMA_POP()
