#include "zarray_delegate.h"
#include "zvirtual_machine.h"
#include "object/zfunction_prototype.h"
#include "utility/zparameter_stream.h"

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

  struct array_iterator_ref {

    inline array_iterator_ref(object& obj) noexcept
        : index(obj._ex1_u32)
        , pointer(obj._pointer) {}

    uint32_t& index;
    void*& pointer;

    ZS_CK_INLINE int_t idx() const noexcept { return (int_t)index; }
    ZS_CK_INLINE zs::object*& ptr() const noexcept { return reinterpret_cast<zs::object*&>(pointer); }
    ZS_CK_INLINE bool operator==(const array_iterator_ref& it) const noexcept { return ptr() == it.ptr(); }
    ZS_CK_INLINE bool operator!=(const array_iterator_ref& it) const noexcept { return ptr() != it.ptr(); }
  };

  object create_array_iterator(zs::vm_ref vm, int_t index, const object* ptr);

  inline object create_array_iterator(zs::vm_ref vm, array_iterator_ref it_ref) {
    return create_array_iterator(vm, it_ref.idx(), it_ref.ptr());
  }

  int_t array_iterator_add_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 3) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in array_iterator._add.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in array_iterator._add.\n");
      return -1;
    }

    int_t rhs;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(rhs), -1);

    object it = create_array_iterator(vm, array_iterator_ref(it_atom));
    array_iterator_ref it_ref(it);
    it_ref.index += rhs;
    it_ref.ptr() += rhs;
    return vm.push(it);
  }

  int_t array_iterator_pre_incr_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in array_iterator.++.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in array_iterator.++.\n");
      return -1;
    }

    array_iterator_ref it_ref(it_atom);
    ++it_ref.index;
    ++it_ref.ptr();
    return vm.push(it_atom);
  }

  int_t array_iterator_pre_decr_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in array_iterator.++.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in array_iterator.++.\n");
      return -1;
    }

    object it = create_array_iterator(vm, array_iterator_ref(it_atom));
    array_iterator_ref it_ref(it);
    --it_ref.index;
    --it_ref.ptr();
    return vm.push(it);
  }

  zs::object create_array_iterator_delegate(zs::engine* eng) {
    object obj = object::create_table(eng);
    table_object& tbl = obj.as_table();
    tbl.reserve(10);

    tbl.emplace(constants::get<meta_method::mt_typeof>(), _ss("array_iterator"));

    tbl.emplace(constants::get<meta_method::mt_add>(), array_iterator_add_impl);
    tbl.emplace(constants::get<meta_method::mt_pre_incr>(), array_iterator_pre_incr_impl);
    tbl.emplace(constants::get<meta_method::mt_pre_decr>(), array_iterator_pre_decr_impl);

    tbl.emplace("next", [](vm_ref vm) -> int_t {
      array_iterator_ref it_ref(vm[0]);
      return vm.push(create_array_iterator(vm, it_ref.index + 1, it_ref.ptr() + 1));
    });

    tbl.emplace("__compare", [](vm_ref vm) -> int_t {
      array_iterator_ref lhs(vm[0]);
      array_iterator_ref rhs(vm[1]);

      if (lhs.ptr() == rhs.ptr()) {
        return vm.push_integer(0);
      }
      else if (lhs.ptr() < rhs.ptr()) {
        return vm.push_integer(-1);
      }
      return vm.push_integer(1);
    });

    tbl.emplace("is_same",
        [](vm_ref vm) -> int_t { return vm.push(array_iterator_ref(vm[0]) == array_iterator_ref(vm[1])); });

    tbl.emplace("get", [](vm_ref vm) -> int_t { return vm.push(*array_iterator_ref(vm[0]).ptr()); });

    tbl.emplace("get_if_not", [](vm_ref vm) -> int_t {
      array_iterator_ref it_ref(vm[0]);
      return it_ref != array_iterator_ref(vm[1]) ? vm.push(*it_ref.ptr()) : vm.push_null();
    });

    tbl.emplace("safe_get", [](vm_ref vm) -> int_t {
      array_iterator_ref it_ref(vm[0]);
      return vm[1].as_array().is_ptr_in_range(it_ref.ptr()) ? vm.push(*it_ref.ptr()) : vm.push_null();
    });

    tbl.emplace("get_key", [](vm_ref vm) -> int_t { return vm.push(array_iterator_ref(vm[0]).idx()); });

    tbl.emplace("safe_key", [](vm_ref vm) -> int_t {
      array_iterator_ref it_ref(vm[0]);
      return vm[1].as_array().is_ptr_in_range(it_ref.ptr()) ? vm.push(it_ref.idx()) : vm.push_null();
    });

    tbl.emplace("get_key_if_not", [](vm_ref vm) -> int_t {
      array_iterator_ref it_ref(vm[0]);
      return it_ref != array_iterator_ref(vm[1]) ? vm.push(it_ref.idx()) : vm.push_null();
    });

    return obj;
  }

  object create_array_iterator(zs::vm_ref vm, int_t index, const object* ptr) {
    if (object& obj = vm->get_delegated_atom_delegates_table()
                          .as_table()[(int_t)constants::k_atom_array_iterator_delegate_id];
        !obj.is_table()) {
      obj = create_array_iterator_delegate(vm.get_engine());
    }

    object it;
    it._type = object_type::k_atom;
    it._atom_type = atom_type::atom_array_iterator;
    it._pointer = (void*)ptr;
    it._ex1_u32 = (uint32_t)index;
    it._ex2_delegate_id = constants::k_atom_array_iterator_delegate_id;
    return it;
  }

  static inline int_t array_size_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.size().");
      return -1;
    }

    return vm.push(arr_ptr->size());
  }

  static inline int_t array_capacity_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.capacity().");
      return -1;
    }

    return vm.push(arr_ptr->capacity());
  }

  static inline int_t array_clear_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.clear().");
      return -1;
    }

    arr_ptr->clear();
    return vm.push(vm[0]);
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
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.copy().");
      return -1;
    }

    return vm.push(arr_ptr->clone());
  }

  static inline int_t array_begin_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.begin().");
      return -1;
    }

    return vm.push(create_array_iterator(vm, 0, arr_ptr->data()));
  }

  static inline int_t array_end_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.end().");
      return -1;
    }

    return vm.push(create_array_iterator(vm, arr_ptr->size(), arr_ptr->data() + arr_ptr->size()));
  }

  static inline int_t array_is_empty_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.is_empty().");
      return -1;
    }
    return vm.push(arr_ptr->empty());
  }

  static inline int_t array_resize_impl(zs::vm_ref vm) noexcept {

    if (vm.stack_size() != 2) {
      vm.set_error("Invalid array argument in array::resize().");
      return -1;
    }

    object& obj = vm->stack()[-2];
    const object& value = vm->top();
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

    if (!value.is_type(object_type::k_integer, object_type::k_bool)) {
      vm->handle_error(zs::errc::invalid_type, { -1, -1 }, "Invalid index type in 'array.get()'.",
          zb::source_location::current());
      return -1;
    }

    object dst;
    if (auto err = arr.get(value._int, dst)) {
      vm->handle_error(err, { -1, -1 }, "Could not get value from array in 'array.get()'.",
          zb::source_location::current());
      return -1;
    }

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
    return vm.push(pos._int);
  }

  static inline int_t array_erase_get_impl(zs::vm_ref vm) noexcept {

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
    object val = arr[pos._int];
    arr.erase(arr.begin() + pos._int);
    return vm.push(std::move(val));
  }

  static inline int_t array_erase_indices_impl(zs::vm_ref vm) noexcept {
    int_t nargs = vm.stack_size();

    if (nargs < 2) {
      vm.set_error("Invalid number of arguments in array::erase_indexes(indices).");
      return -1;
    }

    object& obj = vm[0];
    if (!obj.is_array()) {
      vm.set_error("Invalid array argument in array::erase_indices().");
      return -1;
    }

    zs::vector<int_t> indices(zs::allocator<int_t>(vm->get_engine()));

    const object& indices_start_obj = vm[1];

    if (indices_start_obj.is_array()) {
      const zs::array_object& arr_indices = indices_start_obj.as_array();
      indices.resize(arr_indices.size());

      for (int_t i = 0; i < arr_indices.size(); i++) {
        int_t index = -1;
        if (auto err = arr_indices[i].get_integer(index)) {
          vm.set_error("Parameter 1 should be an array of indices");
          return -1;
        }

        indices[i] = index;
      }
    }
    else if (indices_start_obj.is_integer()) {
      for (int_t i = 1; i < nargs; i++) {
        int_t index = -1;
        if (auto err = vm[i].get_integer(index)) {
          vm.set_error("Parameter 1 should be an array of indices");
          return -1;
        }

        indices.push_back(index);
      }
    }
    else {

      vm.set_error("Parameter 1 should be an array of indices");
      return -1;
    }

    std::sort(indices.begin(), indices.end(), std::greater<int_t>());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    array_object& arr = obj.as_array();

    for (size_t i = 0; i < indices.size(); i++) {
      arr.erase(arr.begin() + indices[i]);
    }
    //  arr.erase(arr.begin() + pos._int);
    return vm.push(obj);
  }

  static inline int_t array_erase_if_impl(zs::vm_ref vm) noexcept {

    ZS_ARRAY_BEGIN_IMPL("erase_if", 2);

    const object& erval = vm[1];

    if (erval.is_function()) {
      size_t n = std::erase_if(arr.to_vec(), [&](const object& e) {
        object ret;

        if (auto err = vm->call(erval, { obj, e }, ret)) {
          return false;
        }

        return ret.is_bool() ? (bool)ret._int : false;
      });

      return vm.push(n);
    }

    size_t n = std::erase(arr.to_vec(), erval);
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

      return ret.is_bool() ? (bool)ret._int : false;
    });

    return vm.push(obj);
  }

  template <object_type Obj>
  int_t array_delegate_is_type_array_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.is_type_array().");
      return -1;
    }
    return vm.push(arr_ptr->is_type_array(Obj));
  }

  int_t array_delegate_is_number_array_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.is_number_array().");
      return -1;
    }
    return vm.push(arr_ptr->is_number_array());
  }

  int_t array_is_string_array_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.is_string_array().");
      return -1;
    }
    return vm.push(arr_ptr->is_string_array());
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

  static int_t array_delegate_get_impl(zs::vm_ref vm) noexcept {
    object& obj = vm[0];
    if (!obj.is_array()) {
      vm.set_error("Invalid array argument in array::[].");
      return -1;
    }

    array_object& arr = obj.as_array();

    const object& key = vm[1];

    // If the key is a number, we access the element directly.
    if (key.is_number()) {
      int_t index = key.convert_to_integer_unchecked();
      const int_t sz = (int_t)arr.size();

      if (index < 0) {
        index += sz;
      }

      if (index >= 0 && index < sz) {
        return vm.push(arr[index]);
      }

      vm->set_error("Out of bounds\n");
      return -1;
    }

    return vm.push(zs::none());
  }

  static inline int_t array_get_delegate_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm[0];

    return vm.push(obj.as_array().get_delegate());
  }

  int_t array_min_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array::min(...).");
      return -1;
    }

    array_object& arr = *arr_ptr;

    bool has_float = false;
    if (!arr.is_number_array(has_float)) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_type, "A number array was expected in array.min().");
      return -1;
    }

    if (has_float) {
      float_t min_value = arr[0].convert_to_float_unchecked();
      for (const object& elem : arr) {
        if (float_t value = elem.convert_to_float_unchecked(); value < min_value) {
          min_value = value;
        }
      }

      return vm.push_float(min_value);
    }

    int_t min_value = arr[0].convert_to_integer_unchecked();
    for (const object& elem : arr) {
      if (float_t value = elem.convert_to_integer_unchecked(); value < min_value) {
        min_value = value;
      }
    }

    return vm.push_integer(min_value);
  }

  int_t array_max_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array::max(...).");
      return -1;
    }

    array_object& arr = *arr_ptr;

    bool has_float = false;
    if (!arr.is_number_array(has_float)) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_type, "A number array was expected in array.max().");
      return -1;
    }

    if (has_float) {
      float_t max_value = arr[0].convert_to_float_unchecked();
      for (const object& elem : arr) {
        if (float_t value = elem.convert_to_float_unchecked(); value > max_value) {
          max_value = value;
        }
      }

      return vm.push_float(max_value);
    }

    int_t max_value = arr[0].convert_to_integer_unchecked();
    for (const object& elem : arr) {
      if (float_t value = elem.convert_to_integer_unchecked(); value > max_value) {
        max_value = value;
      }
    }

    return vm.push_integer(max_value);
  }

  int_t array_index_range_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.index_range(...).");
      return -1;
    }

    array_object& arr = *arr_ptr;
    return vm.push(zs::_a(vm, { (int_t)0, arr.empty() ? 0 : arr.size() - 1 }));
  }

  int_t array_value_range_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.value_range(...).");
      return -1;
    }

    array_object& arr = *arr_ptr;

    bool has_float = false;
    if (!arr.is_number_array(has_float)) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_type, "A number array was expected in array.value_range().");
      return -1;
    }

    if (has_float) {
      float_t min_value = arr[0].convert_to_float_unchecked();
      float_t max_value = min_value;

      for (const object& elem : arr) {
        float_t value = elem.convert_to_float_unchecked();

        if (value < min_value) {
          min_value = value;
        }

        if (value > max_value) {
          max_value = value;
        }
      }

      return vm.push(zs::_a(vm, { min_value, max_value }));
    }

    int_t min_value = arr[0].convert_to_integer_unchecked();
    int_t max_value = min_value;
    for (const object& elem : arr) {
      int_t value = elem.convert_to_integer_unchecked();

      if (value < min_value) {
        min_value = value;
      }

      if (value > max_value) {
        max_value = value;
      }
    }

    return vm.push(zs::_a(vm, { min_value, max_value }));
  }

  static inline int_t array_visit_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    const object& obj = *ps;

    array_object* arr_ptr = nullptr;
    if (auto err = ps.require<array_parameter>(arr_ptr)) {
      vm->ZS_VM_ERROR(err, "An array was expected in array.visit(...).");
      return -1;
    }

    array_object& arr = *arr_ptr;

    object fct;
    if (auto err = ps.require<function_parameter>(fct)) {
      vm->ZS_VM_ERROR(err, "A closure was expected in array.visit(...).");
      return -1;
    }

    const size_t sz = arr.size();
    object ret_value;

    std::array<object, 3> params_array = { obj, nullptr, 0 };
    const bool has_3_params = fct.get_parameter_interface().get_parameters_count() == 3;
    const int_t n_params = 2 + has_3_params;

    object& index = params_array[1];
    object& item = params_array[n_params - 1];
    zs::parameter_list params(params_array.data(), n_params);

    for (size_t i = 0; i < sz; i++) {
      index = (int_t)i;
      item = arr[i];
      if (auto err = vm->call(fct, params, ret_value)) {
        vm->ZS_VM_ERROR(err, "Invalid visit call in array.visit(...).");
        return -1;
      }

      if (ret_value.is_bool() and ret_value._int == true) {
        return 0;
      }
    }

    return 0;
  }

  inline constexpr object k_is_number_array_name = zs::_sv("is_number_array");
  inline constexpr object k_is_string_array_name = zs::_sv("is_string_array");
  inline constexpr object k_is_float_array_name = zs::_sv("is_float_array");
  inline constexpr object k_is_integer_array_name = zs::_sv("is_integer_array");

} // namespace

zs::object create_array_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  zs::table_object& t = obj.as_table();
  t.reserve(30);

  t.emplace(zs::_sv(constants::k_mt_get_string), array_delegate_get_impl);

  t.emplace(_ss("get_delegate"), array_get_delegate_impl);
  t.emplace(_ss("size"), array_size_impl);
  t.emplace(_ss("length"), array_size_impl);
  t.emplace(_ss("push"), array_push_impl);
  t.emplace(_ss("capacity"), array_capacity_impl);
  t.emplace(_ss("is_empty"), array_is_empty_impl);
  t.emplace(_ss("resize"), array_resize_impl);
  t.emplace(_ss("reserve"), array_reserve_impl);
  t.emplace(_ss("get"), array_get_impl);
  t.emplace(_ss("erase"), array_erase_impl);
  t.emplace(_ss("erase_get"), array_erase_get_impl);
  t.emplace(_ss("erase_indices"), array_erase_indices_impl);
  t.emplace(_ss("insert"), array_insert_impl);
  t.emplace(_ss("clear"), array_clear_impl);
  t.emplace(_ss("pop"), array_pop_impl);
  t.emplace(_ss("append"), array_append_impl);
  t.emplace(_ss("erase_if"), array_erase_if_impl);
  t.emplace(_ss("sort"), array_sort_impl);
  t.emplace(_ss("copy"), array_copy_impl);
  t.emplace(_ss("begin"), array_begin_impl);
  t.emplace(_ss("end"), array_end_impl);
  t.emplace(_ss("min"), array_min_impl);
  t.emplace(_ss("max"), array_max_impl);
  t.emplace(_ss("index_range"), array_index_range_impl);
  t.emplace(_ss("value_range"), array_value_range_impl);
  t.emplace(_ss("visit"), array_visit_impl);

  t.emplace(k_is_number_array_name, array_delegate_is_number_array_impl);
  t.emplace(k_is_string_array_name, array_is_string_array_impl);
  t.emplace(k_is_float_array_name, array_delegate_is_type_array_impl<object_type::k_float>);
  t.emplace(k_is_integer_array_name, array_delegate_is_type_array_impl<object_type::k_integer>);

  t.set_no_default_none();
  return obj;
}
} // namespace zs.
