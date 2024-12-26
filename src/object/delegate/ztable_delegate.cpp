#include "ztable_delegate.h"
#include "zvirtual_machine.h"
#include "utility/zparameter_stream.h"

namespace zs {
namespace {

  static_assert(std::is_trivially_destructible_v<zs::object_unordered_map<object>::iterator>, "");
  static_assert(std::is_trivially_copyable_v<zs::object_unordered_map<object>::iterator>, "");

  struct table_iterator_ref {

    inline table_iterator_ref(object& obj)
        : pointer(obj._atom.table_it.get()) {}

    zs::object_unordered_map<zs::object>::iterator& pointer;

    inline const object& key() const noexcept { return pointer->first; }

    inline zs::object_unordered_map<zs::object>::iterator& ptr() const noexcept { return pointer; }
    inline zs::object_unordered_map<zs::object>::iterator itptr() const noexcept { return pointer; }

    inline bool operator==(const table_iterator_ref& it) const noexcept { return ptr() == it.ptr(); }
    inline bool operator!=(const table_iterator_ref& it) const noexcept { return ptr() != it.ptr(); }
  };

  object create_table_iterator(zs::vm_ref vm, zs::object_unordered_map<zs::object>::iterator ptr);

  inline object create_table_iterator(zs::vm_ref vm, table_iterator_ref it_ref) {
    return create_table_iterator(vm, it_ref.ptr());
  }

  int_t table_iterator_add_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 3) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in table_iterator._add.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in table_iterator._add.\n");
      return -1;
    }

    int_t rhs;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(rhs), -1);

    if (rhs != 1) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid increment in table_iterator._add.\n");
      return -1;
    }

    object it = create_table_iterator(vm, table_iterator_ref(it_atom));
    table_iterator_ref it_ref(it);

    ++it_ref.ptr();
    return vm.push(it);
  }

  int_t table_iterator_pre_incr_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in table_iterator._add.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in table_iterator._add.\n");
      return -1;
    }

    object it = create_table_iterator(vm, table_iterator_ref(it_atom));
    table_iterator_ref it_ref(it);
    ++it_ref.ptr();
    return vm.push(it);
  }

  zs::object create_table_iterator_delegate(zs::engine* eng) {
    object obj = object::create_table(eng);
    table_object& tbl = obj.as_table();
    tbl.reserve(10);

    tbl.emplace(constants::get<meta_method::mt_typeof>(), _ss("table_iterator"));
    tbl.emplace(constants::get<meta_method::mt_add>(), table_iterator_add_impl);
    tbl.emplace(constants::get<meta_method::mt_pre_incr>(), table_iterator_pre_incr_impl);

    tbl.emplace("next", [](vm_ref vm) -> int_t {
      table_iterator_ref it_ref(vm[0]);
      return vm.push(create_table_iterator(vm, ++it_ref.itptr()));
    });

    tbl.emplace("is_same",
        [](vm_ref vm) -> int_t { return vm.push(table_iterator_ref(vm[0]) == table_iterator_ref(vm[1])); });

    tbl.emplace("get", [](vm_ref vm) -> int_t { return vm.push(table_iterator_ref(vm[0]).ptr()->second); });

    tbl.emplace("get_if_not", [](vm_ref vm) -> int_t {
      table_iterator_ref it_ref(vm[0]);
      return it_ref != table_iterator_ref(vm[1]) ? vm.push(it_ref.ptr()->second) : vm.push_null();
    });

    tbl.emplace("safe_get", [](vm_ref vm) -> int_t {
      table_iterator_ref it_ref(vm[0]);
      return vm[1].as_table().end() != it_ref.ptr() ? vm.push(it_ref.ptr()->second) : vm.push_null();
    });

    tbl.emplace("get_key", [](vm_ref vm) -> int_t { return vm.push(table_iterator_ref(vm[0]).key()); });

    tbl.emplace("safe_key", [](vm_ref vm) -> int_t {
      table_iterator_ref it_ref(vm[0]);
      return vm[1].as_table().end() != it_ref.ptr() ? vm.push(it_ref.key()) : vm.push_null();
    });

    tbl.emplace("get_key_if_not", [](vm_ref vm) -> int_t {
      table_iterator_ref it_ref(vm[0]);
      return it_ref != table_iterator_ref(vm[1]) ? vm.push(it_ref.key()) : vm.push_null();
    });

    return obj;
  }

  object create_table_iterator(zs::vm_ref vm, zs::object_unordered_map<zs::object>::iterator ptr) {
    if (object& obj = vm->get_delegated_atom_delegates_table()
                          .as_table()[(int_t)constants::k_atom_table_iterator_delegate_id];
        !obj.is_table()) {
      obj = create_table_iterator_delegate(vm.get_engine());
    }

    object it;
    it._type = object_type::k_atom;
    it._atom_type = atom_type::atom_custom;
    it._atom.table_it.construct(ptr);
    it._ex2_delegated_atom_delegate_id = constants::k_atom_table_iterator_delegate_id;
    return it;
  }

  static inline int_t table_size_impl(zs::vm_ref vm) noexcept {

    const int_t nargs = vm.stack_size();
    if (nargs != 1) {
      vm->handle_error(errc::invalid_parameter_count, { -1, -1 }, ZS_DEVELOPER_SOURCE_LOCATION(),
          "Invalid parameter count (", nargs, ") in table.size(), expected 1.\n");
      return -1;
    }

    zs::parameter_stream ps(vm);

    const table_object* tbl_obj = nullptr;
    if (auto err = ps.optional<table_parameter>(tbl_obj)) {
      vm->handle_error(errc::not_a_table, { -1, -1 }, ZS_DEVELOPER_SOURCE_LOCATION(),
          "Invalid table parameter in table.size().\n");
      return -1;
    }

    return vm.push(tbl_obj->size());
  }

  static inline int_t table_is_empty_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    const object& obj = vm->top();
    table_object* tbl = obj._table;
    return vm.push(tbl->get_map().empty());
  }

  static inline int_t table_clear_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    const object& obj = vm->top();
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
    return vm.push(create_table_iterator(vm, vm[0].as_table().begin()));
  }

  static inline int_t table_end_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 1) {
      zb::print("Error: table.table_contains_impl (a, b)");
      return -1;
    }

    return vm.push(create_table_iterator(vm, vm[0].as_table().end()));
  }

  static inline int_t table_contains_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 2) {
      zb::print("Error: table.table_contains_impl (a, b)");
      return -1;
    }

    const object& obj = vm[0];
    return vm.push(obj.as_table().contains(vm[1]));
  }

  static inline int_t table_set_delegate_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 2) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm->stack()[-2];
    const object& delegate = vm->top();

    if (ZBASE_UNLIKELY(!(delegate.is_table() or delegate.is_null()))) {
      return -1;
    }

    table_object* tbl = obj._table;
    tbl->set_delegate(delegate);
    return 0;
  }

  static inline int_t table_get_delegate_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm[0];

    return vm.push(obj.as_table().get_delegate());
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

        return ret.is_bool() ? (bool)ret._int : false;
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

        return ret.is_bool() ? (bool)ret._int : false;
      });
    }

    return vm.push(arr);
  }

} // namespace

zs::object create_table_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();
  tbl.reserve(20);

  tbl.set(_ss("size"), _nf(table_size_impl));
  tbl.set(_ss("is_empty"), _nf(table_is_empty_impl));
  tbl.set(_ss("clear"), _nf(table_clear_impl));
  tbl.set(_ss("contains"), _nf(table_contains_impl));
  tbl.set(_ss("set_delegate"), _nf(table_set_delegate_impl));
  tbl.set(_ss("get_delegate"), _nf(table_get_delegate_impl));
  tbl.set(_ss("emplace"), _nf(table_optset_impl));
  tbl.set(_ss("begin"), _nf(table_begin_impl));
  tbl.set(_ss("end"), _nf(table_end_impl));
  tbl.set(_ss("to_array"), _nf(table_to_array_impl));
  tbl["to_sorted_array"] = _nf(table_to_sorted_array_impl);
  tbl["to_pairs_array"] = _nf(table_to_pairs_array_impl);
  tbl["to_sorted_pairs_array"] = _nf(table_to_sorted_pairs_array_impl);

  tbl.set_delegate(zs::object::create_none());
  tbl.set_use_default_delegate(false);

  return obj;
}

} // namespace zs.
