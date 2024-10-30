#pragma once

#include <zscript/core/common.h>
#include <zscript/core/error.h>
#include <zscript/core/line_info.h>
#include <zscript/core/version.h>
#include <zscript/core/types.h>
#include <zscript/core/object_type.h>
#include <zscript/core/api.h>
#include <zscript/core/memory.h>
#include <zscript/core/engine_holder.h>
#include <zscript/core/object_base.h>
#include <zscript/core/object.h>
#include <zscript/core/engine.h>
#include <zscript/core/vm.h>

namespace zs {

zs::object closure_object_t::call(zs::vm_ref vm, zb::span<const object> params) { return (*fct)(vm, params); }

zs::object closure_object_t::call(zs::vm_ref vm, std::initializer_list<const object> params) {
  return (*fct)(vm, zb::span<const object>(params));
}

template <class T>
ZS_CHECK inline constexpr T* allocator<T>::allocate(size_t n) {
  if (ZBASE_UNLIKELY(n > std::allocator_traits<allocator>::max_size(*this))) {
    zs::throw_exception(zs::error_code::out_of_memory);
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

ZS_CXPR object object::create_native_function(zs::native_cpp_closure_t fct) noexcept { return object(fct); }

template <class Fct>
object object::create_native_closure_function(zs::engine* eng, Fct&& fct) {
  if constexpr (zb::is_function_pointer_v<Fct>) {
    using fct_ptr_type = zb::function_pointer_type_t<Fct>;
    return create_native_closure_function(eng, (fct_ptr_type)fct);
  }
  else {
    using traits = zb::function_traits<Fct>;
    using R = typename traits::result_type;
    using class_type = typename traits::class_type;
    using args_list = typename traits::args_list;
    return create_native_closure_function<class_type, R>(eng, std::forward<Fct>(fct), args_list{});
  }
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
    zs::throw_exception(zs::error_code::invalid_type);
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

} // namespace zs.
