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

#include <zbase/zbase.h>
#include <zbase/sys/assert.h>

#include <zbase/sys/file_view.h>
#include <zbase/utility/print.h>
#include <zbase/strings/string_view.h>
#include <zbase/utility/traits.h>

#include <zscript/core/zcore.h>

#include <filesystem>
#include <string>
#include <stdexcept>

//
// MARK: Config
//

#define ZS_FUNCTION_DEF +[](zs::vm_ref vm) -> int_t

namespace zs {

namespace constants {
  inline constexpr bool k_is_object_stack_resizable = true;
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
    stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER);

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
