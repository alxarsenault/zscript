#include <zscript/std/zmath.h>
#include "zvirtual_machine.h"
#include <random>

namespace zs {

namespace {
#define ZMATH_TRIGO(op)                     \
  int_t zmath_##op(zs::vm_ref vm) {         \
    zs::float_t res;                        \
    if (auto err = vm.get_float(-1, res)) { \
      return -1;                            \
    }                                       \
                                            \
    vm.push_float(std::op(res));            \
    return 1;                               \
  }

  int_t zmath_sin(zs::vm_ref vm) {
    if (zs::optional_result<zs::float_t> res = vm.get_float(-1)) {
      vm.push_float(std::sin(res.value()));
      return 1;
    }

    return -1;
  }

  //  ZMATH_TRIGO(sin)
  ZMATH_TRIGO(cos)
  ZMATH_TRIGO(tan)
  ZMATH_TRIGO(log)
  ZMATH_TRIGO(log2)
  ZMATH_TRIGO(log10)
  ZMATH_TRIGO(abs)
  ZMATH_TRIGO(sqrt)
  ZMATH_TRIGO(ceil)
  ZMATH_TRIGO(floor)
  ZMATH_TRIGO(round)

  int_t zmath_min(zs::vm_ref vm) {
    int_t count = vm.stack_size();

    bool has_float = false;
    for (int_t i = 1; i < count; i++) {
      const object& obj = vm->stack()[i];
      if (!obj.is_number()) {
        return -1;
      }

      if (obj.is_float()) {
        has_float = true;
      }
    }

    if (has_float) {
      zs::float_t vmin = (std::numeric_limits<float_t>::max)();
      zs::float_t res;
      for (int_t i = 1; i < count; i++) {
        if (auto err = vm.get_float(i, res)) {
          return -1;
        }

        if (res < vmin) {
          vmin = res;
        }
      }

      vm.push_float(vmin);
      return 1;
    }
    else {
      zs::int_t vmin = (std::numeric_limits<int_t>::max)();
      zs::int_t res;
      for (int_t i = 1; i < count; i++) {

        if (auto err = vm.get_integer(i, res)) {
          return -1;
        }

        if (res < vmin) {
          vmin = res;
        }
      }

      vm.push_integer(vmin);
      return 1;
    }
  }

  int_t zmath_max(zs::vm_ref vm) {
    int_t count = vm.stack_size();

    bool has_float = false;
    for (int_t i = 1; i < count; i++) {
      const object& obj = vm->stack()[i];
      if (!obj.is_number()) {
        return -1;
      }

      if (obj.is_float()) {
        has_float = true;
      }
    }

    if (has_float) {
      zs::float_t vmax = -(std::numeric_limits<float_t>::max)();
      zs::float_t res;
      for (int_t i = 1; i < count; i++) {

        if (auto err = vm.get_float(i, res)) {
          return -1;
        }

        if (res > vmax) {
          vmax = res;
        }
      }

      vm.push_float(vmax);
      return 1;
    }
    else {

      zs::int_t vmax = -(std::numeric_limits<int_t>::max)();
      zs::int_t res;
      for (int_t i = 1; i < count; i++) {

        if (auto err = vm.get_integer(i, res)) {
          return -1;
        }

        if (res > vmax) {
          vmax = res;
        }
      }

      vm.push_integer(vmax);
      return -1;
    }
  }

  static int_t zmath_random_uniform(zs::vm_ref vm) {
    const int_t count = vm.stack_size();
    if (count != 3) {
      zb::print("Error: math.zmath_random_uniform (a, b)");
      return -1;
    }

    const object& a = vm->stack()[-2];
    const object& b = vm->stack()[-1];

    if (!a.is_number() || !b.is_number()) {
      zb::print("Error: math.zmath_random_uniform (a, b) - invalid types");
      return -1;
    }

    if (a.is_float() || b.is_float()) {
      // Real distribution.
      float_t f_a = 0;
      if (auto err = a.get_float(f_a)) {
        zb::print("Error: math.zmath_random_uniform - convertion error");
        return -1;
      }

      float_t f_b = 0;
      if (auto err = b.get_float(f_b)) {
        zb::print("Error: math.zmath_random_uniform - convertion error");
        return -1;
      }

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<float_t> dis(f_a, f_b);

      vm.push_float(dis(gen));
      return 1;
    }

    // Integer distribution.
    int_t i_a = 0;
    if (auto err = a.get_integer(i_a)) {
      zb::print("Error: math.zmath_random_uniform - convertion error");
      return -1;
    }

    int_t i_b = 0;
    if (auto err = b.get_integer(i_b)) {
      zb::print("Error: math.zmath_random_uniform - convertion error");
      return -1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int_t> dis(i_a, i_b);

    vm.push_integer(dis(gen));
    return 1;
  }

  static int_t zmath_random_normal(zs::vm_ref vm) {
    const int_t count = vm.stack_size();
    if (count != 3) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    const object& a = vm->stack()[-2];
    const object& b = vm->stack()[-1];

    if (!a.is_number() || !b.is_number()) {
      zb::print("Error: math.zmath_random_normal (a, b) - invalid types");
      return -1;
    }

    float_t f_mean = 0;
    if (auto err = a.get_float(f_mean)) {
      zb::print("Error: math.zmath_random_normal - convertion error");
      return -1;
    }

    float_t f_stddev = 0;
    if (auto err = b.get_float(f_stddev)) {
      zb::print("Error: math.zmath_random_normal - convertion error");
      return -1;
    }

    if (f_stddev < 0) {
      zb::print("Error: math.zmath_random_normal - stddev < 0");
      return -1;
    }

    // If the standard deviation is zero, let's just return f_mean.
    if (f_stddev == 0) {
      vm.push_float(f_mean);
      return 1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float_t> dis(f_mean, f_stddev);

    vm.push_float(dis(gen));
    return 1;
  }

  //
  // MARK: normal_distribution
  //

  struct normal_distribution {

    inline normal_distribution(float_t mean, float_t stddev)
        : _gen(std::random_device{}())
        , _dist(mean, stddev) {}

    static int_t generate(zs::vm_ref vm) {
      const object& obj = vm->top();
      user_data_object* uobj = obj._udata;
      normal_distribution* nd = uobj->data<normal_distribution>();
      vm->push(nd->_dist(nd->_gen));
      return 1;
    }

    static int_t get_mean(zs::vm_ref vm) {
      const int_t count = vm.stack_size();
      if (count != 1) {
        zb::print("Error: math.zmath_random_normal (a, b)");
        return -1;
      }

      const object& obj = vm->top();
      user_data_object* uobj = obj._udata;
      normal_distribution* nd = uobj->data<normal_distribution>();
      vm->push(nd->_dist.mean());

      return 1;
    }

    static int_t set_mean(zs::vm_ref vm) {
      const int_t count = vm.stack_size();
      if (count != 2) {
        zb::print("Error: math.zmath_random_normal (a, b)");
        return -1;
      }

      const object& a = vm->stack()[-1];

      if (!a.is_number()) {
        zb::print("Error: math.zmath_random_normal (a, b) - invalid types");
        return -1;
      }

      float_t f_mean = 0;
      if (auto err = a.get_float(f_mean)) {
        zb::print("Error: math.zmath_random_normal - convertion error");
        return -1;
      }

      object& obj = vm->stack()[-2];
      user_data_object* uobj = obj._udata;
      normal_distribution* nd = uobj->data<normal_distribution>();
      nd->_dist.param(std::normal_distribution<float_t>::param_type{ f_mean, nd->_dist.stddev() });

      return 1;
    }

    static int_t _get(zs::vm_ref vm) {
      object& obj = vm->stack()[-2];
      const object& key = vm->top();

      if (key == "mean") {

        user_data_object* uobj = obj._udata;
        normal_distribution* nd = uobj->data<normal_distribution>();
        vm->push(nd->_dist.mean());
        return 1;
        ;
      }
      else if (key == "stddev") {

        user_data_object* uobj = obj._udata;
        normal_distribution* nd = uobj->data<normal_distribution>();
        vm->push(nd->_dist.stddev());
        return 1;
        ;
      }

      return -1;
    }

    static int_t _set(zs::vm_ref vm) {
      object& obj = vm->stack()[-3];
      object& key = vm->stack()[-2];
      const object& value = vm->top();
      if (key == "mean") {

        float_t f_mean = 0;
        if (auto err = value.get_float(f_mean)) {
          zb::print("Error: math.zmath_random_normal - convertion error");
          return -1;
        }

        user_data_object* uobj = obj._udata;
        normal_distribution* nd = uobj->data<normal_distribution>();
        //      vm->push(nd->_dist.mean());
        nd->_dist.param(std::normal_distribution<float_t>::param_type{ f_mean, nd->_dist.stddev() });

        return 0;
      }
      else if (key == "stddev") {
        float_t f_stddev = 0;
        if (auto err = value.get_float(f_stddev)) {
          zb::print("Error: math.zmath_random_normal - convertion error");
          return -1;
        }

        user_data_object* uobj = obj._udata;
        normal_distribution* nd = uobj->data<normal_distribution>();
        nd->_dist.param(std::normal_distribution<float_t>::param_type{ nd->_dist.mean(), f_stddev });

        return 0;
      }

      return -1;
    }

    std::mt19937 _gen;
    std::normal_distribution<float_t> _dist;
  };

  static int_t zmath_create_normal_distribution(zs::vm_ref vm) {
    const int_t count = vm.stack_size();
    if (count != 3) {
      zb::print("Error: math.zmath_create_normal_distribution (a, b)");
      return -1;
    }

    zs::engine* eng = vm.get_engine();
    const object& a = vm->stack()[-2];
    const object& b = vm->stack()[-1];

    float_t f_mean = 0;
    if (auto err = a.get_float(f_mean)) {
      zb::print("Error: math.zmath_random_normal - convertion error");
      return -1;
    }

    float_t f_stddev = 0;
    if (auto err = b.get_float(f_stddev)) {
      zb::print("Error: math.zmath_random_normal - convertion error");
      return -1;
    }
    object obj = zs::object::create_user_data<normal_distribution>(eng, f_mean, f_stddev);
    zs::user_data_object* uobj = obj._udata;

    object delegate_obj = object::create_table(eng);
    table_object* delegate = delegate_obj._table;
    uobj->set_delegate(delegate_obj);

    object gen = zs::_nc(eng, normal_distribution::generate);
    delegate->set(zs::_ss("gen"), gen);
    delegate->set(zs::constants::get<meta_method::mt_call>(), gen);
    delegate->set(zs::constants::get<meta_method::mt_get>(), zs::_nc(eng, normal_distribution::_get));
    delegate->set(zs::constants::get<meta_method::mt_set>(), zs::_nc(eng, normal_distribution::_set));
    delegate->set(zs::_ss("set_mean"), zs::_nc(eng, normal_distribution::set_mean));
    delegate->set(zs::_ss("get_mean"), zs::_nc(eng, normal_distribution::get_mean));

    vm->push(obj);
    return 1;
  }

} // namespace.

zs::object create_math_lib(zs::vm_ref vm) {
  zs::engine* eng = vm->get_engine();

  zs::object math_table = zs::object::create_table(vm->get_engine());
  zs::table_object* math_tbl = math_table._table;
  math_tbl->reserve(20);

  math_tbl->set(zs::_ss("sin"), zs::_nc(eng, zmath_sin));
  math_tbl->set(zs::_ss("cos"), zs::_nc(eng, zmath_cos));
  math_tbl->set(zs::_ss("tan"), zs::_nc(eng, zmath_tan));
  math_tbl->set(zs::_ss("ln"), zs::_nc(eng, zmath_log));
  math_tbl->set(zs::_ss("log2"), zs::_nc(eng, zmath_log2));
  math_tbl->set(zs::_ss("log"), zs::_nc(eng, zmath_log10));
  math_tbl->set(zs::_ss("abs"), zs::_nc(eng, zmath_abs));
  math_tbl->set(zs::_ss("sqrt"), zs::_nc(eng, zmath_sqrt));
  math_tbl->set(zs::_ss("ceil"), zs::_nc(eng, zmath_ceil));
  math_tbl->set(zs::_ss("floor"), zs::_nc(eng, zmath_floor));
  math_tbl->set(zs::_ss("round"), zs::_nc(eng, zmath_round));
  math_tbl->set(zs::_ss("min"), zs::_nc(eng, zmath_min));
  math_tbl->set(zs::_ss("max"), zs::_nc(eng, zmath_max));

  math_tbl->set(zs::_ss("rand_uniform"), zs::_nc(eng, zmath_random_uniform));
  math_tbl->set(zs::_ss("rand_normal"), zs::_nc(eng, zmath_random_normal));

  math_tbl->set(zs::_ss("normal_dist"), zs::_nc(eng, zmath_create_normal_distribution));

  // Constants.
  math_tbl->set(zs::_ss("pi"), object(zb::pi<float_t>));
  math_tbl->set(zs::_ss("e"), object(zb::e<float_t>));

  return math_table;
}
} // namespace zs.
