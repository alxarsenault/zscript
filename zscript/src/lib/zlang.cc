namespace zs {

namespace {
  int_t zlang_import_impl(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    const object& name = vm->top();

    if (nargs != 2 or !name.is_string()) {
      return -1;
    }

    zs::engine* eng = vm.get_engine();
    table_object& tbl = vm->get_imported_module_cache().as_table();

    object dest;
    if (!tbl.get(name, dest)) {
      return vm.push(dest);
    }

    return vm.push(object::create_none());
  }

int_t zlang_apply_impl(zs::vm_ref vm) {
  const int_t nargs = vm.stack_size();
 
  if(nargs < 2) {
    vm.set_error("Invalid number of parameters in zs::apply.\n");
    return -1;
  }

  const object& fct = vm[1];
  if(!fct.is_function()) {
    vm.set_error("Invalid function parameter in zs::apply.\n");
    return -1;
  }
  
  if(nargs == 2) {
    zs::object ret_value;
    if(auto err = vm->call(fct,  {vm[0]}, ret_value)) {
      return -1;
    }
    
    return vm.push(ret_value);
  }
  
  
  
  zs::vector<zs::object> args((zs::allocator<zs::object>(vm.get_engine())));
  
  for(int_t i = 2; i < nargs; i++) {
    const object& p = vm[i];
    if(p.is_array()) {
      const array_object& parr = p.as_array();
      for(const object& obj : parr) {
        args.push_back(obj);
      }
    }
    else {
      args.push_back(p);
    }
  }
 
  zs::object ret_value;
  if(auto err = vm->call(fct, std::span<const object>(args), ret_value)) {
    return -1;
  }
  
  return vm.push(ret_value);
  
}

 

  int_t zlang_setdelegate_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 3) {
      return -1;
    }

    object& dst = vm->stack_get(-2);
    object& delegate = vm->stack_get(-1);

    if (auto err = dst.set_delegate(delegate)) {
      return -1;
    }

    return 0;
  }

  int_t zlang_print_impl(zs::vm_ref vm) {
    const int_t sz = vm.stack_size();

    for (int_t i = 1; i < sz - 1; i++) {
      std::cout << vm->stack_get(i).convert_to_string() << " ";
    }

    std::cout << vm->stack_get(sz - 1).convert_to_string() << "\n";
    return 0;
  }

  int_t zlang_dprint_impl(zs::vm_ref vm) {
    const int_t sz = vm.stack_size();

    for (int_t i = 1; i < sz - 1; i++) {
      std::cout << vm->stack_get(i).to_debug_string() << " ";
    }

    std::cout << vm->stack_get(sz - 1).to_debug_string() << "\n";
    return 0;
  }

  int_t zlang_now_impl(zs::vm_ref vm) {
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    vm.push_integer(now);
    return 1;
  }

  int_t zlang_tostring_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_string()) {
      return vm.push(val);
    }

    zs::string s((zs::allocator<char>(vm.get_engine())));

    if (auto err = val.convert_to_string(s)) {
      vm.set_error("Invalid string convertion");
      return -1;
    }

    return vm.push(zs::_s(vm.get_engine(), s));
  }

  int_t zlang_toint_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_integer()) {
      return vm.push(val);
    }

    int_t v = 0;
    if (auto err = val.convert_to_integer(v)) {
      vm.set_error("Invalid integer convertion");
      return -1;
    }

    return vm.push(v);
  }

  int_t zlang_tofloat_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_float()) {
      return vm.push(val);
    }

    float_t v = 0;
    if (auto err = val.convert_to_float(v)) {
      vm.set_error("Invalid float convertion");
      return -1;
    }

    return vm.push(v);
  }

  int_t zlang_optget_impl(zs::vm_ref vm) {

    zs::object ret;
    if (auto err = vm->get(vm[1], vm[2], ret)) {
      return vm.push_null();
    }

    return vm.push(ret);
  }

  int_t zlang_create_array_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs == 1) {
      return vm.push(zs::_a(vm, 0));
    }

    const object& arg1 = vm[1];

    if (arg1.is_meta_argument()) {
      if (!arg1.is_array()) {
        return -1;
      }

      const array_object& meta_arr = arg1.as_array();
      if (meta_arr.size() != 1) {
        return -1;
      }

      if (!meta_arr[0].has_flags(object_flags_t::array_type)) {
        return -1;
      }

      native_array_type ntype = (native_array_type)meta_arr[0]._int;

      if (nargs >= 3) {

        const object& arg2 = vm[2];
        if (arg2.is_integer()) {
          return vm.push(zs::object::create_native_array(vm.get_engine(), ntype, arg2._int));
        }

        if (arg2.is_array()) {
          const array_object& in_arr = arg2.as_array();

          int_t arr_size = in_arr.size();
          auto arr = zs::object::create_native_array(vm.get_engine(), ntype, arr_size);

          zs::native_array_object_interface& arr_interface = arr.as_native_array_interface();

          for (int_t i = 0; i < arr_size; i++) {
            arr_interface.set(i, in_arr[i]);
          }

          return vm.push(std::move(arr));
        }

        return -1;
      }

      return vm.push(zs::object::create_native_array(vm.get_engine(), ntype, 0));
    }

    if (!arg1.is_integer()) {
      return -1;
    }

    return vm.push(zs::_a(vm, arg1._int));
  }

  int_t zlang_create_mutable_string_impl(zs::vm_ref vm) {
    return vm.push(zs::object::create_mutable_string(vm.get_engine(), vm[1].get_string_unchecked()));
  }

  int_t zlang_create_struct_impl(zs::vm_ref vm) {
    return vm.push(zs::object::create_struct(vm.get_engine()));
  }

  zs::error_result object_char_or_string_to_string(const object& obj, zs::string& s, std::string_view& sv) {

    if (obj.is_string()) {
      sv = obj.get_string_unchecked();
      return {};
    }
    if (obj.is_integer()) {
      s.clear();
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)obj._int), s);
      sv = s;
      return {};
    }

    return zs::error_code::invalid;
  }

  class tmp_string_view {
  public:
    inline tmp_string_view(zs::engine* eng)
        : _buffer((zs::string_allocator(eng))) {}

    inline tmp_string_view(zs::engine* eng, std::string_view s)
        : _buffer((zs::string_allocator(eng)))
        , _str(s) {}

    zs::error_result assign(const object& obj) { return object_char_or_string_to_string(obj, _buffer, _str); }

    inline bool empty() const noexcept { return _str.empty(); }

    friend inline std::ostream& operator<<(std::ostream& stream, const tmp_string_view& s) {
      if (!s.empty()) {
        return stream << s._str;
      }
      return stream;
    }
    zs::string _buffer;
    std::string_view _str;
  };
  //
  //  zs::error_result zlang_print_internal(zs::vm_ref vm, zb::span<const object> params, std::ostream&
  //  stream,
  //      std::string_view in_sep = "", std::string_view in_endl = "") {
  //
  //    zs::engine* eng = vm.get_engine();
  //
  //    tmp_string_view sep(eng, in_sep);
  //    tmp_string_view endl(eng, in_endl);
  //
  //    if (params.size() == 1) {
  //
  //      stream << endl._str;
  //
  //      return {};
  //    }
  //
  //    auto it = params.begin() + 1;
  //
  //    if (it->is_meta_argument()) {
  //      const zs::array_object& arr = it++->as_array();
  //      const int_t n_template = arr.size();
  //
  //      if (n_template <= 0 or n_template > 2) {
  //        vm.set_error("Invalid meta arguments count in zs.print.\n");
  //        return zs::error_code::invalid;
  //      }
  //
  //      if (auto err = sep.assign(arr[0])) {
  //        vm.set_error("Invalid end-line meta argument in zs.print.\n");
  //        return err;
  //      }
  //
  //      if (n_template == 2) {
  //        if (auto err = endl.assign(arr[1])) {
  //          vm.set_error("Invalid end-line meta argument in zs.print.\n");
  //          return err;
  //        }
  //      }
  //    }
  //
  //    if (it < params.end()) {
  //      stream << zs::streamer<zs::serializer_type::plain>(*it++);
  //
  //      while (it < params.end()) {
  //        stream << sep << zs::streamer<zs ::serializer_type::plain>(*it++);
  //      }
  //    }
  //
  //    stream << endl;
  //
  //    return {};
  //  }

  zs::error_result zlang_print_internal(zs::vm_ref vm, zs::parameter_list params, std::ostream& stream,
      std::string_view in_sep, std::string_view in_endl) {

    zs::engine* eng = vm.get_engine();
    tmp_string_view sep(eng, in_sep);
    tmp_string_view endl(eng, in_endl);

    if (!params.has_arguments()) {
      stream << endl;
      return {};
    }

    if (params.has_meta_arguments()) {
      auto arr = params.get_meta_arguments();
      const int_t n_template = arr.size();

      if (n_template <= 0 or n_template > 2) {
        vm.set_error("Invalid meta arguments count in zs.print.\n");
        return zs::error_code::invalid;
      }

      if (auto err = sep.assign(arr[0])) {
        vm.set_error("Invalid end-line meta argument in zs.print.\n");
        return err;
      }

      if (n_template == 2) {
        if (auto err = endl.assign(arr[1])) {
          vm.set_error("Invalid end-line meta argument in zs.print.\n");
          return err;
        }
      }
    }

    if (auto args = params.get_normal_arguments(); !args.empty()) {
      auto it = args.begin();
      stream << zs::streamer<zs::serializer_type::plain>(*it);

      while (++it < args.end()) {
        stream << sep << zs::streamer<zs ::serializer_type::plain>(*it);
      }
    }

    stream << endl;

    return {};
  }

} // namespace.

zs::error_result include_lang_lib(zs::virtual_machine* vm) {
  zs::engine* eng = vm->get_engine();

  zs::table_object& root = vm->get_root().as_table();
  root.reserve(30);

  // array_type.
#define _X(name, str, type) root[str] = native_array_type::name;
  ZS_NATIVE_ARRAY_TYPE_ENUM(_X)
#undef _X

  root[_ss("import")] = _nf(zlang_import_impl);
  root[_ss("apply")] = _nf(zlang_apply_impl);

  root[_ss("set_delegate")] = _nf(zlang_setdelegate_impl);

  root[_ss("print")] = [](zs::vm_ref vm, parameter_list params) -> var {
    return zlang_print_internal(vm, params, vm, " ", "\n");
  };

  root[_ss("dprint")] = _nf(zlang_dprint_impl);
  root[_ss("now")] = _nf(zlang_now_impl);
  root[_ss("tostring")] = _nf(zlang_tostring_impl);
  root[_ss("toint")] = _nf(zlang_toint_impl);
  root[_ss("tofloat")] = _nf(zlang_tofloat_impl);
  root[_ss("optget")] = _nf(zlang_optget_impl);
  root[_ss("create_array")] = _nf(zlang_create_array_impl);
  root["mutable_string"] = zlang_create_mutable_string_impl;
  root["create_struct"] = zlang_create_struct_impl;

  zs::object string_table_obj = zs::_t(eng);
  zs::table_object& string_tbl = string_table_obj.as_table();
  string_tbl[_ss("size")] = _nf(
      [](zs::vm_ref vm) -> int_t { return vm.push(zb::unicode::length(vm[1].get_string_unchecked())); });

  root[_ss("string")] = std::move(string_table_obj);

  root["stable"] = zs::_nf([](zs::vm_ref vm) {
    return vm.push(zs::object::create_table_with_delegate(vm.get_engine(),
        zs::_t(vm, { { zs::constants::get<zs::meta_method::mt_get>(), zs::_nf([](zs::vm_ref vm) {
                        return vm.push(vm[0].as_table().emplace(vm[1], zs::_t(vm)).first->second);
                      }) } })));
  });

  {

    zs::object na_module = zs::_t(eng);
    zs::table_object& na_module_tbl = na_module.as_table();
    na_module_tbl.reserve(8);
#define _X(name, str, type) { zs::_ss(str), (int_t)native_array_type::name },
    na_module_tbl["type"] = zs::_t(eng, { ZS_NATIVE_ARRAY_TYPE_ENUM(_X) });
#undef _X

    //    na_module_tbl["type"] = zs::_t(eng,
    //        {
    //            { zs::_ss("u8"), (int_t)native_array_type::n_uint8_t }, //
    //            { zs::_ss("i8"), (int_t)native_array_type::n_int8_t }, //
    //            { zs::_ss("u16"), (int_t)native_array_type::n_uint16_t }, //
    //            { zs::_ss("i16"), (int_t)native_array_type::n_int16_t }, //
    //            { zs::_ss("u32"), (int_t)native_array_type::n_uint32_t }, //
    //            { zs::_ss("i32"), (int_t)native_array_type::n_int32_t }, //
    //            { zs::_ss("u64"), (int_t)native_array_type::n_uint64_t }, //
    //            { zs::_ss("i64"), (int_t)native_array_type::n_int64_t }, //
    //            { zs::_ss("f32"), (int_t)native_array_type::n_float }, //
    //            { zs::_ss("f64"), (int_t)native_array_type::n_double } //
    //        });

    if (auto err = vm->make_enum_table(na_module_tbl["type"])) {
      return err;
    }

    na_module_tbl[_ss("create_array")] = +[](zs::vm_ref vm) -> int_t {
      if (vm.stack_size() == 3) {
        native_array_type ntype = (native_array_type)vm[2]._int;
        zs::object obj = zs::object::create_native_array(vm.get_engine(), ntype, vm[1]._int);
        return vm.push(obj);
      }

      zs::object obj = zs::object::create_native_array<int>(vm.get_engine(), vm[1]._int);

      return vm.push(obj);
    };

    vm->get_imported_module_cache().as_table()["na"] = na_module;
  }

  return {};
}
} // namespace zs.
