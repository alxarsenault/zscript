#include <zscript/std/zmath.h>
#include "zvirtual_machine.h"
#include "utility/zparameter_stream.h"
#include <random>

namespace zs {

namespace {

  template <auto Fct>
  int_t zmath_trigo_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    ++ps;

    float_t value = 0;
    if (auto err = ps.require<number_parameter>(value)) {
      vm->ZS_VM_ERROR(err, "A number was expected in math::cos().");
      return -1;
    }

    return vm.push_float(Fct(value));
  }

  int_t zmath_min_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    ++ps;

    if (ps.has_type(object_type::k_float)) {

      float_t min_value = (std::numeric_limits<float_t>::max)();

      while (ps) {
        float_t value = 0;
        if (auto err = ps.require<number_parameter>(value)) {
          vm->ZS_VM_ERROR(err, "A number was expected in math::min(...).");
          return -1;
        }

        if (value < min_value) {
          min_value = value;
        }
      }

      return vm.push(min_value);
    }

    int_t min_value = (std::numeric_limits<int_t>::max)();

    while (ps) {
      int_t value = 0;
      if (auto err = ps.require<number_parameter>(value)) {
        vm->ZS_VM_ERROR(err, "A number was expected in math::min(...).");
        return -1;
      }

      if (value < min_value) {
        min_value = value;
      }
    }

    return vm.push(min_value);
  }

  int_t zmath_max_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    ++ps;

    if (ps.has_type(object_type::k_float)) {
      float_t max_value = -(std::numeric_limits<float_t>::max)();

      while (ps) {
        float_t value = 0;
        if (auto err = ps.require<number_parameter>(value)) {
          vm->ZS_VM_ERROR(err, "A number was expected in math::max(...).");
          return -1;
        }

        if (value > max_value) {
          max_value = value;
        }
      }

      return vm.push(max_value);
    }

    int_t max_value = -(std::numeric_limits<int_t>::max)();

    while (ps) {
      int_t value = 0;
      if (auto err = ps.require<number_parameter>(value)) {
        vm->ZS_VM_ERROR(err, "A number was expected in math::max(...).");
        return -1;
      }

      if (value > max_value) {
        max_value = value;
      }
    }

    return vm.push(max_value);
  }

  int_t zmath_random_impl(zs::vm_ref vm) {

    zs::parameter_stream ps(vm);
    ++ps;

    if (!ps) {
      std::mt19937 gen(std::random_device{}());
      return vm.push_integer(
          std::uniform_int_distribution((int_t)0, (std::numeric_limits<int_t>::max)())(gen));
    }

    int_t a = 0;
    if (auto err = ps.require<number_parameter>(a)) {
      vm->ZS_VM_ERROR(err, "A number was expected in math::random(...).");
      return -1;
    }

    std::mt19937 gen(std::random_device{}());
    return vm.push_integer(std::uniform_int_distribution((int_t)0, a)(gen));
  }

  int_t zmath_random_uniform_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    ++ps;

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "A two number were expected in math::random_uniform(a, b).");
      return -1;
    }

    if (ps.has_type(object_type::k_float)) {
      float_t a = 0;
      float_t b = 0;
      if (auto err = ps.require<number_parameter>(a)) {
        vm->ZS_VM_ERROR(err, "A number was expected in math::random_uniform(...).");
        return -1;
      }

      if (auto err = ps.require<number_parameter>(b)) {
        vm->ZS_VM_ERROR(err, "A number was expected in math::random_uniform(...).");
        return -1;
      }

      std::mt19937 gen(std::random_device{}());
      return vm.push_float(std::uniform_real_distribution(a, b)(gen));
    }

    int_t a = 0;
    int_t b = 0;
    if (auto err = ps.require<number_parameter>(a)) {
      vm->ZS_VM_ERROR(err, "A number was expected in math::random_uniform(...).");
      return -1;
    }

    if (auto err = ps.require<number_parameter>(b)) {
      vm->ZS_VM_ERROR(err, "A number was expected in math::random_uniform(...).");
      return -1;
    }

    std::mt19937 gen(std::random_device{}());
    return vm.push_integer(std::uniform_int_distribution(a, b)(gen));
  }

  int_t zmath_random_normal_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    ++ps;

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "A two number were expected in math::random_uniform(a, b).");
      return -1;
    }

    float_t mean = 0;
    float_t stddev = 0;
    if (auto err = ps.require<number_parameter>(mean)) {
      vm->ZS_VM_ERROR(err, "A number was expected in math::random_uniform(...).");
      return -1;
    }

    if (auto err = ps.require<number_parameter>(stddev)) {
      vm->ZS_VM_ERROR(err, "A number was expected in math::random_uniform(...).");
      return -1;
    }

    // If stddev is zero, let's return the mean.
    if (stddev == 0.0) {
      return vm.push_float(mean);
    }
    else if (stddev < 0) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_range, "Invalid standard deviation in math::random_uniform(...).");
      return -1;
    }

    std::mt19937 gen(std::random_device{}());
    return vm.push_float(std::normal_distribution(mean, stddev)(gen));
  }

} // namespace.

zs::object create_math_lib(zs::vm_ref vm) {
  zs::engine* eng = vm->get_engine();

  zs::object math_table = zs::_t(eng);
  zs::table_object& tbl = math_table.as_table();
  tbl.reserve(50);

  tbl.emplace(_ss("min"), zmath_min_impl);
  tbl.emplace(_ss("max"), zmath_max_impl);

  tbl.emplace(_ss("random"), zmath_random_impl);
  tbl.emplace(_ss("rand_uniform"), zmath_random_uniform_impl);
  tbl.emplace(_ss("rand_normal"), zmath_random_normal_impl);

  tbl.emplace(_ss("ceil"), zmath_trigo_impl<std::ceil<float_t>>);
  tbl.emplace(_ss("floor"), zmath_trigo_impl<std::floor<float_t>>);
  tbl.emplace(_ss("round"), zmath_trigo_impl<std::round<float_t>>);
  tbl.emplace(_ss("abs"), zmath_trigo_impl<(float_t(*)(float_t))std::abs>);

  //
  // Log/Exponential.
  //

  tbl.emplace(_ss("exp"), zmath_trigo_impl<std::exp<float_t>>);
  tbl.emplace(_ss("ln"), zmath_trigo_impl<std::log<float_t>>);
  tbl.emplace(_ss("log2"), zmath_trigo_impl<std::log2<float_t>>);
  tbl.emplace(_ss("log"), zmath_trigo_impl<std::log10<float_t>>);
  tbl.emplace(_ss("sqrt"), zmath_trigo_impl<std::sqrt<float_t>>);
  tbl.emplace(_ss("cbrt"), zmath_trigo_impl<std::cbrt<float_t>>);

  //
  // Trigo.
  //

  tbl.emplace(_ss("sin"), zmath_trigo_impl<std::sin<float_t>>);
  tbl.emplace(_ss("cos"), zmath_trigo_impl<std::cos<float_t>>);
  tbl.emplace(_ss("tan"), zmath_trigo_impl<std::tan<float_t>>);

  tbl.emplace(_ss("asin"), zmath_trigo_impl<std::asin<float_t>>);
  tbl.emplace(_ss("acos"), zmath_trigo_impl<std::acos<float_t>>);
  tbl.emplace(_ss("atan"), zmath_trigo_impl<std::atan<float_t>>);

  tbl.emplace(_ss("sinh"), zmath_trigo_impl<std::sinh<float_t>>);
  tbl.emplace(_ss("cosh"), zmath_trigo_impl<std::cosh<float_t>>);
  tbl.emplace(_ss("tanh"), zmath_trigo_impl<std::tanh<float_t>>);

  tbl.emplace(_ss("asinh"), zmath_trigo_impl<std::asinh<float_t>>);
  tbl.emplace(_ss("acosh"), zmath_trigo_impl<std::acosh<float_t>>);
  tbl.emplace(_ss("atanh"), zmath_trigo_impl<std::atanh<float_t>>);

  //
  // Constants.
  //

  tbl.emplace(_ss("zero"), zb::zero<float_t>);
  tbl.emplace(_ss("one"), zb::one<float_t>);
  tbl.emplace(_ss("minus_one"), zb::minus_one<float_t>);
  tbl.emplace(_ss("e"), zb::e<float_t>);
  tbl.emplace(_ss("pi"), zb::pi<float_t>);
  tbl.emplace(_ss("two_pi"), zb::two_pi<float_t>);
  tbl.emplace(_ss("four_pi"), zb::four_pi<float_t>);
  tbl.emplace(_ss("eight_pi"), zb::eight_pi<float_t>);
  tbl.emplace(_ss("two_pi_squared"), zb::two_pi_squared<float_t>);
  tbl.emplace(_ss("one_over_pi"), zb::one_over_pi<float_t>);
  tbl.emplace(_ss("two_over_pi"), zb::two_over_pi<float_t>);
  tbl.emplace(_ss("four_over_pi"), zb::four_over_pi<float_t>);
  tbl.emplace(_ss("eight_over_pi"), zb::eight_over_pi<float_t>);
  tbl.emplace("one_over_pi_squared", zb::one_over_pi_squared<float_t>);
  tbl.emplace("two_over_pi_squared", zb::two_over_pi_squared<float_t>);
  tbl.emplace("four_over_pi_squared", zb::four_over_pi_squared<float_t>);
  tbl.emplace("eight_over_pi_squared", zb::eight_over_pi_squared<float_t>);
  tbl.emplace(_ss("pi_over_eight"), zb::pi_over_eight<float_t>);
  tbl.emplace(_ss("pi_over_four"), zb::pi_over_four<float_t>);
  tbl.emplace(_ss("pi_over_two"), zb::pi_over_two<float_t>);
  tbl.emplace(_ss("sqrt_2"), zb::sqrt_2<float_t>);
  tbl.emplace(_ss("sqrt_2_over_2"), zb::sqrt_2_over_2<float_t>);
  tbl.emplace(_ss("log_2"), zb::log_2<float_t>);
  tbl.emplace(_ss("log_10"), zb::log_10<float_t>);
  tbl.emplace(_ss("log_pi"), zb::log_pi<float_t>);
  tbl.emplace(_ss("log_two_pi"), zb::log_two_pi<float_t>);
  tbl.emplace("log_sqrt_two_pi", zb::log_sqrt_two_pi<float_t>);

  return math_table;
}
} // namespace zs.
