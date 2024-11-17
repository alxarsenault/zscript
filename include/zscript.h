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

#include <zbase/sys/file_view.h>
#include <zbase/strings/string_view.h>
#include <zbase/utility/print.h>
#include <zbase/utility/traits.h>

#include <filesystem>
#include <string>
#include <stdexcept>

#define ZS_FUNCTION_DEF +[](zs::vm_ref vm) -> int_t

namespace zs {

zs::object closure_object_t::call(zs::vm_ref vm, zb::span<const object> params) { return (*fct)(vm, params); }

zs::object closure_object_t::call(zs::vm_ref vm, std::initializer_list<const object> params) {
  return (*fct)(vm, zb::span<const object>(params));
}

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
  inline constexpr bool k_is_object_stack_resizable = true;
  //  inline constexpr size_t default_stack_size = 1024;
} // namespace constants.

//

/// Default file loader.
class default_file_loader {
public:
  ZS_INLINE default_file_loader(ZBASE_MAYBE_UNUSED zs::engine* eng) noexcept {}

  ZS_CHECK zs::error_result open(const char* filepath) noexcept;

  ZS_CHECK zs::error_result open(std::string_view filepath) noexcept;

  template <class _Allocator>
  ZS_CK_INLINE zs::error_result open(
      const std::basic_string<char, std::char_traits<char>, _Allocator>& filepath) noexcept {
    return this->open(filepath.c_str());
  }

  ZS_CK_INLINE std::string_view content() const noexcept { return _fv.str(); }

  ZS_CK_INLINE size_t size() const noexcept { return _fv.size(); }

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

  if constexpr (std::is_convertible_v<Fct, zs::native_cclosure_t>) {
    return create_native_closure(eng, (zs::native_cclosure_t)fct);
  }
  else if constexpr (std::is_convertible_v<Fct, zs::native_cpp_closure_t>) {
    return create_native_closure(eng, (zs::native_cpp_closure_t)fct);
  }
  else {
    struct closure_type : native_closure {

      inline closure_type(zs::engine* eng, Fct&& fct)
          : native_closure(eng)
          , _fct(std::forward<Fct>(fct)) {}

      virtual ~closure_type() override = default;

      virtual int_t call(virtual_machine* v) override {
        if constexpr (std::is_invocable_v<Fct, vm_ref>) {
          return _fct(vm_ref(v));
        }
        else if constexpr (std::is_invocable_v<Fct, virtual_machine*>) {
          return _fct(v);
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

template <size_t I = 0, bool StartsWithVm = false, class... Args>
inline zs::error_result fill_function_arg_tuple(vm_ref vm, std::tuple<Args...>& tup) {
  using args_list = zb::type_list<Args...>;

  if constexpr (I >= sizeof...(Args)) {
    return {};
  }
  else {

    using vtype = typename args_list::template type_at_index<I>;
    vtype v;

    if (auto err = vm[I + 1].get_value<vtype>(v)) {
      zbase_error("Got ", zb::quoted(zs::get_object_type_name(vm[I + 1].get_type())), " expected ",
          zb::quoted(zs::var::get_value_conv_obj_name<vtype>()), " aka : ", zb::quoted(typeid(vtype).name()));
      return err;
    }

    std::get<I>(tup) = std::move(v);
    if constexpr (I + 1 < sizeof...(Args)) {
      return fill_function_arg_tuple<I + 1, StartsWithVm>(vm, tup);
    }
    else {
      return {};
    }
  }
}

template <class... Args>
inline static constexpr bool function_pointer_starts_with_vm_ref() noexcept {
  if constexpr (sizeof...(Args) == 0) {
    return false;
  }
  else {
    using args_list = zb::type_list<Args...>;
    return std::is_same_v<typename args_list::template type_at_index<0>, zs::vm_ref>;
  }
}

template <class... Args>
inline static constexpr bool function_pointer_starts_with_virtual_machine_ptr() noexcept {
  if constexpr (sizeof...(Args) == 0) {
    return false;
  }
  else {
    using args_list = zb::type_list<Args...>;
    return std::is_same_v<typename args_list::template type_at_index<0>, zs::virtual_machine*>;
  }
}

namespace object_func_detail {
  template <class F, class VM, class Tuple, std::size_t... I>
  constexpr decltype(auto) apply_impl(F&& f, VM vm, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), vm, std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class F, class Tuple, std::size_t... I>
  constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class F, class Tuple>
  constexpr decltype(auto) apply(F&& f, Tuple&& t) {
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <class F, class VM, class Tuple>
  constexpr decltype(auto) apply(F&& f, VM vm, Tuple&& t) {
    return apply_impl(std::forward<F>(f), vm, std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <class ClassType, class F, class VM, class Tuple, std::size_t... I>
  constexpr decltype(auto) apply_member_impl(
      ClassType* obj, F&& f, VM vm, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), obj, vm, std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class ClassType, class F, class Tuple, std::size_t... I>
  constexpr decltype(auto) apply_member_impl(ClassType* obj, F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), obj, std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class F, class ClassType, class Tuple>
  constexpr decltype(auto) apply_member_fct(F&& f, ClassType* obj, Tuple&& t) {
    return apply_member_impl(obj, std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <class F, class ClassType, class VM, class Tuple>
  constexpr decltype(auto) apply_member_fct(F&& f, ClassType* obj, VM vm, Tuple&& t) {
    return apply_member_impl(obj, std::forward<F>(f), vm, std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }
} // namespace object_func_detail

// Function pointer.
template <class R, class... Args>
object object::create_native_closure_function(zs::engine* eng, zb::function_pointer<R, Args...> fct) {

  using args_list = zb::type_list<Args...>;
  constexpr size_t n_fct_pointer_args = args_list::size();
  constexpr bool starts_with_vm_ref = function_pointer_starts_with_vm_ref<Args...>();
  constexpr bool starts_with_virtual_machine_ptr
      = function_pointer_starts_with_virtual_machine_ptr<Args...>();

  // If the function pointer starts with a vm_ref, we shift everything by one.
  if constexpr (starts_with_vm_ref || starts_with_virtual_machine_ptr) {

    return create_native_closure(eng, [f = fct](vm_ref vm) mutable -> zs::int_t {
      // Number of arguments required by the function pointer (excluding vm_ref).
      constexpr size_t n_args = n_fct_pointer_args - 1;
      using params_list = typename args_list::template n_last_list<n_args>;

      // Make sure there is enough args on the stack.
      // We ignore the first 'this' arg.
      if (vm.stack_size() - 1 != (zs::int_t)n_args) {
        zb::print("Invalid native function call args size", vm.stack_size() - 1, n_args);
        return -1;
      }

      using tuple_type = typename params_list::tuple_type_no_ref;
      tuple_type tup;
      if (auto err = fill_function_arg_tuple(vm, tup)) {
        zb::print("Invalid native function call type");
        return -1;
      }

      // Call function - No return type.
      if constexpr (std::is_same_v<R, void>) {

        if constexpr (starts_with_vm_ref) {
          object_func_detail::apply(f, vm, std::move(tup));
        }
        else {
          object_func_detail::apply(f, vm.get_virtual_machine(), std::move(tup));
        }

        return 0;
      }

      // Call function - With return type.
      else {
        if constexpr (starts_with_vm_ref) {
          auto ret = zs::var(vm.get_engine(), object_func_detail::apply(f, vm, std::move(tup)));
          vm.push(ret);
        }
        else {
          auto ret = zs::var(
              vm.get_engine(), object_func_detail::apply(f, vm.get_virtual_machine(), std::move(tup)));
          vm.push(ret);
        }

        return 1;
      }
    });
  }
  else {
    return create_native_closure(eng, [f = fct](vm_ref vm) mutable -> zs::int_t {
      // Number of arguments required by the function pointer (excluding vm_ref).
      constexpr size_t n_args = n_fct_pointer_args;
      using params_list = args_list;

      // Make sure there is enough args on the stack.
      // We ignore the first 'this' arg.
      if (vm.stack_size() <= (zs::int_t)n_args) {
        zb::print("Invalid native function call args size");
        return -1;
      }

      using tuple_type = typename params_list::tuple_type_no_ref;
      tuple_type tup;
      if (auto err = fill_function_arg_tuple(vm, tup)) {

        zb::print("Invalid native function call type");
        return -1;
      }

      // Call function - No return type.
      if constexpr (std::is_same_v<R, void>) {
        object_func_detail::apply(f, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        auto ret = zs::var(vm.get_engine(), object_func_detail::apply(f, std::move(tup)));
        vm.push(ret);
        return 1;
      }
    });
  }
}

// Any member function pointer.
template <class Fct, class ClassType, class R, class... Args>
object object::create_native_closure_function(zs::engine* eng,
    zb::member_function_pointer_wrapper<Fct, ClassType, R, Args...> fct, const zs::object& uid) {

  using args_list = zb::type_list<Args...>;
  constexpr size_t n_fct_pointer_args = args_list::size();
  constexpr bool starts_with_vm_ref = function_pointer_starts_with_vm_ref<Args...>();
  constexpr bool starts_with_virtual_machine_ptr
      = function_pointer_starts_with_virtual_machine_ptr<Args...>();

  // If the function pointer starts with a vm_ref, we shift everything by one.
  if constexpr (starts_with_vm_ref || starts_with_virtual_machine_ptr) {

    return create_native_closure(eng, [f = fct.fct, u = uid](vm_ref vm) mutable -> zs::int_t {
      if (vm.stack_size() < 1) {
        zb::print("Invalid native function call args size");
        return -1;
      }

      if (!vm[0].is_user_data()) {
        zb::print("Invalid data type in function call, expected user data");
        return -1;
      }

      if (!u.is_null() && !vm[0].has_user_data_uid(u)) {
        zb::print("Invalid uid type in function call");
        return -1;
      }

      ClassType* udata = (ClassType*)vm[0].get_user_data_data();

      // Number of arguments required by the function pointer (excluding
      // vm_ref).
      constexpr size_t n_args = n_fct_pointer_args - 1;
      using params_list = typename args_list::template n_last_list<n_args>;

      // Make sure there is enough args on the stack.
      // We ignore the first 'this' arg.
      if (vm.stack_size() - 1 != (zs::int_t)n_args) {
        zb::print("Invalid native function call args size", vm.stack_size() - 1, n_args);
        return -1;
      }

      using tuple_type = typename params_list::tuple_type_no_ref;
      tuple_type tup;
      if (auto err = fill_function_arg_tuple(vm, tup)) {
        zb::print("Invalid native function call type");
        return -1;
      }

      // Call function - No return type.
      if constexpr (std::is_same_v<R, void>) {
        if constexpr (starts_with_vm_ref) {
          object_func_detail::apply_member_fct(f, udata, vm, std::move(tup));
        }
        else {
          object_func_detail::apply_member_fct(f, udata, vm.get_virtual_machine(), std::move(tup));
        }
        return 0;
      }

      // Call function - With return type.
      else {
        if constexpr (starts_with_vm_ref) {
          auto ret
              = zs::var(vm.get_engine(), object_func_detail::apply_member_fct(f, udata, vm, std::move(tup)));
          vm.push(ret);
        }
        else {
          auto ret = zs::var(vm.get_engine(),
              object_func_detail::apply_member_fct(f, udata, vm.get_virtual_machine(), std::move(tup)));
          vm.push(ret);
        }

        return 1;
      }
    });
  }

  else {
    return create_native_closure(eng, [f = fct.fct, u = uid](vm_ref vm) mutable -> zs::int_t {
      if (vm.stack_size() < 1) {
        zb::print("Invalid native function call args size");
        return -1;
      }

      if (!vm[0].is_user_data()) {
        zb::print("Invalid data type in function call, expected user data");
        return -1;
      }

      if (!u.is_null() && !vm[0].has_user_data_uid(u)) {
        zb::print("Invalid uid type in function call");
        return -1;
      }

      ClassType* udata = (ClassType*)vm[0].get_user_data_data();

      // Number of arguments required by the function pointer (excluding vm_ref).
      constexpr size_t n_args = n_fct_pointer_args;
      using params_list = args_list;

      // Make sure there is enough args on the stack.
      // We ignore the first 'this' arg.
      if (vm.stack_size() <= (zs::int_t)n_args) {
        zb::print("Invalid native function call args size");
        return -1;
      }

      using tuple_type = typename params_list::tuple_type_no_ref;
      tuple_type tup;
      if (auto err = fill_function_arg_tuple(vm, tup)) {
        zb::print("Invalid native function call type");
        return -1;
      }

      // Call function - No return type.
      if constexpr (std::is_same_v<R, void>) {
        object_func_detail::apply_member_fct(f, udata, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        auto ret = zs::var(vm.get_engine(), object_func_detail::apply_member_fct(f, udata, std::move(tup)));
        vm.push(ret);
        return 1;
      }
    });
  }
}

// Member function.
template <class ClassType, class R, class... Args>
object object::create_native_closure_function(
    zs::engine* eng, zb::member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid) {
  return zs::object::create_native_closure_function(eng,
      zb::member_function_pointer_wrapper<zb::member_function_pointer<ClassType, R, Args...>, ClassType, R,
          Args...>{ fct },
      uid);
}

// Const member function.
template <class ClassType, class R, class... Args>
object object::create_native_closure_function(
    zs::engine* eng, zb::const_member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid) {
  return zs::object::create_native_closure_function(eng,
      zb::member_function_pointer_wrapper<zb::const_member_function_pointer<ClassType, R, Args...>, ClassType,
          R, Args...>{ fct },
      uid);
}

// Lamda function.
template <typename ClassType, typename ReturnType, class Fct, typename... Args>
object object::create_native_closure_function(zs::engine* eng, Fct&& fct, zb::type_list<Args...>) {

  using args_list = zb::type_list<Args...>;
  constexpr size_t n_fct_pointer_args = args_list::size();
  constexpr bool starts_with_vm_ref = function_pointer_starts_with_vm_ref<Args...>();
  constexpr bool starts_with_virtual_machine_ptr
      = function_pointer_starts_with_virtual_machine_ptr<Args...>();

  // If the function pointer starts with a vm_ref, we shift everything by one.
  if constexpr (starts_with_vm_ref || starts_with_virtual_machine_ptr) {

    return create_native_closure(eng, [f = std::forward<Fct>(fct)](vm_ref vm) mutable -> zs::int_t {
      // Number of arguments required by the function pointer (excluding vm_ref).
      constexpr size_t n_args = n_fct_pointer_args - 1;
      using params_list = typename args_list::template n_last_list<n_args>;

      // Make sure there is enough args on the stack.
      // We ignore the first 'this' arg.
      if (vm.stack_size() - 1 != (zs::int_t)n_args) {
        zb::print("Invalid native function call args size", vm.stack_size() - 1, n_args);
        return -1;
      }

      using tuple_type = typename params_list::tuple_type_no_ref;
      tuple_type tup;
      if (auto err = fill_function_arg_tuple(vm, tup)) {
        zb::print("Invalid native function call type");
        return -1;
      }

      // Call function - No return type.
      if constexpr (std::is_same_v<ReturnType, void>) {
        if constexpr (starts_with_vm_ref) {
          object_func_detail::apply(std::move(f), vm, std::move(tup));
        }
        else {
          object_func_detail::apply(std::move(f), vm.get_virtual_machine(), std::move(tup));
        }
        return 0;
      }

      // Call function - With return type.
      else {
        if constexpr (starts_with_vm_ref) {
          auto ret = zs::var(vm.get_engine(), object_func_detail::apply(std::move(f), vm, std::move(tup)));
          vm.push(ret);
        }
        else {
          auto ret = zs::var(vm.get_engine(),
              object_func_detail::apply(std::move(f), vm.get_virtual_machine(), std::move(tup)));
          vm.push(ret);
        }
        return 1;
      }
    });
  }

  else {
    return create_native_closure(eng, [f = std::forward<Fct>(fct)](vm_ref vm) mutable -> zs::int_t {
      // Number of arguments required by the function pointer (excluding vm_ref).
      constexpr size_t n_args = n_fct_pointer_args;
      using params_list = args_list;

      // Make sure there is enough args on the stack.
      // We ignore the first 'this' arg.
      if (vm.stack_size() <= (zs::int_t)n_args) {
        zb::print("Invalid native function call args size");
        return -1;
      }

      using tuple_type = typename params_list::tuple_type_no_ref;
      tuple_type tup;
      if (auto err = fill_function_arg_tuple(vm, tup)) {

        zb::print("Invalid native function call type");
        return -1;
      }

      // Call function - No return type.
      if constexpr (std::is_same_v<ReturnType, void>) {
        object_func_detail::apply(f, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        auto ret = zs::var(vm.get_engine(), object_func_detail::apply(f, std::move(tup)));
        vm.push(ret);
        return 1;
      }
    });
  }
}
//
//
//
//

template <class Fct>
zs::error_result vm_ref::new_closure(Fct&& fct) {

  struct closure_type : native_closure {

    inline closure_type(zs::engine* eng, Fct&& fct)
        : native_closure(eng)
        , _fct(std::forward<Fct>(fct)) {}

    virtual ~closure_type() override = default;

    virtual int_t call(virtual_machine* v) override {
      if constexpr (std::is_invocable_v<Fct, vm_ref>) {
        return _fct(vm_ref(v));
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
