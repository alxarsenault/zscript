#include <zscript.h>
#include <format>

namespace zs {

namespace {
  template <typename T>
  static inline std::string int_to_hex(T i) {
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << i;
    return stream.str();
  }

  template <typename T>
  static inline std::ostream& int_to_hex(std::ostream& stream, T i) {

    stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2);

    if constexpr (std::is_unsigned_v<T>) {
      stream << std::hex << (uint64_t)i;
    }
    else {
      stream << std::hex << (int64_t)i;
    }

    return stream;
  }

  static zs::string str_replace_all(
      zs::engine* eng, std::string_view str, std::string_view from, std::string_view to) {
    zs::string output_str((zs::allocator<char>(eng)));
    output_str.reserve(str.length());

    std::string_view::size_type last_pos = 0;
    std::string_view::size_type find_pos;

    while ((find_pos = str.find(from, last_pos)) != std::string_view::npos) {
      output_str.append(str, last_pos, find_pos - last_pos);
      output_str.append(to);
      last_pos = find_pos + from.length();
    }

    // Care for the rest after last occurrence.
    output_str.append(str.substr(last_pos));

    return output_str;
  }

  static std::string str_replace_all(std::string_view str, std::string_view from, std::string_view to) {
    std::string output_str;
    output_str.reserve(str.length());

    std::string_view::size_type last_pos = 0;
    std::string_view::size_type find_pos;

    while ((find_pos = str.find(from, last_pos)) != std::string_view::npos) {
      output_str.append(str, last_pos, find_pos - last_pos);
      output_str.append(to);
      last_pos = find_pos + from.length();
    }

    // Care for the rest after last occurrence.
    output_str.append(str.substr(last_pos));

    return output_str;
  }
} // namespace.
std::ostream& node_attribute_value(std::ostream& stream, std::string_view s) {

  stream << '"';
  auto it2 = s.begin();
  auto end = s.end();

  for (auto it = s.begin(); it < end; ++it) {
    switch (*it) {
    case '<':
      stream << "&lt;";
      break;

    case '>':
      stream << "&gt;";
      break;

    case '"':
      stream << "&quot;";
      break;

    case '\'':
      stream << "&apos;";
      break;

    case '&':
      stream << "&amp;";
      break;

    case '\\': {
      ++it;
      switch (*it) {
      case '\\':
        stream << '\\';
        break;
      case 'n':
        stream << '\n';
        break;
      case 'r':
        stream << '\r';
        break;
      case 't':
        stream << '\t';
        break;
      case 'f':
        stream << '\f';
        break;
      case 'v':
        stream << '\v';
        break;
      case 'b':
        stream << '\b';
        break;
      }
      break;
    }

    default:
      stream << *it;
    }
  }

  return stream << '"';
}
std::ostream& node_value(std::ostream& stream, std::string_view s) {

  //  stream << '"';
  auto it2 = s.begin();
  auto end = s.end();

  for (auto it = s.begin(); it < end; ++it) {
    switch (*it) {
    case '<':
      stream << "&lt;";
      break;

    case '>':
      stream << "&gt;";
      break;

    case '&':
      stream << "&amp;";
      break;

    case '\\': {
      switch (*++it) {
      case '\\':
        stream << '\\';
        break;
      case 'n':
        stream << '\n';
        break;
      case 'r':
        stream << '\r';
        break;
      case 't':
        stream << '\t';
        break;
      case 'f':
        stream << '\f';
        break;
      case 'v':
        stream << '\v';
        break;
      case 'b':
        stream << '\b';
        break;
      }
      break;
    }

    default:
      stream << *it;
    }
  }

  return stream;
}

std::ostream& node_stream_internal(std::ostream& stream, const serializer& s) {
  const node_object& node = s.obj.as_node();
  int indent = s.indent;

  stream << zb::indent_t(indent, 4) << "<" << node.name().get_string_unchecked();

  if (const size_t sz = node.attributes().size()) {
    for (size_t i = 0; i < sz; i++) {

      //

      stream << " " << node.attributes()[i].name.get_string_unchecked() << "=";

      if (node.attributes()[i].value.is_string()) {
        node_attribute_value(stream, node.attributes()[i].value.get_string_unchecked());
        //                stream << node.attributes()[i].value;
      }
      else {
        stream << zb::quoted(node.attributes()[i].value);
      }

      //      << node.attributes()[i].value;
    }
  }

  stream << ">";
  const size_t sz = node.children().size();
  bool is_one_liner = !sz;

  if (!node.value().is_type(object_type::k_null, object_type::k_none)) {

    if (!is_one_liner) {
      stream << "\n" << zb::indent_t(indent + 1, 4);
    }

    if (node.value().is_string()) {
      node_value(stream, node.value().get_string_unchecked());
    }
    else {
      stream << serializer(s.type, node.value());
    }
  }

  if (sz) {
    stream << "\n";

    for (size_t i = 0; i < sz; i++) {
      if (node.children()[i].is_node()) {

        node_stream_internal(stream, serializer(s.type, node.children()[i], indent + 1));
        stream << "\n";
      }
      else if (node.children()[i].is_string()) {
        stream << zb::indent_t(indent + 1, 4) << node.children()[i].get_string_unchecked() << "\n";
      }
      else {
        stream << zb::indent_t(indent + 1, 4) << node.children()[i] << "\n";
      }
    }

    stream << zb::indent_t(indent, 4) << "</" << node.name().get_string_unchecked() << ">";
  }
  else {

    if (!is_one_liner) {
      stream << "\n" << zb::indent_t(indent, 4);
    }
    stream << "</" << node.name().get_string_unchecked() << ">";
  }

  return stream;
}

std::ostream& array_stream_internal(std::ostream& stream, const serializer& s) {
  //  const node_object& node = s.obj.as_node();
  const array_object& arr = s.obj.as_array();

  int indent = s.indent;

  bool is_small_array = true;

  stream << "[";

  //  indent++;

  if (const size_t sz = arr.size()) {

    for (const auto& item : arr) {
      if (item.is_ref_counted()) {
        is_small_array = false;
        break;
      }
    }

    if (!is_small_array) {
      stream << "\n";
      indent++;
    }

    for (size_t i = 0; i < sz - 1; i++) {
      stream << zb::indent_t(indent, 4);
      stream << zs::serializer(s.type, arr[i], indent);

      //       if (is_small_array) {
      stream << ",";
      //       }
      //       else {
      //         stream << ",";
      //         //          stream << ",\n" << zb::indent_t(indent);
      //       }
    }
    stream << zb::indent_t(indent, 4);
    stream << zs::serializer(s.type, arr.back(), indent);
  }

  if (is_small_array) {
    return stream << "]";
  }

  indent--;
  return stream << "\n" << zb::indent_t(indent, 4) << "]";
}

std::ostream& string_stream_internal(std::ostream& stream, const serializer& s) {

  std::string_view str = s.obj.get_string_unchecked();
  int indent = s.indent;

  if (s.type == serializer_type::quoted) {
    return stream << zb::quoted(str);
  }
  else if (s.type == serializer_type::json or s.type == serializer_type::json_compact) {

    if (str.contains("\n") or str.contains("\"")) {

      if (s.obj.is_ref_counted()) {
        zs::engine* eng = s.obj.as_ref_counted().get_engine();
        return stream << zb::quoted(
                   str_replace_all(eng, str_replace_all(eng, str, "\"", "\\\""), "\n", "\\n"));
      }
      else {
        return stream << zb::quoted(str_replace_all(str_replace_all(str, "\"", "\\\""), "\n", "\\n"));
      }
    }
    else {
      return stream << zb::quoted(str);
    }
  }
  else {
    return stream << str;
  }

  return stream;
}

std::ostream& table_stream_internal(std::ostream& stream, const serializer& s) {
  const table_object& tbl = s.obj.as_table();

  int indent = s.indent;
  //  indent++;

  stream << "{\n";

  const size_t sz = tbl.size();
  size_t count = 0;
  for (auto it : tbl) {

    stream << zb::indent_t(indent, 4);

    if (it.first.is_table() and &tbl == it.first._table) {
      stream << "<RECURSION>";
    }
    else {

      stream << zs::serializer(s.type, it.first, indent);
    }

    stream << ": ";

    if (it.second.is_table() and &tbl == it.second._table) {
      stream << "<RECURSION>";
    }
    else {

      stream << zs::serializer(s.type, it.second, indent);
    }

    stream << ((++count == sz) ? "\n" : ",\n");
  }

  indent--;
  return stream << zb::indent_t(indent, 4) << "}";
}

std::ostream& operator<<(std::ostream& stream, const serializer& s) {
  const object_base& obj = s.obj;

  switch (s.obj.get_type()) {
  case object_type::k_null:
    return stream << "null";
  case object_type::k_none:
    return stream << "none";

  case object_type::k_bool:
    return stream << (obj._bool ? "true" : "false");

  case object_type::k_integer:
    return stream << obj._int;

  case object_type::k_float:
    return stream << obj._float;

  case object_type::k_long_string:
  case object_type::k_small_string:
  case object_type::k_string_view:
  case object_type::k_mutable_string:
    return string_stream_internal(stream, s);

  case object_type::k_table:
    //    stream << ;

    return table_stream_internal(stream, zs::serializer(s.type, s.obj, s.indent + 1));
    //      return table_stream_internal(stream, s);

  case object_type::k_array:
    return array_stream_internal(stream, s);

  case object_type::k_node:
    return node_stream_internal(stream, s);

  case object_type::k_user_data: {
    obj._udata->convert_to_string(stream);
    return stream;
  }

  default:
    // @Alex
    //    std::format_to(std::ostream_iterator<char>{ stream }, "{:#016x}", obj._value);
    return stream;
  }
  return stream;
}

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
// void replaceAll(std::string& str, const std::string& from, const std::string& to) {
//  if(from.empty())
//      return;
//  size_t start_pos = 0;
//  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
//      str.replace(start_pos, from.length(), to);
//      start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
//  }
//}
//
////stream << zb::quoted( str_replace_all(eng, str_replace_all(eng, str, "\"", "\\\""), "\n", "\\n"));
// static zs::string ewerwestr_replace_all(
//     zs::engine* eng, zs::string& str, std::string_view from, std::string_view to) {
////  zs::string output_str((zs::allocator<char>(eng)));
////  output_str.reserve(str.length());
//
////  char* wptr = str.data();
////  for(size_t i = 0; i < str.size();) {
////    if(str[i] == '\\') {
////    }
////  }
//
//  std::string_view::size_type last_pos = 0;
//
//  while ((last_pos = str.find(from, last_pos)) != std::string_view::npos) {
//    str.replace(last_pos, from.length(), to);
//    last_pos += to.length();
//  }
////
////  // Care for the rest after last occurrence.
////  output_str.append(str.substr(last_pos));
////
////  return output_str;
//}
//
//
//
//   zs::error_result serialize_string_to_json(zs::engine* eng, std::ostream& stream, const object_base& o,
//   int idt ) {
//
//  std::string_view str = o.get_string_unchecked();
//
//    if (str.contains("\n") or str.contains("\"")) {
//
//      zs::string output_str(str, eng);
//
//      if(output_str.contains("\""))
//      {
//        constexpr std::string_view from = "\"";
//        constexpr std::string_view to = "\\\"";
//        std::string_view::size_type last_pos = 0;
//
//        while ((last_pos = output_str.find(from, last_pos)) != std::string_view::npos) {
//          output_str.replace(last_pos, from.length(), to);
//          last_pos += to.length();
//        }
//      }
//
//      if(output_str.contains("\n"))
//      {
//        constexpr std::string_view from = "\n";
//        constexpr std::string_view to = "\\n";
//        std::string_view::size_type last_pos = 0;
//
//        while ((last_pos = output_str.find(from, last_pos)) != std::string_view::npos) {
//          output_str.replace(last_pos, from.length(), to);
//          last_pos += to.length();
//        }
//      }
//
//
//
//        stream << zb::quoted(output_str);
//
//    }
//    else {
//   stream << zb::quoted(str);
//    }
//
//
//    return {};
//}
//
//
//
//
//
//
//
// zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, const object_base& o, int idt ) {
//
//
//  switch (o.get_type()) {
//  case object_type::k_null:
//      stream << "null";
//      return {};
//  case object_type::k_none:
//    stream << "none";
//      return {};
//
//  case object_type::k_bool:
//      stream << (o._bool ? "true" : "false");
//      return {};
//
//  case object_type::k_integer:
//      stream << o._int;
//      return {};
//
//  case object_type::k_float:
//    stream << o._float;
//      return {};
//
//  case object_type::k_long_string:
//  case object_type::k_small_string:
//  case object_type::k_string_view:
//  case object_type::k_mutable_string:
////    return string_stream_internal(stream, s);
//      return serialize_string_to_json(eng, stream, o, idt);
////  return {};
//
//  case object_type::k_table:
//      return o._table->serialize_to_json(eng, stream, idt);
////    stream << ;
//
////      return table_stream_internal(stream, zs::serializer(s.type, s.obj, s.indent+1));
////      return table_stream_internal(stream, s);
//
//  case object_type::k_array:
//      return o._array->serialize_to_json(eng, stream, idt);
////    return array_stream_internal(stream, s);
//
////  case object_type::k_node:
////    return node_stream_internal(stream, s);
////
////  case object_type::k_user_data: {
////    obj._udata->convert_to_string(stream);
////    return stream;
////  }
//
//  default:
//    // @Alex
//    //    std::format_to(std::ostream_iterator<char>{ stream }, "{:#016x}", obj._value);
////      stream;
//  }
////  return stream;
//
//  return {};
//}
} // namespace zs.
