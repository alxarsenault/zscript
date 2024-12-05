namespace zs {

template <exposed_object_type Type>
zs::error_result virtual_machine::arithmetic_operation(enum arithmetic_uop op, object& target, object& src) {
  target = nullptr;
  return zs::error_code::invalid_operation;
}

// Integer.
template <>
zs::error_result virtual_machine::arithmetic_operation<exposed_object_type::ke_integer>(
    enum arithmetic_uop op, object& target, object& src) {

  int_t a = src._int;

  switch (op) {
  case arithmetic_uop::incr:
    target = src;
    src = a + 1;
    return {};
  case arithmetic_uop::decr:
    target = src;
    src = a - 1;
    return {};
  case arithmetic_uop::pincr:
    target = src = a + 1;
    return {};
  case arithmetic_uop::pdecr:
    target = src = a - 1;
    return {};
  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }
  return zs::error_code::invalid_operation;
}

// ext.
template <>
zs::error_result virtual_machine::arithmetic_operation<exposed_object_type::ke_extension>(
    enum arithmetic_uop op, object& target, object& src) {

  if (src.is_array_iterator()) {

    switch (op) {
    case arithmetic_uop::incr:
      target = src;
      src._array_it++;
      src._reserved_u32++;

      return {};
    case arithmetic_uop::decr:
      target = src;
      src._array_it--;
      src._reserved_u32--;
      return {};
    case arithmetic_uop::pincr:
      src._array_it++;
      src._reserved_u32++;
      target = src;
      return {};
    case arithmetic_uop::pdecr:
      src._array_it--;
      src._reserved_u32--;
      target = src;
      return {};
    default:
      target = nullptr;
      return zs::error_code::invalid_operation;
    }
  }

  if (src.is_table_iterator()) {

    switch (op) {
    case arithmetic_uop::incr:
      target = src;
      src._table_it.get()++;
      return {};
    case arithmetic_uop::pincr:
      ++(src._table_it.get());
      target = src;
      return {};
    default:
      target = nullptr;
      return zs::error_code::invalid_operation;
    }
  }
  return zs::error_code::invalid_operation;
}

//
//
//

template <exposed_object_type LType, exposed_object_type RType>
zs::error_result virtual_machine::arithmetic_operation(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  target = nullptr;
  return zs::error_code::invalid_operation;
}

template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_float>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs);

template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs);

//
// MARK: Bool
//

// Bool and Bool.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_bool, exposed_object_type::ke_bool>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_integer>(
      op, target, object((int_t)lhs._bool), object((int_t)rhs._bool));
}

// Bool and Integer.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_bool, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_integer>(
      op, target, object((int_t)lhs._bool), rhs);
}

// Bool and Float.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_bool, exposed_object_type::ke_float>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_float>(
      op, target, object((float_t)lhs._bool), rhs);
}

//
// MARK: Integer
//

// Integer and Bool.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_bool>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_integer>(
      op, target, lhs, object((int_t)rhs._bool));
}

// Integer and Integer.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  int_t a = lhs._int;
  int_t b = rhs._int;

  switch (op) {
  case arithmetic_op::aop_add:
    target = a + b;
    return {};
  case arithmetic_op::aop_sub:
    target = a - b;
    return {};
  case arithmetic_op::aop_mul:
    target = a * b;
    return {};
  case arithmetic_op::aop_div:
    target = a / b;
    return {};
  case arithmetic_op::aop_mod:
    target = a % b;
    return {};
  case arithmetic_op::aop_exp:
    target = (int_t)std::pow(a, b);
    return {};
  case arithmetic_op::aop_bitwise_or:
    target = a | b;
    return {};
  case arithmetic_op::aop_bitwise_and:
    target = a & b;
    return {};
  case arithmetic_op::aop_bitwise_xor:
    target = a ^ b;
    return {};
  case arithmetic_op::aop_lshift:
    target = a << b;
    return {};
  case arithmetic_op::aop_rshift:
    target = a >> b;
    return {};

  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }
  return zs::error_code::invalid_operation;
}

// Integer and Float.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_integer, exposed_object_type::ke_float>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  float_t bf = rhs._float;
  int_t a = lhs._int;
  int_t b = bf;
  float_t af = a;

  switch (op) {
  case arithmetic_op::aop_add:
    target = af + bf;
    return {};
  case arithmetic_op::aop_sub:
    target = af - bf;
    return {};
  case arithmetic_op::aop_mul:
    target = af * bf;
    return {};
  case arithmetic_op::aop_div:
    target = af / bf;
    return {};
  case arithmetic_op::aop_mod:
    target = std::fmod(af, bf);
    return {};
  case arithmetic_op::aop_exp:
    target = std::pow(af, bf);
    return {};

  case arithmetic_op::aop_bitwise_or:
    if (b != bf) {
      set_error("Can't do bitwise or with an integer and a float with decimal.\n");
      target = nullptr;
      return zs::error_code::invalid_operation;
    }

    target = a | b;
    return {};
  case arithmetic_op::aop_bitwise_and:
    if (b != bf) {
      set_error("Can't do bitwise and with an integer and a float with decimal.\n");
      target = nullptr;
      return zs::error_code::invalid_operation;
    }

    target = a & b;
    return {};
  case arithmetic_op::aop_bitwise_xor:
    if (b != bf) {
      set_error("Can't do xor with an integer and a float with decimal.\n");
      target = nullptr;
      return zs::error_code::invalid_operation;
    }

    target = a ^ b;
    return {};
  case arithmetic_op::aop_lshift:
    if (b != bf) {
      set_error("Can't do left shift with an integer and a float with decimal.\n");
      target = nullptr;
      return zs::error_code::invalid_operation;
    }

    target = a << b;
    return {};
  case arithmetic_op::aop_rshift:
    if (b != bf) {
      set_error("Can't do right shift with an integer and a float with decimal.\n");
      target = nullptr;
      return zs::error_code::invalid_operation;
    }

    target = a >> b;
    return {};

  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  return zs::error_code::invalid_operation;
}

//
// MARK: Float
//

// Float and Bool.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_bool>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_float>(
      op, target, lhs, object((float_t)rhs._bool));
}

// Float and Integer.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_float>(
      op, target, lhs, object((float_t)rhs._int));
}

// Float and Float.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_float, exposed_object_type::ke_float>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  float_t a = lhs._float;
  float_t b = rhs._float;

  switch (op) {
  case arithmetic_op::aop_add:
    target = a + b;
    return {};
  case arithmetic_op::aop_sub:
    target = a - b;
    return {};
  case arithmetic_op::aop_mul:
    target = a * b;
    return {};
  case arithmetic_op::aop_div:
    target = a / b;
    return {};
  case arithmetic_op::aop_mod:
    target = std::fmod(a, b);
    return {};
  case arithmetic_op::aop_exp:
    target = std::pow(a, b);
    return {};
  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  return zs::error_code::invalid_operation;
}

//
// MARK: String
//

// String and String.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_string>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  std::string_view a = lhs.get_string_unchecked();
  std::string_view b = rhs.get_string_unchecked();

  switch (op) {
  case arithmetic_op::aop_add:
    target = zs::object::create_concat_string(_engine, a, b);
    return {};

  case arithmetic_op::aop_sub: {
    std::string::size_type i = a.find(b);
    if (i == std::string::npos) {
      target = lhs;
      return {};
    }

    const size_t bsz = b.size();
    zs::string str(a, zs::allocator<char>(_engine));

    while (i != std::string::npos) {
      str.erase(i, bsz);
      i = str.find(b, i);
    }

    target = zs::_s(_engine, str);
    return {};
  }

  case arithmetic_op::aop_div: {
    //    auto split = a | std::ranges::views::split(b) | std::ranges::views::transform([](auto&& str) {
    //      return std::string_view(&*str.begin(), std::ranges::distance(str));
    //    });
    //
    //    zs::object arr_obj = zs::object::create_array(_engine, 0);
    //    zs::array_object& arr = arr_obj.as_array();
    //    for (auto&& k : split) {
    //      if (!k.empty()) {
    //        arr.push_back(zs::_s(_engine, k));
    //      }
    //    }
    //
    //    target = arr_obj;
    return {};
  }

  case arithmetic_op::aop_mod: {
    if (a == b) {
      target = lhs;
      return {};
    }

    const size_t sz = zb::minimum(a.size(), b.size());

    for (size_t i = 0; i < sz; i++) {
      if (a[i] != b[i]) {
        target = zs::_s(_engine, a.substr(0, i));
        return {};
      }
    }

    target = a.size() == sz ? lhs : rhs;
    return {};
  }
  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }
  return zs::error_code::invalid_operation;
}

// String and Integer.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  std::string_view a = lhs.get_string_unchecked();
  int_t b = rhs._int;

  switch (op) {
  case arithmetic_op::aop_add:
    target = nullptr;
    set_error("Can't add a string with an integer.\nUse tostring(v) to concat "
              "a string with a value.\n");
    return zs::error_code::invalid_operation;

  case arithmetic_op::aop_sub: {
    int_t bi = -b;
    const int_t asz = a.size();

    if (bi >= 0 && bi <= asz) {
      a = a.substr(0, asz - bi);
    }
    else if (bi < 0 && -bi <= asz) {
      a = a.substr(-bi);
    }

    target = zs::_s(_engine, a);
    return {};
  }

  case arithmetic_op::aop_mul: {
    const int_t asz = a.size();

    if (b <= 0) {
      set_error("Can't mul a string with a negative number.\n");
      return zs::error_code::invalid_operation;
    }

    zs::string mult_str(a, zs::allocator<char>(_engine));
    mult_str.resize(asz * b, ' ');

    const char* begin_buffer = mult_str.data();
    char* arun_ptr = mult_str.data() + asz;
    for (int_t i = 1; i < b; i++) {
      ::memcpy(arun_ptr, begin_buffer, asz);
      arun_ptr += asz;
    }

    target = zs::_s(_engine, mult_str);
    return {};
  }

  case arithmetic_op::aop_lshift: {
    if (b < 0) {
      set_error("Can't left shift a string with a negative number.\n");
      return zs::error_code::invalid_argument;
    }

    const int_t asz = a.size();
    target = zs::_s(_engine, a.substr(zb::minimum(b, asz)));
    return {};
  }

  case arithmetic_op::aop_rshift: {
    if (b < 0) {
      set_error("Can't right shift a string with a negative number.\n");
      return zs::error_code::invalid_argument;
    }

    const int_t asz = a.size();
    target = zs::_s(_engine, a.substr(0, asz - zb::minimum(b, asz)));
    return {};
  }

  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  return zs::error_code::invalid_operation;
}

// String and bool.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_bool>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  switch (op) {
  case arithmetic_op::aop_add:
    target = nullptr;
    set_error("Can't add a string with a boolean.\nUse tostring(v) to concat a "
              "string with a value.\n");
    return zs::error_code::invalid_operation;

  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }
  return zs::error_code::invalid_operation;
}

// String and Float.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_float>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  switch (op) {
  case arithmetic_op::aop_add:
    target = nullptr;
    set_error("Can't add a string with a float.\nUse tostring(v) to concat a "
              "string with a value.\n");
    return zs::error_code::invalid_operation;

  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }
  return zs::error_code::invalid_operation;
}

//
// MARK: Table
//

// Table and Integer.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_table, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  zs::table_object& tbl = lhs.as_table();
  if (tbl.has_delegate()) {
    zs::object& del = tbl.get_delegate();
    switch (op) {
    case arithmetic_op::aop_add:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_add, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));

    case arithmetic_op::aop_sub:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_sub, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
    case arithmetic_op::aop_mul:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_mul, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));

    default:
      break;
    }
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

// Table and String.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_table, exposed_object_type::ke_string>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  zs::table_object& tbl = lhs.as_table();

  if (tbl.has_delegate()) {
    zs::object& del = tbl.get_delegate();
    switch (op) {
    case arithmetic_op::aop_add:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_add, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
    case arithmetic_op::aop_sub:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_sub, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
    case arithmetic_op::aop_mul:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_mul, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));

    default:
      break;
    }
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

// Table and Table.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_table, exposed_object_type::ke_table>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (!zs::arithmetic_op_has_meta_method(op)) {
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  {
    bool has_delegate = lhs.has_delegate();

    meta_method mt = zs::arithmetic_op_to_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));
    object del = has_delegate ? lhs.get_delegate() : lhs;
    if (auto err = runtime_action<runtime_code::table_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else if (meta_func.is_function()) {
      push(lhs);
      push(rhs);
      int_t nparams = 3;
      if (meta_func.is_closure() and meta_func.as_closure().get_proto()._parameter_names.size() < 3) {
        nparams--;
      }
      else {

        push(del);
      }

      if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
        pop(nparams);
        return err;
      }

      pop(nparams);
      return {};
    }
    else {
      return zs::errc::invalid_type;
    }

    if (has_delegate) {
      return runtime_action<meta_arith>(
          zs::arithmetic_op_to_meta_method(op), CREF(lhs), CREF(rhs), REF(target), REF(del));
    }
  }

  {
    bool has_delegate = rhs.has_delegate();

    meta_method mt = zs::arithmetic_op_to_rhs_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));
    object del = has_delegate ? rhs.get_delegate() : rhs;
    if (auto err = runtime_action<runtime_code::table_get>(CREF(rhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else if (meta_func.is_function()) {
      push(rhs);
      push(lhs);
      int_t nparams = 3;
      if (meta_func.is_closure() and meta_func.as_closure().get_proto()._parameter_names.size() < 3) {
        nparams--;
      }
      else {

        push(del);
      }

      if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
        pop(nparams);
        return err;
      }

      pop(nparams);
      return {};
    }
    else {
      return zs::errc::invalid_type;
    }

    if (has_delegate) {
      return runtime_action<meta_arith>(
          zs::arithmetic_op_to_rhs_meta_method(op), CREF(rhs), CREF(lhs), REF(target), REF(del));
    }
  }

  //  if ( lhs.has_delegate()) {
  //
  //    meta_method mt = zs::arithmetic_op_to_meta_method(op);
  //    object meta_func;
  //    object key = zs::_ss(zs::meta_method_name(mt));
  //    object del =lhs.get_delegate();
  //    if (auto err = runtime_action<runtime_code::table_get>(CREF(lhs), CREF(key), REF(meta_func))) {
  //      if (err != zs::errc::not_found) {
  //        return err;
  //      }
  //    }
  //    else {
  //
  //      if (meta_func.is_function()) {
  //        push(lhs);
  //        push(rhs);
  //        int_t nparams = 3;
  //        if (meta_func.is_closure() and meta_func.as_closure().get_proto()._parameter_names.size() < 3) {
  //          nparams--;
  //        }
  //        else {
  //          push(del);
  //        }
  //
  //        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
  //          pop(nparams);
  //          return err;
  //        }
  //
  //        pop(nparams);
  //        return {};
  //      }
  //    }
  //
  //    return runtime_action<meta_arith>(zs::arithmetic_op_to_meta_method(op), CREF(lhs), CREF(rhs),
  //    REF(target),
  //        REF(del));
  //  }

  //  if ( rhs.has_delegate()) {
  //
  //    meta_method mt = zs::arithmetic_op_to_rhs_meta_method(op);
  //    object meta_func;
  //    object key = zs::_ss(zs::meta_method_name(mt));
  //    object del = rhs.get_delegate();
  //    if (auto err = runtime_action<runtime_code::table_get>(CREF(rhs), CREF(key), REF(meta_func))) {
  //      if (err != zs::errc::not_found) {
  //        return err;
  //      }
  //    }
  //    else {
  //
  //      if (meta_func.is_function()) {
  //        push(rhs);
  //        push(lhs);
  //        int_t nparams = 3;
  //        if (meta_func.is_closure() and meta_func.as_closure().get_proto()._parameter_names.size() < 3) {
  //          nparams--;
  //        }
  //        else {
  //          push(rhs .get_delegate());
  //        }
  //
  //        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
  //          pop(nparams);
  //          return err;
  //        }
  //
  //        pop(nparams);
  //        //      if (!dest->is_string()) {
  //        //        zb::print("Invalid typeof operator return type (should be a
  //        //        string)"); return zs::error_code::invalid_type;
  //        //      }
  //        return {};
  //      }
  //    }
  //
  //    return runtime_action<meta_arith>(zs::arithmetic_op_to_rhs_meta_method(op), CREF(rhs), CREF(lhs),
  //    REF(target),
  //        REF(del));
  //  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

//
// MARK: ext
//

template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_extension, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (lhs.is_array_iterator()) {

    switch (op) {
    case arithmetic_op::aop_add:
      target = lhs;
      target._array_it += rhs._int;
      target._reserved_u32 += rhs._int;
      return {};
    case arithmetic_op::aop_sub:
      target = lhs;
      target._array_it -= rhs._int;
      target._reserved_u32 -= rhs._int;

      return {};
    default:
      target = nullptr;
      return zs::error_code::invalid_operation;
    }
  }

  if (lhs.is_table_iterator()) {

    switch (op) {
    case arithmetic_op::aop_add:
      target = lhs;

      for (int_t i = 0; i < rhs._int; i++) {
        target._table_it.get()++;
      }

      return {};
    default:
      target = nullptr;
      return zs::error_code::invalid_operation;
    }
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

// UserData and String.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_user_data, exposed_object_type::ke_string>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  zs::user_data_object& tbl = lhs.as_udata();

  if (tbl.has_delegate()) {
    zs::object& del = tbl.get_delegate();
    switch (op) {
    case arithmetic_op::aop_add:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_add, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
    case arithmetic_op::aop_sub:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_sub, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));

    case arithmetic_op::aop_mul:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_mul, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));

    case arithmetic_op::aop_div:
      return runtime_action<runtime_code::meta_arith>(
          meta_method::mt_div, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));

    default:
      break;
    }
  }

  target = nullptr;
  return zs::errc::invalid_operation;
}

// UserData and UserData.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_user_data, exposed_object_type::ke_user_data>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (!zs::arithmetic_op_has_meta_method(op)) {
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  if (lhs.has_delegate()) {

    meta_method mt = zs::arithmetic_op_to_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));
    object del = lhs.get_delegate();
    if (auto err = runtime_action<runtime_code::user_data_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 3;
        if (meta_func.is_closure() and meta_func.as_closure().get_proto()._parameter_names.size() < 3) {
          nparams--;
        }
        else {
          push(del);
        }

        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }

    return runtime_action<meta_arith>(
        zs::arithmetic_op_to_meta_method(op), CREF(lhs), CREF(rhs), REF(target), REF(del));
  }

  if (rhs.has_delegate()) {

    meta_method mt = zs::arithmetic_op_to_rhs_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));
    object del = rhs.get_delegate();
    if (auto err = runtime_action<runtime_code::user_data_get>(CREF(rhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(rhs);
        push(lhs);
        int_t nparams = 3;
        if (meta_func.is_closure() and meta_func.as_closure().get_proto()._parameter_names.size() < 3) {
          nparams--;
        }
        else {
          push(rhs.get_delegate());
        }

        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        //      if (!dest->is_string()) {
        //        zb::print("Invalid typeof operator return type (should be a
        //        string)"); return zs::error_code::invalid_type;
        //      }
        return {};
      }
    }

    return runtime_action<meta_arith>(
        zs::arithmetic_op_to_rhs_meta_method(op), CREF(rhs), CREF(lhs), REF(target), REF(del));
  }

  target = nullptr;
  return zs::error_code::invalid_operation;

  //
  //  if (lhs.has_delegate()) {
  //    zs::object  del = tbl.get_delegate();
  //    switch (op) {
  //    case arithmetic_op::aop_add:
  //      return runtime_action<runtime_code::meta_arith>(
  //          meta_method::mt_add, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
  //
  //    case arithmetic_op::aop_sub:
  //      return runtime_action<runtime_code::meta_arith>(
  //          meta_method::mt_sub, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
  //
  //    case arithmetic_op::aop_mul:
  //      return runtime_action<runtime_code::meta_arith>(
  //          meta_method::mt_mul, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
  //
  //    case arithmetic_op::aop_div:
  //      return runtime_action<runtime_code::meta_arith>(
  //          meta_method::mt_div, zb::wcref(lhs), zb::wcref(rhs), zb::wref(target), zb::wref(del));
  //
  //    default:
  //      break;
  //    }
  //  }
  //
  //  target = nullptr;
  //  return zs::errc::invalid_operation;
}

// MutableString and String.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_mutable_string, exposed_object_type::ke_string>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  zs::mutable_string_object& tbl = lhs.as_mutable_string();

  if (tbl.has_delegate()) {
    if (zs::arithmetic_op_has_meta_method(op)) {
      zs::object& del = tbl.get_delegate();
      return runtime_action<meta_arith>(
          zs::arithmetic_op_to_meta_method(op), CREF(lhs), CREF(rhs), REF(target), REF(del));
    }

    target = nullptr;
    return zs::errc::invalid_operation;
  }

  return arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_string>(
      op, target, lhs, rhs);
}

// MutableString and MutableString.
template <>
zs::error_result virtual_machine::arithmetic_operation<exposed_object_type::ke_mutable_string,
    exposed_object_type::ke_mutable_string>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (zs::mutable_string_object& mstr = lhs.as_mutable_string(); mstr.has_delegate()) {
    if (zs::arithmetic_op_has_meta_method(op)) {
      return runtime_action<meta_arith>(
          zs::arithmetic_op_to_meta_method(op), CREF(lhs), CREF(rhs), REF(target), REF(mstr.get_delegate()));
    }

    target = nullptr;
    return zs::errc::invalid_operation;
  }

  if (zs::mutable_string_object& mstr = rhs.as_mutable_string(); mstr.has_delegate()) {
    if (zs::arithmetic_op_has_rhs_meta_method(op)) {
      return runtime_action<meta_arith>(zs::arithmetic_op_to_rhs_meta_method(op), CREF(rhs), CREF(lhs),
          REF(target), REF(mstr.get_delegate()));
    }

    target = nullptr;
    return zs::errc::invalid_operation;
  }

  return arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_string>(
      op, target, lhs, rhs);
}

// String and MutableString.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_mutable_string>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (zs::mutable_string_object& mstr = rhs.as_mutable_string(); rhs.has_delegate()) {
    if (zs::arithmetic_op_has_rhs_meta_method(op)) {
      return runtime_action<meta_arith>(zs::arithmetic_op_to_rhs_meta_method(op), CREF(rhs), CREF(lhs),
          REF(target), REF(mstr.get_delegate()));
    }

    target = nullptr;
    return zs::errc::invalid_operation;
  }

  return arithmetic_operation<exposed_object_type::ke_string, exposed_object_type::ke_string>(
      op, target, lhs, rhs);
}

// Instance and Instance.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_instance, exposed_object_type::ke_instance>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (!zs::arithmetic_op_has_meta_method(op)) {
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  if (!(lhs.is_struct_instance() and rhs.is_struct_instance())) {
    return zs::errc::unimplemented;
  }

  // Op X eq.
  if (target._struct_instance == lhs._struct_instance) {
    meta_method mt = zs::arithmetic_op_to_meta_eq_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 2;
        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  {
    meta_method mt = zs::arithmetic_op_to_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 2;
        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  {

    meta_method mt = zs::arithmetic_op_to_rhs_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(rhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(rhs);
        push(lhs);
        int_t nparams = 2;

        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

// Instance and Integer.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_instance, exposed_object_type::ke_integer>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (!zs::arithmetic_op_has_meta_method(op)) {
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  if (!(lhs.is_struct_instance())) {
    return zs::errc::unimplemented;
  }

  // Op X eq.
  if (target._struct_instance == lhs._struct_instance) {
    meta_method mt = zs::arithmetic_op_to_meta_eq_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 2;
        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  {
    meta_method mt = zs::arithmetic_op_to_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 2;
        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

// Instance and Float.
template <>
zs::error_result
virtual_machine::arithmetic_operation<exposed_object_type::ke_instance, exposed_object_type::ke_float>(
    enum arithmetic_op op, object& target, const object& lhs, const object& rhs) {

  if (!zs::arithmetic_op_has_meta_method(op)) {
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

  if (!(lhs.is_struct_instance())) {
    return zs::errc::unimplemented;
  }

  // Op X eq.
  if (target._struct_instance == lhs._struct_instance) {
    meta_method mt = zs::arithmetic_op_to_meta_eq_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 2;
        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  {
    meta_method mt = zs::arithmetic_op_to_meta_method(op);
    object meta_func;
    object key = zs::_ss(zs::meta_method_name(mt));

    if (auto err = runtime_action<runtime_code::struct_instance_get>(CREF(lhs), CREF(key), REF(meta_func))) {
      if (err != zs::errc::not_found) {
        return err;
      }
    }
    else {

      if (meta_func.is_function()) {
        push(lhs);
        push(rhs);
        int_t nparams = 2;
        if (auto err = call(meta_func, nparams, stack_size() - nparams, target)) {
          pop(nparams);
          return err;
        }

        pop(nparams);
        return {};
      }
    }
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

} // namespace zs.
