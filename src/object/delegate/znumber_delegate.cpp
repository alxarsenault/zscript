#include "znumber_delegate.h"
#include "zvirtual_machine.h"
#include <zbase/strings/unicode.h>

namespace zs {

static int_t number_delegate_to_int_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_number()) {
    vm->set_error("Not an number in to_integer");
    return -1;
  }

  return vm.push(obj.convert_to_integer_unchecked());
}

static int_t number_delegate_to_float_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_number()) {
    vm->set_error("Not an number in to_float");
    return -1;
  }

  return vm.push(obj.convert_to_float_unchecked());
}

static int_t number_delegate_to_string_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_number()) {
    vm->set_error("Not an number in to_string");
    return -1;
  }

  if (obj.is_integer()) {
    std::array<char, 64> buffer;
    buffer.fill(0);
    int_t sz = ::snprintf(buffer.data(), buffer.size(), "%lld", obj._int);
    return vm.push_string(std::string_view(buffer.data(), sz));
  }

  zs::ostringstream ss(zs::create_string_stream(vm->get_engine()));
  ss << obj._float;
  return vm.push_string(ss.str());
}

static int_t number_delegate_to_char_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_integer()) {
    vm->set_error("Not an integer in to_char");
    return -1;
  }

  uint32_t c = (uint32_t)obj._int;
  char buffer[4] = {};
  zb::unicode::u32_to_u8(&c, &c + 1, buffer);
  return vm.push_string(std::string_view(buffer, zb::unicode::code_point_size_u8(c)));
}

static int_t number_delegate_is_nan_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_number()) {
    vm->set_error("Not a number in is_nan");
    return -1;
  }

  float_t val = obj.convert_to_float_unchecked();
  return vm.push(std::isnan(val));
}

static int_t number_delegate_is_inf_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_number()) {
    vm->set_error("Not a number in is_inf");
    return -1;
  }

  float_t val = obj.convert_to_float_unchecked();
  return vm.push(std::isinf(val));
}

static int_t number_delegate_is_neg_impl(zs::vm_ref vm) {
  const object& obj = vm[0];
  if (!obj.is_number()) {
    vm->set_error("Not a number in is_neg");
    return -1;
  }

  float_t val = obj.convert_to_float_unchecked();
  return vm.push(std::signbit(val));
}

zs::object create_number_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();

  tbl.reserve(6);
  tbl.emplace(zs::_ss("to_float"), number_delegate_to_float_impl);
  tbl.emplace(zs::_ss("to_int"), number_delegate_to_int_impl);
  tbl.emplace(zs::_ss("to_string"), number_delegate_to_string_impl);
  tbl.emplace(zs::_ss("to_char"), number_delegate_to_char_impl);
  tbl.emplace(zs::_ss("is_nan"), number_delegate_is_nan_impl);
  tbl.emplace(zs::_ss("is_inf"), number_delegate_is_inf_impl);
  tbl.emplace(zs::_ss("is_neg"), number_delegate_is_neg_impl);

  tbl.set_delegate(object::create_none());
  tbl.set_use_default_delegate(false);
  return obj;
}

} // namespace zs.
