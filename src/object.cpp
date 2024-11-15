#include <zscript.h>
#include "zvirtual_machine.h"

namespace zs {

struct object::helper {
  template <class Object>
  static inline void set_null(Object& obj) {
    ::memset(&obj, 0, sizeof(object));
    obj._type = object_type::k_null;
  }

  template <object_type ObjectType, class Object>
  static inline void set_type(Object& obj) {
    ::memset(&obj, 0, sizeof(object));
    obj._type = ObjectType;
  }
};

object object::create_vm(zs::engine* eng) {
  //  object obj = create_user_data(eng, (size_t)sizeof(zs::vm));
  //  zb_placement_new(obj.get_user_data_data()) zs::vm(eng);

  object obj;
  obj._type = object_type::k_user_data;
  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(zs::vm));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) zs::vm(eng);

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    zs::vm* t = (zs::vm*)ptr;
    t->~vm();
  });

  uobj->set_uid(zs::_ss("__vm__"));
  uobj->set_typeid(zs::_ss("__vm__"));

  // VERY DANGEROUS.
  //  obj.set_user_data_copy_to_type_function(
  //      [](void* dst, size_t data_size, std::string_view tid, void* data) -> zs::error_result {
  //        if (data_size == sizeof(T) and tid == typeid(T).name()) {
  //          T& dst_ref = *((T*)dst);
  //          const T& src_ref = *((const T*)data);
  //          dst_ref = src_ref;
  //          return {};
  //        }
  //
  //        return zs::error_code::invalid_type;
  //      });

  return obj;
}

object object::create_vm(size_t stack_size, allocate_t alloc_cb, raw_pointer_t user_pointer,
    raw_pointer_release_hook_t user_release) {
  //  object obj = create_user_data(eng, (size_t)sizeof(zs::vm));
  //  zb_placement_new(obj.get_user_data_data()) zs::vm(eng);
  zs::virtual_machine* vm = create_virtual_machine(stack_size, alloc_cb, user_pointer, user_release);
  zs::engine* eng = vm->get_engine();

  object obj;
  obj._type = object_type::k_user_data;
  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(zs::vm));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) zs::vm(eng);

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    zs::vm* t = (zs::vm*)ptr;
    t->~vm();
  });

  uobj->set_uid(zs::_ss("__vm__"));
  uobj->set_typeid(zs::_ss("__vm__"));

  // VERY DANGEROUS.
  //  obj.set_user_data_copy_to_type_function(
  //      [](void* dst, size_t data_size, std::string_view tid, void* data) -> zs::error_result {
  //        if (data_size == sizeof(T) and tid == typeid(T).name()) {
  //          T& dst_ref = *((T*)dst);
  //          const T& src_ref = *((const T*)data);
  //          dst_ref = src_ref;
  //          return {};
  //        }
  //
  //        return zs::error_code::invalid_type;
  //      });

  return obj;
}

object::object(native_closure_object* obj, bool should_retain) noexcept {
  helper::set_type<object_type::k_native_closure>(*this);
  _native_closure = obj;

  if (should_retain) {
    obj->retain();
  }
}

object::object(zs::engine* eng, std::string_view v) {
  ::memset(this, 0, sizeof(object));

  if (v.size() <= constants::k_small_string_max_size) {
    _type = k_small_string;
    memcpy(&_value, v.data(), v.size());
  }
  else {
    _type = object_type::k_long_string;
    _lstring = string_object::create(eng, v);
  }
}

object& object::operator=(const object& obj) {
  if (&obj == this) {
    return *this;
  }

  reset();

  ((object_base&)*this) = (const object_base&)obj;

  if (is_ref_counted()) {
    _ref_counted->retain();
  }

  return *this;
}

object& object::operator=(object&& obj) {
  if (&obj == this) {
    return *this;
  }

  reset();
  ::memcpy(this, &obj, sizeof(object_base));

  helper::set_null(obj);
  return *this;
}

object object::create_string(zs::engine* eng, std::string_view s) { return object(eng, s); }

object object::create_long_string(zs::engine* eng, std::string_view s) {
  object obj;
  obj._type = object_type::k_long_string;
  obj._lstring = string_object::create(eng, s);
  return obj;
}

object object::create_mutable_string(zs::engine* eng, std::string_view s) {
  object obj;
  obj._type = object_type::k_mutable_string;
  obj._mstring = mutable_string_object::create(eng, s);
  return obj;
}

object object::create_concat_string(zs::engine* eng, std::string_view s1, std::string_view s2) {
  size_t sz = s1.size() + s2.size();

  if (sz <= constants::k_small_string_max_size) {
    object s;
    ::memset(&s, 0, sizeof(object));
    s._type = k_small_string;
    ::memcpy((char*)&s._value, s1.data(), s1.size());
    ::memcpy(((char*)&s._value) + s1.size(), s2.data(), s2.size());
    return s;
  }
  else {
    object s;
    ::memset(&s, 0, sizeof(object));
    s._type = k_long_string;
    string_object*& sobj = s._lstring;
    sobj = string_object::create(eng, sz);

    ::memcpy((char*)sobj->data(), s1.data(), s1.size());
    ::memcpy(((char*)sobj->data()) + s1.size(), s2.data(), s2.size());
    return s;
  }
}

object object::create_concat_string(
    zs::engine* eng, std::string_view s1, std::string_view s2, std::string_view s3) {
  size_t sz = s1.size() + s2.size() + s3.size();

  if (sz <= constants::k_small_string_max_size) {
    object s;
    ::memset(&s, 0, sizeof(object));
    s._type = k_small_string;
    ::memcpy((char*)&s._value, s1.data(), s1.size());
    ::memcpy(((char*)&s._value) + s1.size(), s2.data(), s2.size());
    ::memcpy(((char*)&s._value) + s1.size() + s2.size(), s3.data(), s3.size());
    return s;
  }
  else {
    object s;
    ::memset(&s, 0, sizeof(object));
    s._type = k_long_string;
    string_object*& sobj = s._lstring;
    sobj = string_object::create(eng, sz);

    ::memcpy((char*)sobj->data(), s1.data(), s1.size());
    ::memcpy(((char*)sobj->data()) + s1.size(), s2.data(), s2.size());
    ::memcpy(((char*)sobj->data()) + s1.size() + s2.size(), s3.data(), s3.size());

    return s;
  }
}

object object::create_array(zs::engine* eng, size_t sz) {
  object obj;
  obj._type = object_type::k_array;
  obj._array = array_object::create(eng, sz);
  return obj;
}

object object::create_struct(zs::engine* eng) {
  object obj;
  obj._type = object_type::k_struct;
  obj._struct = struct_object::create(eng, 0);
  return obj;
}

object object::create_struct(zs::engine* eng, size_t sz) {
  object obj;
  obj._type = object_type::k_struct;
  obj._struct = struct_object::create(eng, sz);
  return obj;
}

object object::create_native_array(zs::engine* eng, zs::native_array_type ntype, size_t sz) {
  object obj;
  obj._type = object_type::k_native_array;
  obj._na_type = ntype;

  switch (ntype) {
#define _X(name, str, type)                                                 \
  case native_array_type::name:                                             \
    obj._native_array<type>() = native_array_object<type>::create(eng, sz); \
    break;

    ZS_NATIVE_ARRAY_TYPE_ENUM(_X)
#undef _X

  default:
    zbase_assert("Invalid native array type");
    return {};
  }

  return obj;
}

object object::create_table(zs::engine* eng) {
  object obj;
  obj._type = object_type::k_table;
  obj._table = table_object::create(eng);
  return obj;
}

object object::create_class(zs::engine* eng) {
  object obj;
  obj._type = object_type::k_class;
  obj._class = class_object::create(eng);
  return obj;
}

object object::create_table_with_delegate(zs::engine* eng, zs::object delegate) {
  object obj;
  obj._type = object_type::k_table;
  obj._table = table_object::create(eng);
  obj._table->set_delegate(std::move(delegate));
  return obj;
}

object object::create_native_closure(zs::engine* eng, zs::native_cclosure_t nc) {
  object obj;
  obj._type = object_type::k_native_closure;
  obj._native_closure = native_closure_object::create(eng, nc);
  return obj;
}

object object::create_native_closure(zs::engine* eng, zs::native_cpp_closure_t nc) {
  object obj;
  obj._type = object_type::k_native_closure;
  obj._native_closure = native_closure_object::create(eng, (zs::native_cclosure_t)nc);
  return obj;
}

object object::create_native_closure(zs::engine* eng, zs::native_closure* nc) {
  object obj;
  obj._type = object_type::k_native_closure;
  obj._native_closure = native_closure_object::create(eng, nc);
  return obj;
}

object object::create_closure(zs::engine* eng, const object& fct_prototype, const object& root) {
  object obj;
  obj._type = object_type::k_closure;
  zs::closure_object* closure = zs::closure_object::create(eng, fct_prototype, root);
  obj._closure = closure;
  return obj;
}

object object::create_user_data(zs::engine* eng, size_t size) {
  object obj;
  obj._type = object_type::k_user_data;
  zs::user_data_object* uobj = zs::user_data_object::create(eng, size);
  obj._udata = uobj;
  return obj;
}

object object::create_node(zs::engine* eng, const object& name) {
  object obj;
  obj._type = object_type::k_node;
  zs::node_object* node = zs::node_object::create(eng, name);
  obj._node = node;
  return obj;
}

object object::create_node(zs::engine* eng, const object& name, const object& value) {
  object obj;
  obj._type = object_type::k_node;
  zs::node_object* node = zs::node_object::create(eng, name, value);
  obj._node = node;
  return obj;
}

bool object::is_if_true() const noexcept {
  switch (_type) {
  case k_none:
  case k_null:
    return false;
  case k_bool:
    return _bool;
  case k_integer:
    return (bool)_int;
  case k_float:
    return (bool)_float;
  case k_small_string:
    return !get_small_string_unchecked().empty();
  case k_string_view:
    return !get_string_view_unchecked().empty();
  case k_long_string:
    return !get_long_string_unchecked().empty();
  case k_raw_pointer:
    return _pointer != nullptr;
  default:
    return true;
  }

  return true;
}

bool object::is_false() const noexcept {
  switch (_type) {
  case k_none:
  case k_null:
    return true;
  case k_bool:
    return !_bool;
  case k_integer:
    return !(bool)_int;
  case k_float:
    return !(bool)_float;
  case k_small_string:
    return get_small_string_unchecked().empty();
  case k_string_view:
    return get_string_view_unchecked().empty();
  case k_long_string:
    return get_long_string_unchecked().empty();
  case k_raw_pointer:
    return _pointer == nullptr;
  default:
    return _value == 0;
  }

  return true;
}

zs::error_result object::set_closure_release_hook(native_closure_release_hook_t hook) {
  if (!is_native_closure()) {
    return zs::error_code::invalid_type;
  }

  _native_closure->set_release_hook(hook);
  return {};
}

zs::error_result object::set_closure_user_pointer(zs::raw_pointer_t ptr) {
  if (!is_native_closure()) {
    return zs::error_code::invalid_type;
  }

  _native_closure->set_user_pointer(ptr);
  return {};
}

zs::error_result object::get_closure_user_pointer(zs::raw_pointer_t& ptr) const {
  if (!is_native_closure()) {
    return zs::error_code::invalid_type;
  }

  ptr = _native_closure->get_user_pointer();
  return {};
}

zs::error_result object::set_user_data_release_hook(zs::user_data_release_hook_t hook) {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  _udata->set_release_hook(hook);
  return {};
}

void* object::get_user_data_data() const noexcept {
  if (!is_user_data()) {
    return nullptr;
  }

  return _udata->data();
}

zs::error_result object::set_user_data_uid(const zs::object& uid) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  _udata->set_uid(uid);
  return {};
}

zs::error_result object::set_user_data_uid(zs::object&& uid) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  _udata->set_uid(std::move(uid));
  return {};
}

zs::error_result object::get_user_data_uid(zs::object& uid) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  uid = _udata->get_uid();
  return {};
}

zs::object object::get_user_data_uid() const noexcept {
  if (!is_user_data()) {
    return nullptr;
  }

  return _udata->get_uid();
}

bool object::has_user_data_uid(const zs::object& uid) const noexcept {
  if (!is_user_data()) {
    return false;
  }

  return uid == _udata->get_uid();
}

zs::error_result object::set_user_data_typeid(const zs::object& tid) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  _udata->set_typeid(tid);
  return {};
}

zs::error_result object::set_user_data_typeid(zs::object&& tid) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  _udata->set_typeid(std::move(tid));
  return {};
}

zs::error_result object::get_user_data_typeid(zs::object& tid) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  tid = _udata->get_typeid();
  return {};
}

bool object::has_user_data_typeid(const zs::object& tid) const noexcept {
  if (!is_user_data()) {
    return false;
  }

  return tid == _udata->get_typeid();
}

zs::error_result object::copy_user_data_to_type(
    void* obj, size_t data_size, std::string_view tid) const noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  return _udata->copy_to_type(obj, data_size, tid);
}

zs::error_result object::set_user_data_copy_to_type_function(zs::copy_user_data_to_type_t fct) noexcept {
  if (!is_user_data()) {
    return zs::error_code::invalid_type;
  }

  _udata->_copy_fct = fct;
  return {};
}

void object::reset() noexcept {
  if (is_ref_counted()) {
    _ref_counted->release();
  }

  helper::set_null(*this);
}

zs::error_result object::set_delegate(object delegate) noexcept {

  if (ZBASE_UNLIKELY(!(delegate.is_table() or delegate.is_null()))) {
    return zs::error_code::not_a_table;
  }

  if (ZBASE_UNLIKELY(!is_delegable())) {
    return zs::error_code::inaccessible;
  }

  switch (_type) {
  case k_array:
    _array->set_delegate(delegate);
    return {};

  case k_table:
    _table->set_delegate(delegate);
    return {};

  case k_user_data:

    _udata->set_delegate(delegate);
    return {};

  case k_instance:
    return zs::error_code::unimplemented;

  case k_extension:
    return zs::error_code::unimplemented;

  default:
    return zs::error_code::inaccessible;
  }
}

bool object::has_delegate() const noexcept {
  if (!is_delegable()) {
    return false;
  }

  return as_delegate().has_delegate();
}

zs::table_object* object::get_delegate() const noexcept {
  if (!is_delegable()) {
    return nullptr;
  }

  zs::delegate_object& del_obj = as_delegate();

  if (!del_obj.has_delegate()) {
    return nullptr;
  }

  zs::object& del = del_obj.get_delegate();

  if (!del.is_table()) {
    return nullptr;
  }

  return del._table;
}

object object::get_weak_ref() const noexcept {
  if (is_ref_counted() and !is_weak_ref()) {
    return _ref_counted->get_weak_ref(*this);
  }

  return *this;
}

object object::get_weak_ref_value() const noexcept {
  zbase_assert(is_weak_ref());

  if (!is_weak_ref()) {
    return {};
  }

  return _weak_ref->get_object();
}

zs::error_result object::get_binary_size(size_t& write_size) const {
  write_size = 0;

  switch (_type) {
  case k_null:
  case k_none:
  case k_bool:
  case k_integer:
  case k_float:
  case k_small_string:
    write_size += sizeof(object_base);
    return {};

  case k_long_string: {
    std::string_view s = get_long_string_unchecked();
    write_size += sizeof(object_base) + (s.size() <= constants::k_small_string_max_size ? 0 : s.size());
    return {};
  }
  case k_string_view: {
    std::string_view s = get_string_view_unchecked();
    write_size += sizeof(object_base) + (s.size() <= constants::k_small_string_max_size ? 0 : s.size());
    return {};
  }
  case k_array: {
    const zs::vector<object>& arr = as_array();
    write_size += sizeof(object_base);

    for (const auto& item : arr) {
      size_t sz = 0;
      if (auto err = item.get_binary_size(sz)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }

  case k_table: {
    const auto& map = _table->get_map();
    write_size += sizeof(object_base);

    for (auto item : map) {
      size_t sz = 0;
      if (auto err = item.first.get_binary_size(sz)) {
        return err;
      }

      write_size += sz;

      sz = 0;
      if (auto err = item.second.get_binary_size(sz)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }
  default:
    return zs::error_code::invalid_operation;
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

zs::error_result object::to_binary(
    write_function_t write_func, size_t& write_size, void* data, uint32_t flags) const {
  write_size = 0;

  switch (_type) {
  case k_null:
  case k_none:
  case k_bool:
  case k_integer:
  case k_float:
  case k_small_string:
    return write_object(*this, write_func, write_size, data);

  case k_long_string: {
    std::string_view s = _lstring->get_string();

    if (s.size() <= constants::k_small_string_max_size) {
      return zs::_ss(s).to_binary(write_func, write_size, data);
    }

    object_base obj = *this;
    obj._value = s.size();

    if (auto err = write_object(obj, write_func, write_size, data)) {
      return err;
    }

    if (auto err = write_binary_data(s, write_func, write_size, data)) {
      return err;
    }

    return {};
  }
  case k_string_view: {
    std::string_view s = get_string_view_unchecked();

    if (s.size() <= constants::k_small_string_max_size) {
      return zs::_ss(s).to_binary(write_func, write_size, data);
    }

    object_base obj = *this;
    obj._value = s.size();

    if (auto err = write_object(obj, write_func, write_size, data)) {
      return err;
    }

    if (auto err = write_binary_data(s, write_func, write_size, data)) {
      return err;
    }
    return {};
  }

  case k_array: {
    const zs::vector<object>& vec = as_array();
    size_t size = vec.size();

    object_base obj = *this;
    obj._value = size;
    if (auto err = write_object(obj, write_func, write_size, data)) {
      return err;
    }

    for (const auto& item : vec) {
      size_t sz = 0;
      if (auto err = item.to_binary(write_func, sz, data, flags)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }

  case k_table: {
    const auto& map = _table->get_map();
    size_t size = map.size();

    object_base obj = *this;
    obj._value = size;
    if (auto err = write_object(obj, write_func, write_size, data)) {
      return err;
    }

    for (auto item : map) {
      size_t sz = 0;
      if (auto err = item.first.to_binary(write_func, sz, data, flags)) {
        return err;
      }

      write_size += sz;

      sz = 0;
      if (auto err = item.second.to_binary(write_func, sz, data, flags)) {
        return err;
      }

      write_size += sz;
    }

    return {};
  }

  case k_function_prototype: {
    return {};
  }
  default:
    return zs::error_code::invalid_operation;
  }
  return {};
}

zs::error_result object::from_binary(
    zs::engine* eng, std::span<uint8_t> buffer, object& out, size_t& offset) {

  size_t buff_sz = buffer.size();
  if (buff_sz < sizeof(object_base)) {
    return zs::error_code::invalid_argument;
  }
  object_base obj;
  ::memset(&obj, 0, sizeof(object));
  obj._type = k_null;

  memcpy(&obj, buffer.data(), sizeof(object_base));

  offset += sizeof(object_base);

  switch (obj._type) {
  case k_null:
  case k_none:
  case k_bool:
  case k_integer:
  case k_float:
  case k_small_string:
    out = object(obj, false);
    return {};

  case k_long_string:
  case k_string_view: {
    size_t slen = obj._value;
    if (buff_sz < slen) {
      return zs::error_code::invalid_argument;
    }
    out = zs::object::create_string(eng, std::string_view((const char*)buffer.data() + offset, slen));
    offset += slen;
    return {};
  }

  case k_array: {

    size_t len = obj._value;
    out = zs::object::create_array(eng, len);
    zs::vector<object>& vec = out.as_array();

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

    size_t len = obj._value;
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
    return zs::error_code::invalid_operation;
  }

  return {};
}

object* object::operator[](size_t idx) {
  if (is_array()) {
    return &as_array()[idx];
  }

  return nullptr;
}

const object* object::operator[](size_t idx) const {
  if (is_array()) {
    return &as_array()[idx];
  }

  return nullptr;
}

} // namespace zs.
