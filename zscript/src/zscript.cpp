#include <zscript/zscript.h>

#define ZS_SCRIPT_CPP 1

#include <zbase/crypto/base64.h>
#include <zbase/strings/charconv.h>
#include <zbase/container/constexpr_map.h>
#include <zbase/container/enum_array.h>
#include <zbase/utility/math.h>
#include <zbase/strings/parse_utils.h>
#include <zbase/utility/print.h>
#include <zbase/utility/scoped.h>
#include <zbase/strings/stack_string.h>
#include <zbase/memory/ref_wrapper.h>
#include <zbase/graphics/color.h>
#include <ranges>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#include <random>

#if !defined(ZS_UNIX) \
    && (defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(BSD))
#define ZS_UNIX 1
#else
#deine ZS_UNIX 0
#endif

#if ZS_UNIX
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h> // defines "BSD" macro on BSD systems
#include <pwd.h>
#include <glob.h>
#include <fnmatch.h>
#endif

#ifdef BSD
#define ZS_BSD 1
#include <sys/time.h>
#include <sys/sysctl.h>
#else
#define ZS_BSD 0
#endif

#ifdef __APPLE__
#define ZS_APPLE 1
#include <mach-o/dyld.h>
#else
#define ZS_APPLE 0
#endif

#include "zvirtual_machine.h"

// Objects.
#include "object/zfunction_prototype.h"

// Lang.
#include "lang/jit/zclosure_compile_state.h"
#include "lang/zcompiler.h"
#include "lang/jit/zjit_compiler.h"
#include "lang/zlexer.h"
#include "lang/zopcode.h"
#include "lang/ztoken.h"

// Json.
#include "json/zjson_lexer.h"
#include "json/zjson_parser.h"

// Xml.
#include "xml/zxml_lexer.h"
#include "xml/zxml_parser.h"

// Lib.
#include "lib/zfs.h"
#include "lib/zfs/zpath.h"
#include "lib/zfs/zfile.h"
#include "lib/zmath.h"
#include "lib/zlang.h"
#include "lib/zslib.h"
#include "lib/zsbase64.h"
#include "lib/zgraphics.h"
#include "lib/delegate/zarray_delegate.h"
#include "lib/delegate/znative_array_delegate.h"
#include "lib/delegate/ztable_delegate.h"
#include "lib/delegate/zstring_delegate.h"
#include "lib/delegate/zcolor_delegate.h"
#include "lib/delegate/zstruct_delegate.h"

namespace zs {
static_assert(sizeof(object_base) == constants::k_object_size, "sizeof(object) != k_object_size");

//
//
//
zs::error_result default_file_loader::open(const char* filepath) noexcept {
  if (auto err = _fv.open(filepath)) {
    return zs::error_code::open_file_error;
  }

  return {};
}

zs::error_result default_file_loader::open(std::string_view filepath) noexcept {
  const size_t sz = filepath.size();
  char* cfilepath = (char*)zb_alloca(sz + 1);
  ::memcpy(cfilepath, filepath.data(), sz);
  cfilepath[sz] = 0;

  if (auto err = _fv.open(cfilepath)) {
    return zs::error_code::open_file_error;
  }

  return {};
}

//
// MARK: Object
//
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

enum class last_char_t { first, none, endl, comma, quote, bracket };

struct streamer_common {
  static last_char_t last_char;
};

last_char_t streamer_common::last_char = last_char_t::none;

namespace {
  struct object_stream_proxy_tag {};
} // namespace.

template <>
struct internal::proxy<object_stream_proxy_tag> {

  using serializer_type = object_base::serializer_type;

  template <class T, serializer_type SType>
  using streamer_base = zs::streamer_base<T, SType>;

  template <serializer_type SType>
  struct string_streamer : streamer_base<object_base, SType> {
    inline string_streamer(const object_base& o, int idt = 0)
        : streamer_base<object_base, SType>(o, idt) {}
  };

  template <serializer_type SType>
  using object_streamer = streamer_base<object_base, SType>;

  template <serializer_type SType>
  using table_streamer = streamer_base<table_object, SType>;

  template <serializer_type SType>
  using array_streamer = streamer_base<array_object, SType>;

  template <serializer_type SType>
  using struct_instance_streamer = streamer_base<struct_instance_object, SType>;

  template <serializer_type SType>
  using struct_streamer = streamer_base<struct_object, SType>;
};

using object_stream_proxy = internal::proxy<object_stream_proxy_tag>;

template <object_base::serializer_type SType>
std::ostream& operator<<(std::ostream& stream, const object_stream_proxy::string_streamer<SType>& streamer) {
  using stype = object_base::serializer_type;
  //  using sproxy = object_stream_proxy;

  const object_base& obj = streamer.obj;

  std::string_view s = obj.get_string_unchecked();

  if constexpr (SType == stype::quoted) {
    streamer_common::last_char = last_char_t::quote;
    return stream << zb::quoted(s);
  }
  else if constexpr (SType == stype::json or SType == stype::json_compact) {

    if (s.contains("\n") or s.contains("\"")) {

      if (obj.is_ref_counted()) {
        streamer_common::last_char = last_char_t::quote;
        zs::engine* eng = obj.as_ref_counted().get_engine();
        return stream << zb::quoted(str_replace_all(eng, str_replace_all(eng, s, "\"", "\\\""), "\n", "\\n"));
      }
      else {
        streamer_common::last_char = last_char_t::quote;
        return stream << zb::quoted(str_replace_all(str_replace_all(s, "\"", "\\\""), "\n", "\\n"));
      }
    }
    else {
      streamer_common::last_char = last_char_t::quote;
      return stream << zb::quoted(s);
    }
  }
  else {
    streamer_common::last_char = last_char_t::quote;
    return stream << s;
  }

  return stream;
}

template <object_base::serializer_type SType>
std::ostream& operator<<(std::ostream& stream, const object_stream_proxy::table_streamer<SType>& streamer) {
  using stype = object_base::serializer_type;
  using sproxy = object_stream_proxy;

  const table_object& tbl = streamer.obj;
  int indent = streamer.indent;

  bool is_small_table = true;

  for (auto it : tbl) {
    if (it.second.is_ref_counted()) {
      is_small_table = false;
      break;
    }
  }

  if constexpr (SType == stype::json) {

    if (is_small_table) {

      stream << "{";
      streamer_common::last_char = last_char_t::bracket;
    }
    else {

      if (streamer_common::last_char == last_char_t::first) {
        stream << zb::indent_t(indent) << "{\n";
        streamer_common::last_char = last_char_t::endl;
      }
      else {

        stream << "\n" << zb::indent_t(indent) << "{\n";
        streamer_common::last_char = last_char_t::endl;
      }
    }
  }
  else if constexpr (SType == stype::json_compact) {
    streamer_common::last_char = last_char_t::endl;
    stream << "{";
  }
  else if constexpr (SType == stype::plain_compact) {
    streamer_common::last_char = last_char_t::endl;
    stream << "{";
  }
  else {
    if (is_small_table) {
      stream << "{";
    }

    else {
      streamer_common::last_char = last_char_t::endl;
      stream << "{\n";
    }
  }

  if constexpr (!(SType == stype::plain_compact or SType == stype::json_compact)) {
    if (!is_small_table) {
      indent++;
    }
  }

  const size_t sz = tbl.size();
  size_t count = 0;
  for (auto it : tbl) {

    if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {
    }
    else {
      if (!is_small_table) {
        stream << zb::indent_t(indent);
      }
    }

    if (it.first.is_table() and &tbl == it.first._table) {
      stream << "<RECURSION>";
    }
    else {
      if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {
        stream << object_stream_proxy::object_streamer<SType>(it.first);
      }
      else {

        if constexpr (!(SType == stype::json or SType == stype::json_compact)) {
          if (it.first.is_string()) {
            stream << sproxy::string_streamer<stype::quoted>(it.first, indent);
          }
          else {
            stream << sproxy::object_streamer<SType>(it.first, indent);
          }
        }
        else {
          stream << sproxy::object_streamer<SType>(it.first, indent);
        }
      }
    }

    if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {
      stream << ":";
    }
    else {
      stream << ": ";
    }

    if (it.second.is_table() and &tbl == it.second._table) {
      stream << "<RECURSION>";
    }
    else {
      if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {
        stream << sproxy::object_streamer<SType>(it.second);
      }
      else {
        stream << sproxy::object_streamer<SType>(it.second, indent);
      }
    }

    if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {
      stream << ((++count == sz) ? "" : ",");
    }
    else {
      if (is_small_table) {
        stream << ((++count == sz) ? "" : ",");
      }
      else {
        stream << ((++count == sz) ? "\n" : ",\n");
      }
    }

    streamer_common::last_char = last_char_t::endl;
  }

  if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {
    streamer_common::last_char = last_char_t::bracket;
    return stream << "}";
  }
  else {
    if (is_small_table) {
      streamer_common::last_char = last_char_t::bracket;
      return stream << "}";
    }
    else {
      indent--;
      streamer_common::last_char = last_char_t::bracket;
      return stream << zb::indent_t(indent) << "}";
    }
  }
}

template <object_base::serializer_type SType>
std::ostream& operator<<(
    std::ostream& stream, const object_stream_proxy::struct_instance_streamer<SType>& streamer) {
  using stype = object_base::serializer_type;
  using sproxy = object_stream_proxy;

  const zs::struct_instance_object& sobj = streamer.obj;
  int indent = streamer.indent;
  bool is_small_array = true;

  stream << "[";

  const auto statics = sobj.get_static_members();
  const size_t statics_sz = statics.size();
  const size_t objs_sz = sobj.size();

  if (statics_sz + objs_sz >= 8) {
    is_small_array = false;
  }

  if constexpr (!(SType == stype::plain_compact or SType == stype::json_compact)) {
    if (!is_small_array) {
      indent++;
    }
  }

  if (statics_sz) {
    const size_t last_idx = statics_sz + objs_sz - 1;
    for (size_t i = 0; i < statics_sz; i++) {
      stream << "{";
      stream << sproxy::object_streamer<SType>(statics[i].key, indent);

      if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {

        stream << ":";
      }
      else {

        stream << " : ";
      }
      stream << sproxy::object_streamer<SType>(statics[i].value, indent);
      stream << "}";

      if (i < last_idx) {

        if (is_small_array) {
          stream << ",";
          streamer_common::last_char = last_char_t::comma;
        }
        else {
          stream << ",\n" << zb::indent_t(indent);
          streamer_common::last_char = last_char_t::endl;
        }
      }
      else {
        streamer_common::last_char = last_char_t::none;
      }
    }
  }

  if (objs_sz) {

    //      if (!statics.empty()) {
    //
    //        if(is_small_array) {
    //          stream << ",";
    //        }
    //        else {
    //          stream << ",\n";
    //        }
    //      }

    const size_t last_idx = objs_sz - 1;
    for (size_t i = 0; i < objs_sz; i++) {
      stream << "{";
      stream << sproxy::object_streamer<SType>(sobj.key(i), indent);

      if constexpr (SType == stype::plain_compact or SType == stype::json_compact) {

        stream << ":";
      }
      else {

        stream << " : ";
      }
      stream << sproxy::object_streamer<SType>(sobj[i], indent);
      stream << "}";

      if (i < last_idx) {

        if (is_small_array) {
          stream << ",";
          streamer_common::last_char = last_char_t::comma;
        }
        else {
          stream << ",\n" << zb::indent_t(indent);
          streamer_common::last_char = last_char_t::endl;
        }
      }
      else {
        streamer_common::last_char = last_char_t::none;
      }
    }
  }

  if constexpr (!(SType == stype::plain_compact or SType == stype::json_compact)) {
    indent--;
  }
  streamer_common::last_char = last_char_t::bracket;
  return stream << "]";
}

// Struct object.
template <object_base::serializer_type SType>
std::ostream& operator<<(std::ostream& stream, const object_stream_proxy::struct_streamer<SType>& streamer) {
  //  using stype = object::serializer_type;
  using sproxy = object_stream_proxy;

  const zs::struct_object& sobj = streamer.obj;
  int indent = streamer.indent;
  indent++;
  stream << "[";

  const auto statics = sobj.get_static_members();

  if (const size_t sz = statics.size()) {
    const size_t last_idx = sz - 1;
    for (size_t i = 0; i < sz; i++) {
      stream << "{";
      stream << sproxy::object_streamer<SType>(statics[i].key, indent);
      stream << " : ";
      stream << sproxy::object_streamer<SType>(statics[i].value, indent);
      stream << "}";
      if (i < last_idx) {
        streamer_common::last_char = last_char_t::comma;
        stream << ",";
      }
      else {
        streamer_common::last_char = last_char_t::none;
      }
    }
  }
  if (const size_t sz = sobj.size()) {

    if (!statics.empty()) {
      stream << ",";
    }

    const size_t last_idx = sz - 1;
    for (size_t i = 0; i < sz; i++) {
      stream << "{";
      stream << sproxy::object_streamer<SType>(sobj[i].key, indent);
      stream << " : ";
      stream << sproxy::object_streamer<SType>(sobj[i].value, indent);
      stream << "}";
      if (i < last_idx) {
        stream << ",";
        streamer_common::last_char = last_char_t::comma;
      }
      else {
        streamer_common::last_char = last_char_t::none;
      }
    }
  }

  indent--;
  streamer_common::last_char = last_char_t::none;
  return stream << "]";
}

template <object_base::serializer_type SType>
std::ostream& operator<<(std::ostream& stream, const object_stream_proxy::array_streamer<SType>& streamer) {
  using stype = object_base::serializer_type;
  using sproxy = object_stream_proxy;

  const array_object& arr = streamer.obj;
  int indent = streamer.indent;
  bool is_small_array = true;

  stream << "[";

  if constexpr (SType == stype::json) {
    indent++;
  }

  if (const size_t sz = arr.size()) {

    for (const auto& item : arr) {
      if (item.is_ref_counted()) {
        is_small_array = false;
        break;
      }
    }

    for (size_t i = 0; i < sz - 1; i++) {
      stream << sproxy::object_streamer<SType>(arr[i], indent);

      if (is_small_array) {
        stream << ",";
      }
      else {
        stream << ",";
        //          stream << ",\n" << zb::indent_t(indent);
      }
      streamer_common::last_char = last_char_t::comma;
    }

    stream << sproxy::object_streamer<SType>(arr.back(), indent);
  }

  if constexpr (SType == stype::json) {
    indent--;

    if (is_small_array) {
      return stream << "]";
    }
    else {
      return stream << "\n" << zb::indent_t(indent) << "]";
    }
  }
  else {
    streamer_common::last_char = last_char_t::none;
    return stream << "]";
  }

  return stream;
}

namespace {
  template <object_base::serializer_type SType>
  std::ostream& internal_object_stream(
      std::ostream& stream, const object_stream_proxy::object_streamer<SType>& streamer) {
    using sproxy = object_stream_proxy;

    const object_base& obj = streamer.obj;
    int indent = streamer.indent;

    switch (obj.get_type()) {
    case object_type::k_null:
      return stream << "null";

    case object_type::k_bool:
      return stream << (obj._bool ? "true" : "false");

    case object_type::k_integer:
      return stream << obj._int;

    case object_type::k_float: {
      return stream << obj._float;
    }

    case object_type::k_long_string:
    case object_type::k_small_string:
    case object_type::k_string_view:
    case object_type::k_mutable_string:
      return stream << sproxy::string_streamer<SType>(obj, indent);

    case object_type::k_table:
      return stream << sproxy::table_streamer<SType>(obj.as_table(), indent);

    case object_type::k_array:
      return stream << sproxy::array_streamer<SType>(obj.as_array(), indent);

    case object_type::k_struct:
      return stream << sproxy::struct_streamer<SType>(obj.as_struct(), indent);

    case object_type::k_struct_instance:
      return stream << sproxy::struct_instance_streamer<SType>(obj.as_struct_instance(), indent);

    case object_type::k_user_data: {
      obj._udata->convert_to_string(stream);
      return stream;
    }

    default:
      return stream << int_to_hex(obj._value);
    }
    return stream;
  }
} // namespace.

template <>
std::ostream& operator<< <>(std::ostream& stream,
    const object_stream_proxy::object_streamer<object_base::serializer_type::plain>& s) {
  return internal_object_stream(stream, s);
}

template <>
std::ostream& operator<< <>(std::ostream& stream,
    const object_stream_proxy::object_streamer<object_base::serializer_type::quoted>& s) {
  return internal_object_stream(stream, s);
}

template <>
std::ostream& operator<< <>(
    std::ostream& stream, const object_stream_proxy::object_streamer<object_base::serializer_type::json>& s) {
  return internal_object_stream(stream, s);
}

template <>
std::ostream& operator<< <>(std::ostream& stream,
    const object_stream_proxy::object_streamer<object_base::serializer_type::json_compact>& s) {
  return internal_object_stream(stream, s);
}

template <>
std::ostream& operator<< <>(std::ostream& stream,
    const object_stream_proxy::object_streamer<object_base::serializer_type::plain_compact>& s) {
  return internal_object_stream(stream, s);
}

std::string object_base::convert_to_string() const {
  std::ostringstream ss;
  streamer_common::last_char = last_char_t::first;
  ss << object_stream_proxy::object_streamer<object_base::serializer_type::quoted>(*this);
  return ss.str();
}

std::ostream& object_base::stream_to_string(std::ostream& ss) const {
  streamer_common::last_char = last_char_t::first;
  return ss << object_stream_proxy::object_streamer<object_base::serializer_type::quoted>(*this);
}

std::string object_base::to_json() const {
  std::ostringstream ss;
  stream_to_json(ss);
  return ss.str();
}

std::ostream& object_base::stream_to_json(std::ostream& ss) const {
  streamer_common::last_char = last_char_t::first;
  return ss << object_stream_proxy::object_streamer<object_base::serializer_type::json>(*this);
}

std::ostream& object_base::stream_to_json_compact(std::ostream& ss) const {
  streamer_common::last_char = last_char_t::first;
  return ss << object_stream_proxy::object_streamer<object_base::serializer_type::json_compact>(*this);
}

std::ostream& object_base::stream(std::ostream& ss) const {

  return ss << object_stream_proxy::object_streamer<object_base::serializer_type::plain>(*this);
}

std::ostream& object_base::stream(serializer_type stype, std::ostream& ss) const {
  switch (stype) {
  case serializer_type::plain:
    return ss << object_stream_proxy::object_streamer<object_base::serializer_type::plain>(*this);
  case serializer_type::quoted:
    return ss << object_stream_proxy::object_streamer<object_base::serializer_type::quoted>(*this);
  case serializer_type::json:
    return ss << object_stream_proxy::object_streamer<object_base::serializer_type::json>(*this);
  case serializer_type::json_compact:
    return ss << object_stream_proxy::object_streamer<object_base::serializer_type::json_compact>(*this);
  case serializer_type::plain_compact:
    return ss << object_stream_proxy::object_streamer<object_base::serializer_type::plain_compact>(*this);
  default:
    return ss;
  }
}

zs::error_result object_base::convert_to_string(zs::string& s) const {
  const object_type type = get_type();

  switch (type) {
  case object_type::k_null:
    s = "null";
    return {};

  case object_type::k_bool:
    s = _bool ? "true" : "false";
    return {};

  case object_type::k_integer: {
    std::basic_ostringstream<char, std::char_traits<char>, zs::allocator<char>> stream(
        std::ios_base::out, s.get_allocator());

    stream << _int;
    s = stream.str();
    return {};
  }

  case object_type::k_float: {
    std::basic_ostringstream<char, std::char_traits<char>, zs::allocator<char>> stream(
        std::ios_base::out, s.get_allocator());

    char buffer[128] = {};

    if (zb::optional_result<size_t> res = zb::to_chars(buffer, 128, _float)) {
      //      stream << buffer;
      //      zb::print("DSLKDSKDLSD", res.value());
      s = std::string_view(buffer, res.value());
      return {};
    }
    //    zb::print("FDLKFJDKFJDLKFDJLKF", stream.width(), _float);
    //    stream << std::setprecision(6) <<std::fixed<< _float;
    //    s = stream.str();
    return zs::error_code::conversion_error;
  }

  case object_type::k_long_string:
    s = zs::string(_lstring->get_string(), s.get_allocator());
    return {};

  case object_type::k_mutable_string:
    s = *_mstring;
    return {};

  case object_type::k_small_string:
    s = _sbuffer;
    return {};

  case object_type::k_string_view: {
    s = zs::string(this->_sview, this->_sview_size, s.get_allocator());
    return {};
  }

  default: {
    std::basic_ostringstream<char, std::char_traits<char>, zs::allocator<char>> stream(
        std::ios_base::out, s.get_allocator());

    int_to_hex(stream, _value);
    s = stream.str();
  }
  }

  return {};
}

zs::error_result object_base::convert_to_string(std::string& s) const {
  const object_type type = get_type();

  switch (type) {
  case object_type::k_null:
    s = "null";
    return {};

  case object_type::k_bool:
    s = _bool ? "true" : "false";
    return {};

  case object_type::k_integer: {
    std::ostringstream stream;
    stream << _int;
    s = stream.str();
    return {};
  }

  case object_type::k_float: {
    std::ostringstream stream;
    stream << _float;
    s = stream.str();
    return {};
  }

  case object_type::k_long_string:
    s = std::string(_lstring->get_string());
    return {};

  case object_type::k_mutable_string:
    s = std::string(_mstring->get_string());
    return {};

  case object_type::k_small_string:
    s = _sbuffer;
    return {};

  case object_type::k_string_view: {
    s = std::string(this->_sview, this->_sview_size);
    return {};
  }

  default: {
    std::ostringstream stream;
    int_to_hex(stream, _value);
    s = stream.str();
  }
  }

  return {};
}

std::string object_base::to_debug_string() const {
  const object_type type = get_type();

  std::basic_ostringstream<char> stream;

  stream << get_object_type_name(type) << " : ";

  switch (type) {
  case object_type::k_null:

    stream << "null";
    break;

  case object_type::k_bool:
    stream << (_bool ? "true" : "false");
    break;

  case object_type::k_integer: {
    stream << _int;
    break;
  }

  case object_type::k_float: {
    stream << _float;
    break;
  }

  case object_type::k_long_string:
    stream << _lstring->get_string();
    break;

  case object_type::k_mutable_string:
    stream << _mstring->get_string();
    break;

  case object_type::k_small_string:
    stream << _sbuffer;
    break;

  case object_type::k_string_view: {
    stream << std::string_view(this->_sview, this->_sview_size);
    break;
  }

  default: {
    stream << "0x" << std::setfill('0') << std::setw(sizeof(_value) * 2) << std::hex << _value;
  }
  }

  return stream.str();
}

zs::error_result object_base::to_debug_string(zs::string& s) const {
  const object_type type = get_type();

  std::basic_ostringstream<char, std::char_traits<char>, zs::allocator<char>> stream(
      std::ios_base::out, s.get_allocator());

  stream << get_object_type_name(type) << " : ";

  switch (type) {
  case object_type::k_null:
    stream << "null";
    break;

  case object_type::k_bool:
    stream << (_bool ? "true" : "false");
    break;

  case object_type::k_integer: {
    stream << _int;
    break;
  }

  case object_type::k_float: {
    stream << _float;
    break;
  }

  case object_type::k_long_string:
    stream << _lstring->get_string();
    break;

  case object_type::k_mutable_string:
    stream << _mstring->get_string();
    break;

  case object_type::k_small_string:
    stream << _sbuffer;
    break;

  case object_type::k_string_view: {
    stream << std::string_view(this->_sview, this->_sview_size);
    break;
  }

  default: {
    stream << "0x" << std::setfill('0') << std::setw(sizeof(_value) * 2) << std::hex << _value;
  }
  }

  s = stream.str();
  return {};
}

size_t object_base::hash() const noexcept {
  switch (_type) {
  case k_null:
    return 0;

  case k_small_string:
    return zb::rapid_hash(get_small_string_unchecked());

  case k_string_view:
    return zb::rapid_hash(get_string_view_unchecked());

  case k_long_string:
    return zb::rapid_hash(get_long_string_unchecked());

  case k_mutable_string:
    return zb::rapid_hash(get_mutable_string_unchecked());

  case k_integer:
    return zb::rapid_hash(_int);

  case k_float:
    return zb::rapid_hash(_float);

  case k_bool:
    return zb::rapid_hash(_bool);

  default:
    return zb::rapid_hash(_value);
  }
}

ZB_CHECK zs::object_unordered_map<object>* object_base::get_table_internal_map() const noexcept {
  if (!is_table()) {
    return nullptr;
  }

  return &(_table->get_map());
}

zs::vector<object>* object_base::get_array_internal_vector() const noexcept {
  if (!is_array()) {
    return nullptr;
  }

  return &(as_array());
}

namespace {
  template <bool CompareNumbers = true, class Op>
  inline bool compare_object(
      Op&& op, bool null_cmp, const object_base& lhs, const object_base& rhs) noexcept {
    using enum object_type;

    // Only strings and number types can be equal to a different type.
    // Otherwise, we compare the types.
    if (lhs._type != rhs._type) {

      if (lhs.is_string() && rhs.is_string()) {
        return op(lhs.get_string_unchecked(), rhs.get_string_unchecked());
      }

      if constexpr (CompareNumbers) {
        if (lhs.is_number() && rhs.is_number()) {
          switch (lhs._type) {
          case k_integer: {
            int_t v2;
            (void)rhs.convert_to_integer(v2);
            return op(lhs._int, v2);
          }
          case k_float: {
            float_t v2;
            (void)rhs.convert_to_float(v2);
            return op(lhs._float, v2);
          }

          default:
            zbase_error("invalid compare");

            return false;
          }
        }
      }

      return op(lhs._type, rhs._type);
    }

    switch (lhs._type) {
    case k_null:
      // Both null.
      return null_cmp;

    case k_bool:
      return op(lhs._bool, rhs._bool);

    case k_integer:
      return op(lhs._int, rhs._int);

    case k_float:
      return op(lhs._float, rhs._float);

    case k_small_string:
      return op(lhs.get_small_string_unchecked(), rhs.get_small_string_unchecked());

    case k_string_view:
      return op(lhs.get_string_view_unchecked(), rhs.get_string_view_unchecked());

    case k_long_string:
      return op(lhs.get_long_string_unchecked(), rhs.get_long_string_unchecked());

    case k_mutable_string:
      return op(lhs.get_mutable_string_unchecked(), rhs.get_mutable_string_unchecked());

    case k_array:
      return op(lhs.as_array(), rhs.as_array());

    default:
      return op(lhs._value, rhs._value);
    }
  }
} // namespace.

bool object_base::strict_equal(const object_base& rhs) const noexcept {
  return compare_object<false>([](const auto& lhs, const auto& rhs) { return lhs == rhs; }, true, *this, rhs);
}

bool object_base::operator==(const object_base& rhs) const noexcept {
  if (_type == k_table and rhs._type == k_table) {
    return as_table() == rhs.as_table();
  }

  return compare_object([](const auto& lhs, const auto& rhs) { return lhs == rhs; }, true, *this, rhs);
}

bool object_base::operator<(const object_base& rhs) const noexcept {
  return compare_object([](const auto& lhs, const auto& rhs) { return lhs < rhs; }, false, *this, rhs);
}

bool object_base::operator>(const object_base& rhs) const noexcept {
  return compare_object([](const auto& lhs, const auto& rhs) { return lhs > rhs; }, false, *this, rhs);
}

bool object_base::operator<=(const object_base& rhs) const noexcept {
  return compare_object([](const auto& lhs, const auto& rhs) { return lhs <= rhs; }, true, *this, rhs);
}

bool object_base::operator>=(const object_base& rhs) const noexcept {
  return compare_object([](const auto& lhs, const auto& rhs) { return lhs >= rhs; }, true, *this, rhs);
}

bool object_base::operator==(std::string_view rhs) const noexcept {
  return is_string() ? (get_string_unchecked() == rhs) : false;
}

int object_base::compare(const object_base& rhs) const noexcept {
  // Only strings types can be equal to a different string type.
  // Otherwise, we compare the types.
  if (_type != rhs._type) {

    if (is_string() && rhs.is_string()) {
      return get_string_unchecked().compare(rhs.get_string_unchecked());
    }

    return _type < rhs._type ? -1 : 1;
  }

  switch (_type) {
  case k_null:
    return 0;

  case k_bool: {
    bool_t v1 = _bool;
    bool_t v2 = rhs._bool;
    return v1 == v2 ? 0 : v2 ? -1 : 1;
  }

  case k_integer: {
    int_t v1 = _int;
    int_t v2 = rhs._int;
    return v1 == v2 ? 0 : v1 < v2 ? -1 : 1;
  }

  case k_float: {
    float_t v1 = _float;
    float_t v2 = rhs._float;
    return v1 == v2 ? 0 : v1 < v2 ? -1 : 1;
  }

  case k_small_string:
    return get_small_string_unchecked().compare(rhs.get_small_string_unchecked());

  case k_string_view:
    return get_string_view_unchecked().compare(rhs.get_string_view_unchecked());

  case k_long_string:
    return get_long_string_unchecked().compare(rhs.get_long_string_unchecked());

  case k_mutable_string:
    return get_mutable_string_unchecked().compare(rhs.get_mutable_string_unchecked());

  default:
    return _value == rhs._value ? 0 : _value < rhs._value ? -1 : 1;
  }
}

//
// MARK: reference_counted
//

namespace {
  struct reference_counted_engine_proxy_tag {};
} // namespace.

template <>
struct internal::proxy<reference_counted_engine_proxy_tag> {
  static inline int_t& global_ref_count(engine* eng) { return eng->_global_ref_count; }
};

namespace {
  using eng_proxy = internal::proxy<reference_counted_engine_proxy_tag>;
}
reference_counted::reference_counted(zs::engine* eng) noexcept
    : engine_holder(eng) {

  eng_proxy::global_ref_count(eng)++;
}

reference_counted::~reference_counted() {
  zbase_assert(_ref_count == 0, "~reference_counted: ref_count should be zero");
}

void reference_counted::retain() noexcept {
  _ref_count++;

  eng_proxy::global_ref_count(_engine)++;
}

bool reference_counted::release() noexcept {
  zbase_assert(_ref_count > 0, "invalid ref count");

  eng_proxy::global_ref_count(_engine)--;

  if (--_ref_count == 0) {
    internal::zs_delete(_engine, this);
    return true;
  }

  return false;
}

//
//
//

virtual_machine* create_virtual_machine(size_t stack_size, allocate_t alloc_cb, raw_pointer_t user_pointer,
    raw_pointer_release_hook_t user_release, stream_getter_t stream_getter) {
  return virtual_machine::create(stack_size, alloc_cb, user_pointer, user_release, stream_getter);
}

virtual_machine* create_virtual_machine(zs::engine* eng, size_t stack_size) {
  return virtual_machine::create(eng, stack_size);
}

void close_virtual_machine(virtual_machine* v) { v->release(); }

zs::engine* get_engine(virtual_machine* v, bool) noexcept { return v->get_engine(); }
} // namespace zs.

#define ZSCRIPT_INCLUDE_ALL_SOURCES 1
#include "zinclude.h"
#undef ZSCRIPT_INCLUDE_ALL_SOURCES
