
namespace zs {
namespace {

#define ZS_ARRAY_SET_ARG_ERROR(fct_name) vm.set_error("Invalid array argument in array::" fct_name "().")

#define ZS_ARRAY_GET(fct_name)                                        \
  object& obj = vm[0];                                                \
  if (!obj.is_array()) {                                              \
    vm.set_error("Invalid array argument in array::" fct_name "()."); \
    return -1;                                                        \
  }                                                                   \
  array_object& arr = obj.as_array()

#define ZS_ARRAY_BEGIN_IMPL(fct_name, n_args)                              \
  if (vm.stack_size() != n_args) {                                         \
    vm.set_error("Invalid number of arguments in array::" fct_name "()."); \
    return -1;                                                             \
  }                                                                        \
  ZS_ARRAY_GET(fct_name)

  static inline int_t array_size_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("size", 1);
    return vm.push(arr.size());
  }

  static inline int_t array_capacity_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("capacity", 1);
    return vm.push(arr.capacity());
  }

  static inline int_t array_clear_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("clear", 1);
    arr.clear();
    return vm.push(obj);
  }

  static inline int_t array_pop_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();

    if (!zb::is_one_of(nargs, 1, 2)) {
      ZS_ARRAY_SET_ARG_ERROR("pop");
      return -1;
    }

    ZS_ARRAY_GET("pop");

    if (nargs == 1) {

      arr.pop_back();
      return vm.push(obj);
    }

    const object& n_pop = vm[1];
    if (!n_pop.is_integer()) {
      vm.set_error("Invalid pop size argument in array::pop().");
      return -1;
    }

    int_t n_pop_val = zb::minimum(n_pop._int, (int_t)arr.size());
    arr.erase(arr.end() - n_pop_val, arr.end());
    return vm.push(obj);
  }

  static inline int_t array_copy_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("copy", 1);

    object new_obj;
    new_obj._type = object_type::k_array;
    new_obj._array = arr.clone();

    return vm.push(new_obj);
  }

  static inline int_t array_begin_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("begin", 1);
    return vm.push(object::create_array_iterator(obj));
  }

  static inline int_t array_end_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("end", 1);
    return vm.push(object::create_array_iterator(arr.data() + arr.size(), (uint32_t)arr.size()));
  }

  static inline int_t array_swap_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("swap", 3);
    return vm.push(object::create_array_iterator(arr.data() + arr.size(), (uint32_t)arr.size()));
  }

  static inline int_t array_is_empty_impl(zs::vm_ref vm) noexcept {

    if (vm.stack_size() != 1) {
      vm.set_error("Invalid array argument in array::is_empty().");
      return -1;
    }

    object& obj = vm->top();
    array_object* arr = obj._array;
    return vm.push(arr->empty());
  }

  static inline int_t array_resize_impl(zs::vm_ref vm) noexcept {

    if (vm.stack_size() != 2) {
      vm.set_error("Invalid array argument in array::resize().");
      return -1;
    }

    object& obj = vm->stack()[-2];
    object& value = vm->top();
    array_object* arr = obj._array;
    arr->resize(value._int);
    return vm.push(obj);
  }

  static inline int_t array_reserve_impl(zs::vm_ref vm) noexcept {

    ZS_ARRAY_BEGIN_IMPL("reserve", 2);

    const object& reserve_size = vm[1];
    if (!reserve_size.is_integer()) {
      vm.set_error("Invalid reserve size argument in array::reserve().");
      return -1;
    }

    arr.reserve(reserve_size._int);
    return vm.push(obj);
  }

  static inline int_t array_get_impl(zs::vm_ref vm) noexcept {

    ZS_ARRAY_BEGIN_IMPL("get", 2);

    object& value = vm[1];

    object dst;
    arr.get(value._int, dst);

    return vm.push(dst);
  }

  static inline int_t array_erase_impl(zs::vm_ref vm) noexcept {

    if (vm.stack_size() != 2) {
      vm.set_error("Invalid number of arguments in array::erase(index).");
      return -1;
    }

    object& obj = vm[0];
    if (!obj.is_array()) {
      vm.set_error("Invalid array argument in array::erase().");
      return -1;
    }

    const object& pos = vm[1];

    if (!pos.is_integer()) {
      vm.set_error("Pos should be an integer");
      return -1;
    }

    array_object& arr = obj.as_array();
    arr.erase(arr.begin() + pos._int);
    return vm.push(obj);
  }

  static inline int_t array_erase_if_impl(zs::vm_ref vm) noexcept {

    ZS_ARRAY_BEGIN_IMPL("erase_if", 2);

    const object& erval = vm[1];

    if (erval.is_function()) {
      size_t n = std::erase_if(arr, [&](const object& e) {
        object ret;

        if (auto err = vm->call(erval, { obj, e }, ret)) {
          return false;
        }

        return ret.is_bool() ? ret._bool : false;
      });

      return vm.push(n);
    }

    size_t n = std::erase(arr, erval);
    return vm.push(n);
  }

  static inline int_t array_sort_impl(zs::vm_ref vm) noexcept {

    const int_t nargs = vm.stack_size();
    if (nargs < 1) {
      ZS_ARRAY_SET_ARG_ERROR("sort");
      return -1;
    }

    ZS_ARRAY_GET("sort");

    if (nargs == 1) {
      std::sort(arr.begin(), arr.end());
      return vm.push(obj);
    }

    //
    const object& sort_fct = vm[1];
    //
    if (!sort_fct.is_function()) {
      return -1;
    }

    std::sort(arr.begin(), arr.end(), [&](const object& lhs, const object& rhs) {
      object ret;

      if (auto err = vm->call(sort_fct, { obj, lhs, rhs }, ret)) {
        return false;
      }

      return ret.is_bool() ? ret._bool : false;
    });

    return vm.push(obj);
  }

  template <object_type Obj>
  static inline int_t array_is_type_array_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("is_type_array", 1);
    return vm.push(arr.is_type_array(Obj));
  }

  static inline int_t array_is_number_array_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("is_number_array", 1);
    return vm.push(arr.is_number_array());
  }

  static inline int_t array_is_string_array_impl(zs::vm_ref vm) noexcept {
    ZS_ARRAY_BEGIN_IMPL("is_string_array", 1);
    return vm.push(arr.is_string_array());
  }

  static inline int_t array_push_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs < 2) {
      ZS_ARRAY_SET_ARG_ERROR("push");
      return -1;
    }

    ZS_ARRAY_GET("push");

    for (int_t i = 1; i < nargs; i++) {
      arr.push(vm[i]);
    }

    return vm.push(obj);
  }

  static inline int_t array_append_impl(zs::vm_ref vm) noexcept {

    const int_t nargs = vm.stack_size();
    if (nargs < 2) {
      ZS_ARRAY_SET_ARG_ERROR("append");
      return -1;
    }

    ZS_ARRAY_GET("append");

    for (int_t i = 1; i < nargs; i++) {
      const object& rhs = vm[i];

      if (rhs.is_array()) {
        arr.insert(arr.end(), rhs.as_array().begin(), rhs.as_array().end());
      }
      else {
        arr.push(rhs);
      }
    }

    return vm.push(obj);
  }

  static inline int_t array_insert_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (!zb::is_one_of(nargs, 3, 4)) {
      ZS_ARRAY_SET_ARG_ERROR("insert");
      return -1;
    }

    ZS_ARRAY_GET("insert");

    const object& pos = vm[1];
    if (!pos.is_integer()) {
      vm.set_error("Pos should be an integer");
      return -1;
    }

    if (nargs == 3) {
      const object& value = vm[2];
      arr.insert(arr.begin() + pos._int, value);
      return vm.push(obj);
    }

    const object& n_times = vm[2];
    if (!n_times.is_integer()) {
      vm.set_error("n should be an integer");
      return -1;
    }

    const object& value = vm[3];
    arr.insert(arr.begin() + pos._int, (size_t)n_times._int, value);
    return vm.push(obj);
  }

  static const std::string_view s_is_number_array = "is_number_array";
  static const std::string_view s_is_string_array = "is_string_array";
  static const std::string_view s_is_float_array = "is_float_array";
  static const std::string_view s_is_integer_array = "is_integer_array";
} // namespace

zs::object create_array_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  zs::table_object& t = obj.as_table();
  t.reserve(30);
  t["size"] = t["length"] = _nf(array_size_impl);
  t["push"] = _nf(array_push_impl);
  t["capacity"] = _nf(array_capacity_impl);
  t["is_empty"] = _nf(array_is_empty_impl);
  t["resize"] = _nf(array_resize_impl);
  t["reserve"] = _nf(array_reserve_impl);
  t["get"] = _nf(array_get_impl);
  t["erase"] = _nf(array_erase_impl);
  t["insert"] = _nf(array_insert_impl);
  t["clear"] = _nf(array_clear_impl);
  t["pop"] = _nf(array_pop_impl);
  t["append"] = _nf(array_append_impl);
  t["erase_if"] = _nf(array_erase_if_impl);
  t["sort"] = _nf(array_sort_impl);
  t["copy"] = _nf(array_copy_impl);
  t["begin"] = _nf(array_begin_impl);
  t["end"] = _nf(array_end_impl);
  t["swap"] = _nf(array_swap_impl);

  t.set(_sv(s_is_number_array), _nf(array_is_number_array_impl));
  t.set(_sv(s_is_string_array), _nf(array_is_string_array_impl));
  t.set(_sv(s_is_float_array), _nf(array_is_type_array_impl<object_type::k_float>));
  t.set(_sv(s_is_integer_array), _nf(array_is_type_array_impl<object_type::k_integer>));

  return obj;
}
} // namespace zs.
