namespace zs {
namespace {} // namespace

zs::object create_color_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();

  tbl.reserve(20);

  tbl["red"] = ZS_FUNCTION_DEF { return vm.push((int_t)zb::color(vm[0]._color).red()); };
  tbl["green"] = ZS_FUNCTION_DEF { return vm.push((int_t)zb::color(vm[0]._color).green()); };
  tbl["blue"] = ZS_FUNCTION_DEF { return vm.push((int_t)zb::color(vm[0]._color).blue()); };
  tbl["alpha"] = ZS_FUNCTION_DEF { return vm.push((int_t)zb::color(vm[0]._color).alpha()); };

  tbl["redf"] = ZS_FUNCTION_DEF { return vm.push((float_t)zb::color(vm[0]._color).f_red()); };
  tbl["greenf"] = ZS_FUNCTION_DEF { return vm.push((float_t)zb::color(vm[0]._color).f_green()); };
  tbl["bluef"] = ZS_FUNCTION_DEF { return vm.push((float_t)zb::color(vm[0]._color).f_blue()); };
  tbl["alphaf"] = ZS_FUNCTION_DEF { return vm.push((float_t)zb::color(vm[0]._color).f_alpha()); };

  tbl["with_red"] = ZS_FUNCTION_DEF {
    return vm.push(create_color(zb::color(vm[0]._color).red((uint8_t)vm[1].to_int())));
  };

  tbl["with_green"] = ZS_FUNCTION_DEF {
    return vm.push(create_color(zb::color(vm[0]._color).green((uint8_t)vm[1].to_int())));
  };

  tbl["with_blue"] = ZS_FUNCTION_DEF {
    return vm.push(create_color(zb::color(vm[0]._color).blue((uint8_t)vm[1].to_int())));
  };

  tbl["with_alpha"] = ZS_FUNCTION_DEF {
    return vm.push(create_color(zb::color(vm[0]._color).alpha((uint8_t)vm[1].to_int())));
  };

  tbl["components"] = ZS_FUNCTION_DEF {
    zb::color col(vm[0]._color);
    return vm.push(zs::_a(
        vm.get_engine(), { (int_t)col.red(), (int_t)col.green(), (int_t)col.blue(), (int_t)col.alpha() }));
  };

  tbl["fcomponents"] = ZS_FUNCTION_DEF {
    zb::color col(vm[0]._color);
    return vm.push(zs::_a(vm.get_engine(),
        { (float_t)col.f_red(), (float_t)col.f_green(), (float_t)col.f_blue(), (float_t)col.f_alpha() }));
  };

  return obj;
}

zs::object create_array_iterator_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();
  tbl.reserve(10);

  tbl["next"] = ZS_FUNCTION_DEF {

    object next_it = vm[0];
    next_it._array_it++;
    next_it._reserved_u32++;
    return vm.push(next_it);
  };

  tbl["get"] = ZS_FUNCTION_DEF { return vm.push(*(vm[0]._array_it)); };

  tbl["get_if_not"] = ZS_FUNCTION_DEF {

    if (vm[0]._array_it != vm[1]._array_it) {
      return vm.push(*(vm[0]._array_it));
    }

    return vm.push_null();
  };

  tbl["safe_get"] = ZS_FUNCTION_DEF {

    if (vm[0]._array_it != vm[1].as_array().data() + vm[1].as_array().size()) {
      return vm.push(*(vm[0]._array_it));
    }

    return vm.push_null();
  };

  tbl["get_key"] = ZS_FUNCTION_DEF { return vm.push(vm[0]._reserved_u32); };

  tbl["safe_key"] = ZS_FUNCTION_DEF {

    if (vm[0]._array_it != vm[1].as_array().data() + vm[1].as_array().size()) {
      return vm.push(vm[0]._reserved_u32);
    }

    return vm.push_null();
  };

  tbl["get_key_if_not"] = ZS_FUNCTION_DEF {
    if (vm[0]._array_it != vm[1]._array_it) {
      return vm.push(vm[0]._reserved_u32);
    }

    return vm.push_null();
  };

  return obj;
}

zs::object create_table_iterator_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();
  tbl.reserve(10);

  tbl["next"] = ZS_FUNCTION_DEF {

    object next_it = vm[0];
    next_it._table_it.get()++;
    return vm.push(next_it);
  };

  tbl["get"] = ZS_FUNCTION_DEF { return vm.push(vm[0]._table_it.get()->second); };

  tbl["get_if_not"] = ZS_FUNCTION_DEF {

    if (vm[0]._table_it.get() != vm[1]._table_it.get()) {
      return vm.push(vm[0]._table_it.get()->second);
    }

    return vm.push_null();
  };

  tbl["safe_get"] = ZS_FUNCTION_DEF {

    if (vm[0]._table_it.get() != vm[1].as_table().end()) {
      return vm.push(vm[0]._table_it.get()->second);
    }

    return vm.push_null();
  };

  tbl["get_key"] = ZS_FUNCTION_DEF { return vm.push(vm[0]._table_it.get()->first); };

  tbl["safe_key"] = ZS_FUNCTION_DEF {

    if (vm[0]._table_it.get() != vm[1].as_table().end()) {
      return vm.push(vm[0]._table_it.get()->first);
    }

    return vm.push_null();
  };

  tbl["get_key_if_not"] = ZS_FUNCTION_DEF {

    if (vm[0]._table_it.get() != vm[1]._table_it.get()) {
      return vm.push(vm[0]._table_it.get()->first);
    }

    return vm.push_null();
  };

  return obj;
}
} // namespace zs.
