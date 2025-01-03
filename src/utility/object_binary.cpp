#include <zscript/utility/object_binary.h>
#include "zvirtual_machine.h"
#include "object/zfunction_prototype.h"
#include "jit/zjit_compiler.h"
#include "utility/json/zjson_lexer.h"
#include "utility/json/zjson_parser.h"

namespace zs {

zs::error_result get_binary_size(const object& obj, size_t& write_size) {
  write_size = 0;
  using enum object_type;

  switch (obj.get_type()) {

  case k_null:

  case k_bool:
  case k_integer:
  case k_float:
  case k_small_string:
    write_size += sizeof(object_base);
    return {};

  case k_long_string: {
    std::string_view s = obj.get_long_string_unchecked();
    write_size += sizeof(object_base) + (s.size() <= constants::k_small_string_max_size ? 0 : s.size());
    return {};
  }
  case k_string_view: {
    std::string_view s = obj.get_string_view_unchecked();
    write_size += sizeof(object_base) + (s.size() <= constants::k_small_string_max_size ? 0 : s.size());
    return {};
  }
  case k_array: {
    const auto& arr = obj.as_array().to_vec();
    write_size += sizeof(object_base);

    for (const auto& item : arr) {
      size_t sz = 0;
      if (auto err = get_binary_size(item, sz)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }

  case k_table: {
    const auto& map = obj._table->get_map();
    write_size += sizeof(object_base);

    for (auto item : map) {
      size_t sz = 0;
      if (auto err = get_binary_size(item.first, sz)) {
        return err;
      }

      write_size += sz;

      sz = 0;
      if (auto err = get_binary_size(item.second, sz)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }
  default:
    return errc::invalid_operation;
  }
  return {};
}

namespace {
  template <class T>
  inline zs::error_result write_binary_data(
      const T& obj, write_function_t write_func, size_t& write_size, void* data) {
    return write_func(
        (const uint8_t*)obj.data(), write_size = obj.size() * sizeof(typename T::value_type), data);
  }

  inline zs::error_result write_object(
      const object_base& obj, write_function_t write_func, size_t& write_size, void* data) {
    return write_func((const uint8_t*)&obj, write_size = sizeof(object_base), data);
  }
} // namespace

zs::error_result to_binary(
    const object& obj, write_function_t write_func, size_t& write_size, void* data, uint32_t flags) {
  using enum object_type;
  write_size = 0;

  switch (obj.get_type()) {
  case k_null:

  case k_bool:
  case k_integer:
  case k_float:
  case k_small_string:
    return write_object(obj, write_func, write_size, data);

  case k_long_string: {
    std::string_view s = obj._lstring->get_string();

    if (s.size() <= constants::k_small_string_max_size) {
      return to_binary(zs::_ss(s), write_func, write_size, data);
    }

    object_base bobj = obj;
    bobj._lvalue = s.size();

    if (auto err = write_object(bobj, write_func, write_size, data)) {
      return err;
    }

    if (auto err = write_binary_data(s, write_func, write_size, data)) {
      return err;
    }

    return {};
  }
  case k_string_view: {
    std::string_view s = obj.get_string_view_unchecked();

    if (s.size() <= constants::k_small_string_max_size) {
      return to_binary(zs::_ss(s), write_func, write_size, data);
    }

    object_base objbase = obj;
    objbase._lvalue = s.size();

    if (auto err = write_object(objbase, write_func, write_size, data)) {
      return err;
    }

    if (auto err = write_binary_data(s, write_func, write_size, data)) {
      return err;
    }
    return {};
  }

  case k_array: {
    const auto& vec = obj.as_array().to_vec();
    size_t size = vec.size();

    object_base objbase = obj;
    objbase._lvalue = size;
    if (auto err = write_object(objbase, write_func, write_size, data)) {
      return err;
    }

    for (const auto& item : vec) {
      size_t sz = 0;
      if (auto err = to_binary(item, write_func, sz, data, flags)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }

  case k_table: {
    const auto& map = obj._table->get_map();
    size_t size = map.size();

    object_base objbase = obj;
    objbase._lvalue = size;
    if (auto err = write_object(objbase, write_func, write_size, data)) {
      return err;
    }

    for (auto item : map) {
      size_t sz = 0;
      if (auto err = to_binary(item.first, write_func, sz, data, flags)) {
        return err;
      }

      write_size += sz;

      sz = 0;
      if (auto err = to_binary(item.second, write_func, sz, data, flags)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }

  default:
    return errc::invalid_operation;
  }
  return {};
}

zs::error_result from_binary(zs::engine* eng, std::span<uint8_t> buffer, object& out, size_t& offset) {
  using enum object_type;

  size_t buff_sz = buffer.size();
  if (buff_sz < sizeof(object_base)) {
    return errc::invalid_argument;
  }

  object_base objbase;
  zb::memset(&objbase, 0, sizeof(object));
  objbase._type = k_null;

  zb::memcpy(&objbase, buffer.data(), sizeof(object_base));

  offset += sizeof(object_base);

  switch (objbase._type) {
  case k_null:

  case k_bool:
  case k_integer:
  case k_float:
  case k_small_string:
    out = object(objbase, false);
    return {};

  case k_long_string:
  case k_string_view: {
    size_t slen = objbase._lvalue;
    if (buff_sz < slen) {
      return errc::invalid_argument;
    }
    out = zs::_s(eng, std::string_view((const char*)buffer.data() + offset, slen));
    offset += slen;
    return {};
  }

  case k_array: {

    size_t len = objbase._lvalue;
    out = zs::object::create_array(eng, len);
    zs::vector<object>& vec = out.as_array().to_vec();

    for (size_t i = 0; i < len; i++) {
      size_t off = 0;
      if (auto err = from_binary(eng, buffer.subspan(offset), vec[i], off)) {
        return err;
      }

      offset += off;
    }
    return {};
  }

  case k_table: {

    size_t len = objbase._lvalue;
    out = zs::object::create_table(eng);
    auto& map = out._table->get_map();

    for (size_t i = 0; i < len; i++) {
      size_t off = 0;

      zs::object key;
      if (auto err = from_binary(eng, buffer.subspan(offset), key, off)) {
        return err;
      }

      offset += off;

      off = 0;

      zs::object value;
      if (auto err = from_binary(eng, buffer.subspan(offset), value, off)) {
        return err;
      }

      offset += off;

      map[std::move(key)] = std::move(value);
    }
    return {};
  }

  default:
    return errc::invalid_operation;
  }

  return {};
}
} // namespace zs
