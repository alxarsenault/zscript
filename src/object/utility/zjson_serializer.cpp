#include <zscript/zscript.h>
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

} // namespace.

static zs::error_result serialize_string_to_json(
    zs::engine* eng, std::ostream& stream, const object_base& o, int idt) {

  std::string_view str = o.get_string_unchecked();

  if (str.contains("\n") or str.contains("\"")) {

    zs::string output_str(str, eng);

    if (output_str.contains("\"")) {
      constexpr std::string_view from = "\"";
      constexpr std::string_view to = "\\\"";
      std::string_view::size_type last_pos = 0;

      while ((last_pos = output_str.find(from, last_pos)) != std::string_view::npos) {
        output_str.replace(last_pos, from.length(), to);
        last_pos += to.length();
      }
    }

    if (output_str.contains("\n")) {
      constexpr std::string_view from = "\n";
      constexpr std::string_view to = "\\n";
      std::string_view::size_type last_pos = 0;

      while ((last_pos = output_str.find(from, last_pos)) != std::string_view::npos) {
        output_str.replace(last_pos, from.length(), to);
        last_pos += to.length();
      }
    }

    stream << zb::quoted(output_str);
  }
  else {
    stream << zb::quoted(str);
  }

  return {};
}

zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, const object_base& o, int idt) {

  switch (o.get_type()) {
  case object_type::k_null:
    stream << "null";
    return {};

  case object_type::k_none:
    stream << "none";
    return {};

  case object_type::k_atom:
    stream << "atom";
    return {};

  case object_type::k_bool:
    stream << (o._int ? "true" : "false");
    return {};

  case object_type::k_integer:
    stream << o._int;
    return {};

  case object_type::k_float:
    stream << o._float;
    return {};

  case object_type::k_long_string:
  case object_type::k_small_string:
  case object_type::k_string_view:
    return serialize_string_to_json(eng, stream, o, idt);

  case object_type::k_table:
    return o._table->serialize_to_json(eng, stream, idt);

  case object_type::k_array:
    return o._array->serialize_to_json(eng, stream, idt);

    //  case object_type::k_node:
    //    return node_stream_internal(stream, s);
    //
    //  case object_type::k_user_data: {
    //    obj._udata->convert_to_string(stream);
    //    return stream;
    //  }

  default:
    // @Alex
    //    std::format_to(std::ostream_iterator<char>{ stream }, "{:#016x}", obj._value);
    //      stream;
  }
  //  return stream;

  return {};
}
} // namespace zs.
