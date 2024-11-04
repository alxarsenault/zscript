#pragma once

#include <zscript/common.h>
#include <zscript/object.h>
#include <zbase/utility/traits.h>

namespace zs {
struct object_function_wrapper {

  template <class Fct>
  ZS_CHECK inline static object create(zs::engine* eng, Fct&& fct);

  template <class R, class... Args>
  ZS_CHECK inline static object create(zs::engine* eng, zb::function_pointer<R, Args...> fct);

  template <class ClassType, class R, class... Args>
  ZS_CHECK inline static object create(
      zs::engine* eng, zb::member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid = {});

  template <class ClassType, class R, class... Args>
  ZS_CHECK inline static object create(zs::engine* eng,
      zb::const_member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid = {});

private:
  template <typename ClassType, typename ReturnType, class Fct, typename... Args>
  ZS_CHECK inline static object create(zs::engine* eng, Fct&& fct, zb::type_list<Args...>);

  template <class Fct, class ClassType, class R, class... Args>
  ZS_CHECK inline static object create(zs::engine* eng,
      zb::member_function_pointer_wrapper<Fct, ClassType, R, Args...> fct, const zs::object& uid = {});

  template <size_t I = 0, bool StartsWithVm = false, class... Args>
  inline static zs::error_result fill_function_arg_tuple(vm_ref vm, std::tuple<Args...>& tup) {
    using args_list = zb::type_list<Args...>;

    if constexpr (I >= sizeof...(Args)) {
      return {};
    }
    else {
      using vtype = typename args_list::template type_at_index<I>;
      vtype v;

      if (auto err = vm[I + 1].get_value<vtype>(v)) {
        ZS_ERROR("Got ", zb::quoted(zs::get_object_type_name(vm[I + 1].get_type())), " expected ",
            zb::quoted(zs::var::get_value_conv_obj_name<vtype>()),
            " aka : ", zb::quoted(typeid(vtype).name()));
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

  template <class F, class VM, class Tuple, std::size_t... I>
  inline constexpr static decltype(auto) apply_impl(F&& f, VM vm, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), vm, std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class F, class Tuple, std::size_t... I>
  inline constexpr static decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class ClassType, class F, class VM, class Tuple, std::size_t... I>
  inline constexpr static decltype(auto) apply_member_impl(
      ClassType* obj, F&& f, VM vm, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), obj, vm, std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class ClassType, class F, class Tuple, std::size_t... I>
  inline constexpr static decltype(auto) apply_member_impl(
      ClassType* obj, F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), obj, std::get<I>(std::forward<Tuple>(t))...);
  }

  template <class F, class Tuple>
  inline constexpr static decltype(auto) invoke_fct(F&& f, Tuple&& t) {
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <class F, class VM, class Tuple>
  inline constexpr static decltype(auto) invoke_fct(F&& f, VM vm, Tuple&& t) {
    return apply_impl(std::forward<F>(f), vm, std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <class F, class ClassType, class Tuple>
  inline constexpr static decltype(auto) invoke_member_fct(F&& f, ClassType* obj, Tuple&& t) {
    return apply_member_impl(obj, std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <class F, class ClassType, class VM, class Tuple>
  inline constexpr static decltype(auto) invoke_member_fct(F&& f, ClassType* obj, VM vm, Tuple&& t) {
    return apply_member_impl(obj, std::forward<F>(f), vm, std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
  }
};

// Function pointer.
template <class R, class... Args>
object object_function_wrapper::create(zs::engine* eng, zb::function_pointer<R, Args...> fct) {

  using args_list = zb::type_list<Args...>;
  constexpr size_t n_fct_pointer_args = args_list::size();
  constexpr bool starts_with_vm_ref = function_pointer_starts_with_vm_ref<Args...>();

  // If the function pointer starts with a vm_ref, we shift everything by one.
  if constexpr (starts_with_vm_ref) {

    return object::create_native_closure(eng, [f = fct](vm_ref vm) mutable -> zs::int_t {
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
        invoke_fct(f, vm, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        return vm.push(zs::var(vm.get_engine(), invoke_fct(f, vm, std::move(tup))));
      }
    });
  }
  else {
    return object::create_native_closure(eng, [f = fct](vm_ref vm) mutable -> zs::int_t {
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
        invoke_fct(f, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        return vm.push(zs::var(vm.get_engine(), invoke_fct(f, std::move(tup))));
      }
    });
  }
}

// Any member function pointer.
template <class Fct, class ClassType, class R, class... Args>
object object_function_wrapper::create(zs::engine* eng,
    zb::member_function_pointer_wrapper<Fct, ClassType, R, Args...> fct, const zs::object& uid) {

  using args_list = zb::type_list<Args...>;
  constexpr size_t n_fct_pointer_args = args_list::size();
  constexpr bool starts_with_vm_ref = function_pointer_starts_with_vm_ref<Args...>();

  // If the function pointer starts with a vm_ref, we shift everything by one.
  if constexpr (starts_with_vm_ref) {

    return object::create_native_closure(eng, [f = fct.fct, u = uid](vm_ref vm) mutable -> zs::int_t {
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
        invoke_member_fct(f, udata, vm, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        return vm.push(zs::var(vm.get_engine(), invoke_member_fct(f, udata, vm, std::move(tup))));
      }
    });
  }

  else {
    return object::create_native_closure(eng, [f = fct.fct, u = uid](vm_ref vm) mutable -> zs::int_t {
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
        invoke_member_fct(f, udata, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        return vm.push(zs::var(vm.get_engine(), invoke_member_fct(f, udata, std::move(tup))));
      }
    });
  }
}

// Member function.
template <class ClassType, class R, class... Args>
object object_function_wrapper::create(
    zs::engine* eng, zb::member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid) {
  return object_function_wrapper::create(eng,
      zb::member_function_pointer_wrapper<zb::member_function_pointer<ClassType, R, Args...>, ClassType, R,
          Args...>{ fct },
      uid);
}

// Const member function.
template <class ClassType, class R, class... Args>
object object_function_wrapper::create(
    zs::engine* eng, zb::const_member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid) {
  return object_function_wrapper::create(eng,
      zb::member_function_pointer_wrapper<zb::const_member_function_pointer<ClassType, R, Args...>, ClassType,
          R, Args...>{ fct },
      uid);
}

// Lamda function.
template <typename ClassType, typename ReturnType, class Fct, typename... Args>
object object_function_wrapper::create(zs::engine* eng, Fct&& fct, zb::type_list<Args...>) {

  using args_list = zb::type_list<Args...>;
  constexpr size_t n_fct_pointer_args = args_list::size();
  constexpr bool starts_with_vm_ref = function_pointer_starts_with_vm_ref<Args...>();

  // If the function pointer starts with a vm_ref, we shift everything by one.
  if constexpr (starts_with_vm_ref) {

    return object::create_native_closure(eng, [f = std::forward<Fct>(fct)](vm_ref vm) mutable -> zs::int_t {
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
        invoke_fct(std::move(f), vm, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        return vm.push(zs::var(vm.get_engine(), invoke_fct(std::move(f), vm, std::move(tup))));
      }
    });
  }

  else {
    return object::create_native_closure(eng, [f = std::forward<Fct>(fct)](vm_ref vm) mutable -> zs::int_t {
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
        invoke_fct(f, std::move(tup));
        return 0;
      }

      // Call function - With return type.
      else {
        return vm.push(zs::var(vm.get_engine(), invoke_fct(f, std::move(tup))));
      }
    });
  }
}

template <class Fct>
object object_function_wrapper::create(zs::engine* eng, Fct&& fct) {
  if constexpr (zb::is_function_pointer_v<Fct>) {
    using fct_ptr_type = zb::function_pointer_type_t<Fct>;
    return object_function_wrapper::create(eng, (fct_ptr_type)fct);
  }
  else {
    using traits = zb::function_traits<Fct>;
    using R = typename traits::result_type;
    using class_type = typename traits::class_type;
    using args_list = typename traits::args_list;
    return object_function_wrapper::create<class_type, R>(eng, std::forward<Fct>(fct), args_list{});
  }
}
} // namespace zs.
