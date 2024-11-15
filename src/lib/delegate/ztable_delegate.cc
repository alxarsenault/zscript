namespace zs {
namespace {
  static inline int_t table_size_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm->top();
    table_object* tbl = obj._table;
    return vm.push(tbl->size());
  }

  static inline int_t table_is_empty_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm->top();
    table_object* tbl = obj._table;
    return vm.push(tbl->get_map().empty());
  }

  static inline int_t table_clear_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm->top();
    table_object* tbl = obj._table;
    tbl->get_map().clear();
    return 0;
  }

  static inline int_t table_begin_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 1) {
      zb::print("Error: table.table_begin_impl (a, b)");
      return -1;
    }
    return vm.push(object::create_table_iterator(vm[0]));
  }

  static inline int_t table_end_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 1) {
      zb::print("Error: table.table_contains_impl (a, b)");
      return -1;
    }

    return vm.push(object::create_table_iterator(vm[0].as_table().end()));
  }

  static inline int_t table_contains_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 2) {
      zb::print("Error: table.table_contains_impl (a, b)");
      return -1;
    }

    const object& obj = vm[0];
    return vm.push(obj.as_table().contains(vm[1].get_string_unchecked()));
  }

  static inline int_t table_set_delegate_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 2) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm->stack()[-2];
    object& delegate = vm->top();

    if (ZBASE_UNLIKELY(!(delegate.is_table() or delegate.is_null()))) {
      return -1;
    }

    table_object* tbl = obj._table;
    tbl->set_delegate(delegate);
    return 0;
  }
  static inline int_t table_optset_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 3) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm[0];
    const object& key = vm[1];
    const object& value = vm[2];

    if (auto err = obj.as_table().set_no_replace(key, value); err and err != error_code::already_exists) {
      return -1;
    }

    return vm.push(obj.as_table()[key]);
  }

  static inline int_t table_to_array_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    zs::table_object& tbl = vm[0].as_table();
    zs::object arr = _a(vm, tbl.size());
    zs::array_object& a = arr.as_array();
    auto it = a.begin();

    for (auto s : tbl) {
      *it++ = zs::_a(vm, { s.first, s.second });
    }

    return vm.push(arr);
  }

  static inline int_t table_to_pairs_array_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    zs::table_object& tbl = vm[0].as_table();
    zs::object arr = _a(vm, tbl.size());
    zs::array_object& a = arr.as_array();
    auto it = a.begin();

    for (auto s : tbl) {
      *it++ = zs::_t(vm, { { zs::_ss("key"), s.first }, { zs::_ss("value"), s.second } });
    }

    return vm.push(arr);
  }

  static inline int_t table_to_sorted_array_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (!zb::is_one_of(nargs, 1, 2)) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    zs::table_object& tbl = vm[0].as_table();
    zs::object arr = _a(vm, tbl.size());
    zs::array_object& a = arr.as_array();
    auto it = a.begin();

    for (auto s : tbl) {
      *it++ = zs::_a(vm, { s.first, s.second });
    }

    if (nargs == 1) {

      std::sort(a.begin(), a.end(),
          [](const object& a, const object& b) { return a.as_array()[0] < b.as_array()[0]; });
    }
    else {

      const object& sort_fct = vm[1];
      //
      if (!sort_fct.is_function()) {
        return -1;
      }

      std::sort(a.begin(), a.end(), [&](const object& lhs, const object& rhs) {
        object ret;

        if (auto err = vm->call(sort_fct, { arr, lhs, rhs }, ret)) {
          return false;
        }

        return ret.is_bool() ? ret._bool : false;
      });
    }

    return vm.push(arr);
  }

  static inline int_t table_to_sorted_pairs_array_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (!zb::is_one_of(nargs, 1, 2)) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    zs::table_object& tbl = vm[0].as_table();
    zs::object arr = _a(vm, tbl.size());
    zs::array_object& a = arr.as_array();
    auto it = a.begin();

    for (auto s : tbl) {
      *it++ = zs::_t(vm, { { zs::_ss("key"), s.first }, { zs::_ss("value"), s.second } });
    }

    if (nargs == 1) {

      std::sort(a.begin(), a.end(),
          [](const object& a, const object& b) { return a.as_table()["key"] < b.as_table()["key"]; });
    }
    else {

      const object& sort_fct = vm[1];
      //
      if (!sort_fct.is_function()) {
        return -1;
      }

      std::sort(a.begin(), a.end(), [&](const object& lhs, const object& rhs) {
        object ret;

        if (auto err = vm->call(sort_fct, { arr, lhs, rhs }, ret)) {
          return false;
        }

        return ret.is_bool() ? ret._bool : false;
      });
    }

    return vm.push(arr);
  }
} // namespace

zs::object create_table_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object* tbl = obj._table;
  tbl->reserve(20);

  tbl->set(_ss("size"), _nf(table_size_impl));
  tbl->set(_ss("is_empty"), _nf(table_is_empty_impl));
  tbl->set(_ss("clear"), _nf(table_clear_impl));
  tbl->set(_ss("contains"), _nf(table_contains_impl));
  tbl->set(_ss("set_delegate"), _nf(table_set_delegate_impl));
  tbl->set(_ss("emplace"), _nf(table_optset_impl));
  tbl->set(_ss("begin"), _nf(table_begin_impl));
  tbl->set(_ss("end"), _nf(table_end_impl));
  tbl->set(_ss("to_array"), _nf(table_to_array_impl));
  (*tbl)["to_sorted_array"] = _nf(table_to_sorted_array_impl);
  (*tbl)["to_pairs_array"] = _nf(table_to_pairs_array_impl);
  (*tbl)["to_sorted_pairs_array"] = _nf(table_to_sorted_pairs_array_impl);

  return obj;
}
} // namespace zs.
