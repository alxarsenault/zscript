#include <zscript/zscript.h>
#include <zscript/std/zfloat_array.h>
#include "zvirtual_machine.h"
#include "utility/zparameter_stream.h"
#include <zscript/base/strings/charconv.h>
#include <zscript/base/strings/unicode.h>
#include <zscript/base/strings/stack_string.h>
#include <algorithm>
#include <numeric>

namespace zs {
namespace {
  zs::object create_float_array_delegate(zs::engine* eng);
  zs::object& get_float_array_delegate(zs::engine* eng);

  namespace flt_array {
    inline constexpr object uid = _sv("__float_array_object__");
    inline constexpr object reg_id = _sv("__float_array_delegate__");

    inline zs::error_result validate_index(int_t& index, size_t length) {

      if (index < 0) {
        index += length;
      }

      if (index < 0 or index >= (int_t)length) {
        return errc::out_of_bounds;
      }

      return {};
    }

  } // namespace flt_array

  int_t float_array_size_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);
    return vm.push((int_t)arr->size());
  }

  // vm[0] should be the float array.
  // vm[1] should be the key.
  // vm[2] should be the delegate.
  int_t float_array_meta_get_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    int_t index = 0;
    ZS_RETURN_IF_ERROR(ps.optional<integer_parameter>(index), vm.push(zs::none()));

    // Length of the mutable string in u32.
    const size_t length = arr->size();

    ZS_RETURN_IF_ERROR(flt_array::validate_index(index, length), vm.set_error("Out of bounds."));

    return vm.push((*arr)[index]);
  }

  // vm[0] should be the float array.
  // vm[1] should be the key.
  // vm[2] should be the value.
  // vm[3] should be the delegate.
  int_t float_array_meta_set_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    if (ps.size() != 4) {
      return -1;
    }

    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    int_t index = 0;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(index), -1);

    // Length of the mutable string in u32.
    const size_t sz = arr->size();

    ZS_RETURN_IF_ERROR(flt_array::validate_index(index, sz), vm.set_error("Out of bounds\n"));

    float_t input_value = 0;
    ZS_RETURN_IF_ERROR(ps.require<number_parameter>(input_value), -1);

    (*arr)[index] = input_value;
    return vm.push(vm[0]);
  }
  //
  //  // vm[0] should be the mutable string (lhs).
  //  // vm[1] should be the rhs.
  //  // vm[2] should be the delegate.
  //  int_t float_array_meta_add_impl(zs::vm_ref vm) {
  //
  //    zs::parameter_stream ps(vm);
  //
  //    if (ps.size() != 3) {
  //      return -1;
  //    }
  //
  //    float_array* mstr = nullptr;
  //    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(mstr), -1);
  //
  //    const zs::object& rhs = vm[1];
  //
  //    if (!rhs.is_string()) {
  //      vm.set_error("Invalid float_array object");
  //      return -1;
  //    }
  //
  //    std::string_view rhs_str = rhs.get_string_unchecked();
  //    zs::string new_str(*mstr, zs::string_allocator(vm.get_engine()));
  //    new_str += rhs_str;
  //    return vm.push(create_float_array(vm, std::move(new_str)));
  //  }

  int_t float_array_append_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    float_t input_value = 0;
    ZS_RETURN_IF_ERROR(ps.require<number_parameter>(input_value), -1);

    arr->push_back(input_value);
    return vm.push(vm[0]);
  }

  int_t float_array_min_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    if (arr->empty()) {
      return vm.push_null();
    }

    return vm.push(*std::min_element(arr->begin(), arr->end()));
  }
  int_t float_array_max_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    if (arr->empty()) {
      return vm.push_null();
    }

    return vm.push(*std::max_element(arr->begin(), arr->end()));
  }

  int_t float_array_sum_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    if (arr->empty()) {
      return vm.push_float(0);
    }

    return vm.push(std::accumulate(arr->begin(), arr->end(), 0));
  }

  int_t float_array_ramp_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    float_array* arr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<float_array_parameter>(arr), -1);

    if (arr->empty()) {
      return vm.push_float(0);
    }

    float_t val = 0;
    float_t delta = 1;

    if (auto err = ps.require_if_valid<number_parameter>(val)) {
      vm->ZS_VM_ERROR(err, "Invalid number parameter.");
      return -1;
    }

    if (auto err = ps.require_if_valid<number_parameter>(delta)) {
      vm->ZS_VM_ERROR(err, "Invalid number parameter.");
      return -1;
    }

    for (size_t i = 0; i < arr->size(); i++) {
      (*arr)[i] = val;
      val += delta;
    }
    //  std::iota(arr->begin(), arr->end(), val);
    return vm.push(vm[0]);
  }

  zs::object create_float_array_delegate(zs::engine* eng) {
    using namespace literals;

    table_object* tbl = table_object::create(eng);
    tbl->reserve(20);

    tbl->emplace(constants::get<meta_method::mt_typeof>(), "float_array"_ss);
    tbl->emplace(constants::get<meta_method::mt_get>(), float_array_meta_get_impl);
    tbl->emplace(constants::get<meta_method::mt_set>(), float_array_meta_set_impl);
    //    tbl->emplace(constants::get<meta_method::mt_tostring>(), float_array_to_string_impl);
    //    tbl->emplace(constants::get<meta_method::mt_add>(), float_array_meta_add_impl);
    //    tbl->emplace(constants::get<meta_method::mt_add_eq>(), float_array_meta_add_eq_impl);

    //    tbl->emplace("to_string"_ss, float_array_to_string_impl);
    tbl->emplace("size"_ss, float_array_size_impl);
    //    tbl->emplace("ascii_size"_ss, float_array_ascii_size_impl);
    //    tbl->emplace("is_ascii"_ss, float_array_is_ascii_impl);
    //    tbl->emplace("starts_with"_ss, float_array_starts_with_impl);
    //    tbl->emplace("ends_with"_ss, float_array_ends_with_impl);
    tbl->emplace("push"_ss, float_array_append_impl);
    tbl->emplace("append"_ss, float_array_append_impl);
    tbl->emplace("min"_ss, float_array_min_impl);
    tbl->emplace("max"_ss, float_array_max_impl);
    tbl->emplace("sum"_ss, float_array_sum_impl);
    tbl->emplace("ramp"_ss, float_array_ramp_impl);
    //    tbl->emplace("contains"_ss, float_array_contains_impl);

    tbl->set_no_default_none();
    return object(tbl, false);
  }

  zs::object& get_float_array_delegate(zs::engine* eng) {
    object& obj = eng->get_registry_table_object()[flt_array::reg_id];
    return obj.is_table() ? obj : (obj = create_float_array_delegate(eng));
  }

  int_t np_ramp_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    ++ps;

    float_t start = 0;
    ZS_RETURN_IF_ERROR(ps.require<number_parameter>(start), -1);

    float_t end = 0;
    ZS_RETURN_IF_ERROR(ps.require<number_parameter>(end), -1);
    int_t count = 0;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(count), -1);
    float_t dt = (end - start) / count;
    object arr = create_float_array(vm);
    float_t v = start;
    while (v < end) {
      arr.as_udata().data_ref<float_array>().push_back(v);
      v += dt;
    }
    //  std::iota(arr->begin(), arr->end(), val);
    return vm.push(arr);
  }
} // namespace

float_array& float_array::as_float_array(const object& obj) noexcept {
  return obj.as_udata().data_ref<float_array>();
}

bool is_float_array(const object& obj) noexcept {
  return obj.is_user_data() and obj.as_udata().get_uid() == flt_array::uid;
}

zs::error_result float_array_parameter::parse(
    zs::parameter_stream& s, bool output_error, float_array*& value) {

  if (s.is_user_data_with_uid(flt_array::uid)) {
    value = s++->as_udata().data<float_array>();
    return {};
  }

  s.set_opt_error(output_error, "Invalid float array type.");
  return zs::errc::invalid_parameter_type;
}

object create_float_array(zs::vm_ref vm) noexcept {
  zs::engine* eng = vm.get_engine();
  user_data_object* uobj = user_data_object::create<float_array>(eng, zs::allocator<float>(eng));
  uobj->set_uid(flt_array::uid);
  uobj->set_type_id(flt_array::uid);
  uobj->set_delegate(get_float_array_delegate(eng));
  return zs::object(uobj, false);
}

int_t vm_create_float_array(zs::vm_ref vm) {
  zs::parameter_stream ps(vm);
  ++ps;

  if (ps.size() > 1) {
    vm->set_error("Out hjkhjkh bounds\n");
    return -1;
  }

  array_object* arr = nullptr;
  if (!ps.optional<array_parameter>(arr)) {

    if (!arr->is_number_array()) {
      return -1;
    }

    object obj = create_float_array(vm);

    float_array& flt_arr = obj.as_udata().data_ref<float_array>();

    for (const auto& n : *arr) {
      flt_arr.push_back(n.convert_to_float_unchecked());
    }

    return vm.push(obj);
  }

  return vm.push(create_float_array(vm));
}

object create_float_array_lib(zs::engine* eng) noexcept {

  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();
  tbl.reserve(20);

  object ramp = zs::object::create_native_closure(eng, np_ramp_impl);

  ramp.as_native_closure().add_parameter(_ss("this"));
  ramp.as_native_closure().add_parameter(_ss("start"), constants::k_number_mask);
  ramp.as_native_closure().add_parameter(_ss("end"), constants::k_number_mask);
  ramp.as_native_closure().add_parameter(_ss("count"), zs::create_type_mask(object_type::k_integer));
  ramp.as_native_closure().add_default_parameter(20);

  tbl.emplace(_ss("array"), zs::vm_create_float_array);
  tbl.emplace(_ss("ramp"), std::move(ramp));

  return obj;
}

} // namespace zs.
