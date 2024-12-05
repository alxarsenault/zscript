// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zscript/common.h>
#include <zscript/error.h>
#include <zscript/line_info.h>
#include <zscript/version.h>
#include <zscript/types.h>
#include <zscript/object_type.h>
#include <zscript/memory.h>
#include <zscript/engine_holder.h>
#include <zscript/object_base.h>
#include <zscript/object.h>
#include <zscript/engine.h>
#include <zscript/vm.h>
#include <zscript/object_function_wrapper.h>

#include <zbase/container/byte.h>
#include <zbase/sys/file_view.h>
#include <zbase/strings/string_view.h>
#include <zbase/utility/print.h>
#include <zbase/utility/traits.h>

#include <filesystem>
#include <string>
#include <stdexcept>

#define ZS_FUNCTION_DEF +[](zs::vm_ref vm) -> int_t

namespace zs {

#define ZS_DEVELOPER_SOURCE_LOCATION(...) \
  zs::developer_source_location { __FILE__, __PRETTY_FUNCTION__, __LINE__, ZBASE_STRINGIFY(__VA_ARGS__) }

struct developer_source_location {
  std::string_view _filename;
  std::string_view _function_name;
  std::uint_least32_t _line;
  std::string_view _line_content;

  ZB_CK_INLINE std::string_view file_name() const noexcept { return _filename; }

  ZB_CK_INLINE std::string_view function_name() const noexcept { return _function_name; }

  ZB_CK_INLINE std::uint_least32_t line() const noexcept { return _line; }

  ZS_CK_INLINE std::string_view line_content() const noexcept { return _line_content; }

  inline friend std::ostream& operator<<(std::ostream& stream, const developer_source_location& loc) {

    stream << "\ndeveloper:\nfile: '" << loc.file_name() << "'\nfunction: '" << loc.function_name()
           << "'\nline: " << loc.line() << "\n";

    if (!loc.line_content().empty()) {
      stream << "content: '" << loc.line_content() << "'\n";
    }

    return stream;
  }
};
enum class error_source { compiler, virtual_machine };
struct error_message {

  template <class Message, class Filename, class Code>
  inline error_message(zs::engine* eng, error_source esrc, zs::error_code ec, Message&& message,
      Filename&& filename, Code&& code, zs::line_info line, const zs::developer_source_location& loc)
      : ec(ec)
      , err_source(esrc)
      , message(std::forward<Message>(message), eng)
      , filename(std::forward<Filename>(filename), eng)
      , code(std::forward<Code>(code), eng)
      , line(line)
      , loc(loc) {}

  inline std::ostream& print(std::ostream& stream) const {

    stream << "error: " << message;

    if (!message.ends_with('\n')) {
      stream << "\n";
    }

    zb::stream_print(stream, "error-source: ", err_source, "\n");

    if (filename.empty()) {
      stream << "file: unknown\n";
    }
    else {
      stream << "file: " << filename << "\n";
    }

    if (line.line != -1) {
      stream << "line: " << line.line << ":" << line.column << "\n\n";
    }
    else {
      stream << "line: unknown\n\n";
    }

    if (!code.empty()) {
      stream << "'''" << code << "\n\n'''\n";
    }

    stream << "\ndeveloper:\nfile: '" << loc.file_name() << "'\nfunction: '" << loc.function_name()
           << "'\nline: " << loc.line() << "\n";

    if (!loc.line_content().empty()) {
      stream << "content: '" << loc.line_content() << "'\n";
    }

    return stream;
  }

  zs::error_code ec;
  zs::error_source err_source;
  zs::string message;
  zs::string filename;
  zs::string code;
  zs::line_info line;
  zs::developer_source_location loc;
};

struct error_stack : zs::vector<error_message> {
  error_stack(zs::engine* eng)
      : zs::vector<error_message>((zs::allocator<error_message>(eng))) {}

  inline std::ostream& print(std::ostream& stream) const {
    for (const auto& e : *this) {
      e.print(stream);
    }

    return stream;
  }
};

template <class T>
ZS_CHECK inline constexpr T* allocator<T>::allocate(size_t n) {
  if (ZBASE_UNLIKELY(n > std::allocator_traits<allocator>::max_size(*this))) {
    zs::throw_error(zs::error_code::out_of_memory);
  }

  return static_cast<T*>(
      zs::allocate(_engine, n * sizeof(T), (alloc_info_t)ZS_IF_MEMORY_PROFILER_OR(_tag, 0)));
}

template <class T>
ZS_INLINE_CXPR void allocator<T>::deallocate(T* ptr, size_t) noexcept {
  zs::deallocate(_engine, ptr, (alloc_info_t)ZS_IF_MEMORY_PROFILER_OR(_tag, 0));
}

template <class T>
ZS_INLINE constexpr void detail::unique_ptr_deleter<T>::operator()(T* ptr) const noexcept {
  static_assert(sizeof(T) >= 0, "cannot delete an incomplete type");
  static_assert(!std::is_void_v<T>, "cannot delete an incomplete type");
  internal::zs_delete(_engine, ptr);
}

size_t object_table_hash::operator()(const object_base& obj) const noexcept { return obj.hash(); }

size_t object_table_hash::operator()(std::string_view s) const noexcept { return zb::rapid_hash(s); }

size_t object_table_hash::operator()(const std::string& s) const noexcept { return zb::rapid_hash(s); }

size_t object_table_hash::operator()(const char* s) const noexcept { return zb::rapid_hash(s); }

bool object_table_equal_to::operator()(const object_base& lhs, const object_base& rhs) const noexcept {
  return lhs.strict_equal(rhs);
}

bool object_table_equal_to::operator()(const object_base& lhs, std::string_view rhs) const noexcept {
  return this->operator()(lhs, zs::_sv(rhs));
}

bool object_table_equal_to::operator()(std::string_view lhs, const object_base& rhs) const noexcept {
  return this->operator()(zs::_sv(lhs), rhs);
}

template <class T>
object_type object::get_value_conv_obj_type() noexcept {

  using value_type = std::remove_cvref_t<T>;

  if constexpr (std::is_same_v<T, object>) {
    return object_type::k_none;
  }

  else if constexpr (std::is_same_v<bool_t, value_type>) {
    return object_type::k_bool;
  }
  else if constexpr (std::is_same_v<std::nullptr_t, value_type>) {
    return object_type::k_null;
  }
  else if constexpr (std::is_same_v<std::string, T>) {
    return object_type::k_small_string;
  }
  else if constexpr (std::is_constructible_v<std::string_view, T>) {
    return object_type::k_small_string;
  }
  else if constexpr (std::is_floating_point_v<value_type>) {
    return object_type::k_float;
  }
  else if constexpr (std::is_integral_v<value_type>) {
    return object_type::k_integer;
  }

  else if constexpr (zb::is_contiguous_container_v<value_type>) {
    return object_type::k_array;
  }
  else if constexpr (zb::is_map_type_v<value_type>) {
    return object_type::k_table;
  }
  else {
    zb_static_error("invalid type");
    return object_type::k_none;
  }
}

template <class T>
constexpr const char* object::get_value_conv_obj_name() noexcept {

  using value_type = std::remove_cvref_t<T>;

  if constexpr (std::is_same_v<T, object>) {
    return "object";
  }
  else if constexpr (std::is_same_v<bool_t, value_type>) {
    return "bool";
  }
  else if constexpr (std::is_same_v<std::nullptr_t, value_type>) {
    return "null";
  }
  else if constexpr (std::is_same_v<std::string, T>) {
    return "string";
  }
  else if constexpr (std::is_constructible_v<std::string_view, T>) {
    return "string";
  }
  else if constexpr (std::is_floating_point_v<value_type>) {
    return "float";
  }
  else if constexpr (std::is_integral_v<value_type>) {
    return "integer";
  }
  else if constexpr (zb::is_contiguous_container_v<value_type>) {
    return "array";
  }
  else if constexpr (zb::is_map_type_v<value_type>) {
    return "table";
  }
  else {
    //    zb_static_error("invalid type");
    return "unknown";
  }
}

template <class T>
inline zs::error_result object::get_value(T& value) const noexcept {

  using value_type = std::remove_cvref_t<T>;

  if constexpr (std::is_same_v<T, object>) {
    value = *this;
    return {};
  }
  else if constexpr (std::is_same_v<bool_t, value_type>) {
    return get_bool(value);
  }
  else if constexpr (std::is_same_v<std::nullptr_t, value_type>) {
    return {};
  }
  else if constexpr (std::is_same_v<std::string, T>) {
    std::string_view s;
    if (auto err = get_string(s)) {
      value = convert_to_string();
      return {};
    }

    value = value_type(s);
    return {};
  }
  else if constexpr (std::is_constructible_v<std::string_view, decltype(std::forward<T>(value))>) {
    return get_string(value);
  }
  else if constexpr (std::is_floating_point_v<value_type>) {
    float_t f = 0;

    if (auto err = get_float(f)) {
      return err;
    }

    value = (value_type)f;
    return {};
  }
  else if constexpr (std::is_integral_v<value_type>) {
    int_t i = 0;

    if (auto err = get_integer(i)) {
      return err;
    }

    value = (value_type)i;
    return {};
  }
  else if constexpr (zb::is_contiguous_container_v<value_type>) {
    if (get_type() != k_array) {
      return zs::error_code::invalid_type;
    }
    using containter_value_type = zb::container_value_type_t<value_type>;

    if constexpr (zb::has_push_back_v<value_type>) {

      if constexpr (zb::is_string_view_convertible_v<containter_value_type>) {

        const auto& vec = *get_array_internal_vector();
        for (size_t i = 0; i < vec.size(); i++) {
          value.push_back(vec[i].get_value<containter_value_type>());
        }
      }
      else {

        const auto& vec = *get_array_internal_vector();
        for (size_t i = 0; i < vec.size(); i++) {
          value.push_back(vec[i].get_value<containter_value_type>());
        }
      }

      return {};
    }
    else if constexpr (zb::is_array_v<value_type>) {

      const auto& vec = *get_array_internal_vector();

      if (zb::array_size_v<value_type> >= vec.size()) {
        for (size_t i = 0; i < vec.size(); i++) {
          value[i] = vec[i].get_value<containter_value_type>();
        }

        return {};
      }
      else {
        for (size_t i = 0; i < zb::array_size_v<value_type>; i++) {
          value[i] = vec[i].get_value<containter_value_type>();
        }

        return {};
      }
    }
    else {
      zb_static_error("invalid type no push_back");
      return zs::error_code::invalid_type;
    }
    return {};
  }

  else if constexpr (zb::is_map_type_v<value_type>) {
    if (get_type() != k_table) {
      return zs::error_code::invalid_type;
    }

    using key_type = typename value_type::key_type;
    using mapped_type = typename value_type::mapped_type;

    const auto& map = *get_table_internal_map();
    for (auto obj : map) {
      value[obj.first.get_value<key_type>()] = obj.second.get_value<mapped_type>();
    }
    return {};
  }

  else {
    if (get_type() == k_user_data) {
      return copy_user_data_to_type((void*)&value, sizeof(T), typeid(T).name());
    }

    return zs::error_code::invalid_type;
  }
}

template <class T>
T object::get_value() const {
  T value;
  if (auto err = get_value(value)) {
    zs::throw_error(zs::error_code::invalid_type);
  }

  return value;
}

template <class T>
T object::get_value(const T& opt_value) const {
  T value;
  if (auto err = get_value(value)) {
    return opt_value;
  }

  return value;
}

template <class VectorType>
zs::error_result object::to_binary(VectorType& buffer, size_t& write_size, uint32_t flags) const {
  return to_binary(
      (write_function_t)[](const uint8_t* content, size_t size, void* udata)->zs::error_result {
        VectorType& vec = *(VectorType*)udata;

        vec.insert(vec.end(), content, content + size);
        return zs::error_code::success;
      },
      write_size, &buffer, flags);
}

std::ostream& operator<<(std::ostream& stream, const zs::object_base& obj) {
  return obj.stream_to_string(stream);
}

namespace constants {

  template <meta_method MetaMethod>
  ZS_CK_INLINE zs::object get() noexcept {
    if constexpr (false) {
      zb_static_error("invalid meta_method");
      return nullptr;
    }

#define _X(name, str)                                   \
  else if constexpr (MetaMethod == meta_method::name) { \
    return zs::_sv(constants::k_##name##_string);       \
  }
    ZS_META_METHOD_ENUM(_X)
#undef _X

    else {

      zb_static_error("invalid meta_method");
      return nullptr;
    }
  }
} // namespace constants

inline constexpr size_t default_stack_size = 1024;

namespace constants {
  inline constexpr bool k_is_object_stack_resizable = false;
  //  inline constexpr size_t default_stack_size = 1024;
} // namespace constants.

//

/// Default file loader.
class default_file_loader {
public:
  ZS_INLINE default_file_loader(ZBASE_MAYBE_UNUSED zs::engine* eng) noexcept {}

  ZS_CHECK zs::error_result open(const char* filepath) noexcept;

  ZS_CHECK zs::error_result open(std::string_view filepath) noexcept;

  ZS_CHECK zs::error_result open(const object& filepath) noexcept;

  template <class _Allocator>
  ZS_CK_INLINE zs::error_result open(
      const std::basic_string<char, std::char_traits<char>, _Allocator>& filepath) noexcept {
    return this->open(filepath.c_str());
  }

  ZS_CK_INLINE std::string_view content() const noexcept { return _fv.str(); }

  ZS_CK_INLINE size_t size() const noexcept { return _fv.size(); }

  ZS_CK_INLINE zb::byte_view data() const noexcept { return _fv.content(); }

private:
  zb::file_view _fv;
};

using file_loader = ZS_DEFAULT_FILE_LOADER;

//
// MARK: - API
//

ZS_CHECK virtual_machine* create_virtual_machine(size_t stack_size = ZS_DEFAULT_STACK_SIZE,
    allocate_t alloc_cb = ZS_DEFAULT_ALLOCATE, raw_pointer_t user_pointer = nullptr,
    raw_pointer_release_hook_t user_release = nullptr,
    stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER,
    engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER);

ZS_CHECK virtual_machine* create_virtual_machine(zs::engine* eng, size_t stack_size = ZS_DEFAULT_STACK_SIZE);

void close_virtual_machine(virtual_machine* v);

/// Call a callable.
ZS_CHECK ZS_API zs::error_result zs_call(virtual_machine* v, int_t n_params, bool returns, bool pop_callable);

ZS_API void zs_push(virtual_machine* v);

//
// MARK: Convert to std::string
//

//
// zs::string::operator==
//

ZS_CK_INLINE bool operator==(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) == rhs;
}

ZS_CK_INLINE bool operator==(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) == rhs;
}

//
// zs::string::operator!=
//

ZS_CK_INLINE bool operator!=(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) != (const std::string&)rhs;
}

ZS_CK_INLINE bool operator!=(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) != (const std::string&)rhs;
}

//
// zs::string::operator<
//

ZS_CK_INLINE bool operator<(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) < rhs;
}

ZS_CK_INLINE bool operator<(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) < (const std::string&)rhs;
}

//
// zs::string::operator<=
//

ZS_CK_INLINE bool operator<=(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) <= rhs;
}

ZS_CK_INLINE bool operator<=(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) <= (const std::string&)rhs;
}

//
// zs::string::operator>
//

ZS_CK_INLINE bool operator>(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) > rhs;
}

ZS_CK_INLINE bool operator>(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) > (const std::string&)rhs;
}

//
// zs::string::operator>=
//

ZS_CK_INLINE bool operator>=(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) >= rhs;
}

ZS_CK_INLINE bool operator>=(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) >= (const std::string&)rhs;
}

//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// MARK: - Implementation
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <class Fct>
object object::create_native_closure(zs::engine* eng, Fct&& fct) {

  if constexpr (std::is_convertible_v<Fct, int_t (*)(virtual_machine*)>
      or std::is_convertible_v<Fct, int_t (*)(const virtual_machine*)>) {
    zb_static_error("Invalid native closure function");
    return 0;
  }
  else if constexpr (std::is_convertible_v<Fct, zs::function_t>) {
    return create_native_closure(eng, (zs::function_t)fct);
  }
  else {
    struct closure_type : native_closure {

      inline closure_type(zs::engine* eng, Fct&& fct)
          : native_closure(eng)
          , _fct(std::forward<Fct>(fct)) {}

      virtual ~closure_type() override = default;

      virtual int_t call(zs::vm_ref vm) override {
        if constexpr (std::is_invocable_v<Fct, vm_ref>) {
          return _fct(vm);
        }
        else if constexpr (std::is_invocable_v<Fct>) {
          return _fct();
        }
        else {
          zb_static_error("can't call function");
          return 0;
        }
      }

      Fct _fct;
    };

    closure_type* nc = (closure_type*)allocate(eng, sizeof(closure_type));
    nc = zb_placement_new(nc) closure_type(eng, std::forward<Fct>(fct));
    return create_native_closure(eng, (zs::native_closure*)nc);
  }
}

template <class Fct>
zs::error_result vm_ref::new_closure(Fct&& fct) {

  struct closure_type : native_closure {

    inline closure_type(zs::engine* eng, Fct&& fct)
        : native_closure(eng)
        , _fct(std::forward<Fct>(fct)) {}

    virtual ~closure_type() override = default;

    virtual int_t call(zs::vm_ref vm) override {
      if constexpr (std::is_invocable_v<Fct, vm_ref>) {
        return _fct(vm);
      }
      else if constexpr (std::is_invocable_v<Fct>) {
        return _fct();
      }
      else {
        zb_static_error("can't call function");
        return -1;
      }
    }

    Fct _fct;
  };

  closure_type* nc = (closure_type*)zs::allocate(get_engine(), sizeof(closure_type));
  nc = zb_placement_new(nc) closure_type(get_engine(), std::forward<Fct>(fct));
  return new_closure((zs::native_closure*)nc);
}

//
////
//// MARK: Shortcut aliases.
////
//

template <class T>
object& object_unordered_map<T>::operator[](std::string_view s) {
  return base_type::operator[](zs::_s(base_type::get_allocator().get_engine(), s));
}

template <class T>
object& object_unordered_map<T>::operator[](const char* s) {
  return base_type::operator[](zs::_s(base_type::get_allocator().get_engine(), s));
}

template <class T>
template <size_t N>
object& object_unordered_map<T>::operator[](const char (&s)[N]) noexcept {
  return base_type::operator[](zs::_ss(s));
}

} // namespace zs.
