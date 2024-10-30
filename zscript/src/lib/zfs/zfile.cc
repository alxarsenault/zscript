
namespace zs::file_library {

zfile::zfile(zs::engine* eng, std::string_view filepath, int_t openmode)
    : path(filepath, zs::allocator<char>(eng)) {

  std::ios::openmode omode = {};
  omode |= (openmode & open_mode_append) ? std::ios::app : 0;
  omode |= (openmode & open_mode_end) ? std::ios::ate : 0;
  omode |= (openmode & open_mode_binary) ? std::ios::binary : 0;
  omode |= (openmode & open_mode_read) ? std::ios::in : 0;
  omode |= (openmode & open_mode_write) ? std::ios::out : 0;
  omode |= (openmode & open_mode_trunc) ? std::ios::trunc : 0;
  stream.open(path.c_str(), omode);
}

zfile* get_file(zs::vm_ref vm) {
  const zs::object& file_obj = vm[0];
  if (!file_obj.is_user_data()) {
    return nullptr;
  }

  return file_obj._udata->data<zfile>();
}

#define ZS_GET_FILE()                       \
  get_file(vm);                             \
  if (!file) {                              \
    vm.set_error("Invalid fs.file object"); \
    return -1;                              \
  }

static int_t file_get_path_impl(zs::vm_ref vm) {
  zfile* file = ZS_GET_FILE();
  vm.push_string(file->path);
  return 1;
}

static int_t file_is_open_impl(zs::vm_ref vm) {
  zfile* file = ZS_GET_FILE();
  vm.push_bool(file->stream.is_open());
  return 1;
}

static int_t file_close_impl(zs::vm_ref vm) {
  zfile* file = ZS_GET_FILE();
  file->stream.close();
  return 0;
}

static int_t file_write_impl(zs::vm_ref vm) {
  const int_t nargs = vm.stack_size();
  if (nargs == 1) {
    vm.set_error("No args in fs.file.write()");
    return -1;
  }

  zfile* file = ZS_GET_FILE();

  if (!file->stream.is_open()) {
    vm.set_error("File is not open in fs.file.write()");
    return -1;
  }

  for (int_t i = 1; i < nargs; i++) {
    const object& obj = vm[i];

    obj.stream(file->stream);
  }

  //  vm.push_bool(true);
  return vm.push(vm[0]);
}

static int_t file_write_json_impl(zs::vm_ref vm) {
  const int_t nargs = vm.stack_size();
  if (nargs == 1) {
    vm.set_error("No args in fs.file.write()");
    return -1;
  }

  zfile* file = ZS_GET_FILE();

  if (!file->stream.is_open()) {
    vm.set_error("File is not open in fs.file.write()");
    return -1;
  }

  for (int_t i = 1; i < nargs; i++) {
    const object& obj = vm[i];
    obj.stream_to_json(file->stream);
  }
  return vm.push(vm[0]);
}

static int_t file_read_impl(zs::vm_ref vm) {
  const int_t nargs = vm.stack_size();
  if (nargs != 1) {
    vm.set_error("No args in fs.file.read()");
    return -1;
  }

  zfile* file = ZS_GET_FILE();

  if (!file->stream.is_open()) {
    vm.set_error("File is not open in fs.file.file()");
    return -1;
  }

  std::string value;
  file->stream >> value;

  vm.push_string(value);
  return 1;
}

static int_t file_get_impl(zs::vm_ref vm) {
  // vm[0] should be the user_data.
  // vm[1] should be the key.

  if (vm.stack_size() != 2) {
    return -1;
  }

  zfile* file = ZS_GET_FILE();
  const object& key = vm[1];

  if (key == "path") {
    vm.push_string(file->path);
    return 1;
  }

  return -1;
}

static void set_file_delegate_methods(zs::vm_ref vm, zs::object& file_delegate,
    const std::initializer_list<std::pair<zs::object, zs::native_cpp_closure_t>>& list) {
  zs::engine* eng = vm->get_engine();
  zs::table_object* tbl = file_delegate._table;

  for (auto& n : list) {
    tbl->set(std::move(n.first), zs::_nc(eng, n.second));
  }
}

static zs::object create_file_delegate(zs::vm_ref vm) {
  zs::engine* eng = vm->get_engine();

  zs::object delegate_key = zs::_sv(k_file_delegate_name);

  zs::object_unordered_map<zs::object>& registry_map = eng->get_registry_table()._table->get_map();
  if (auto it = registry_map.find(delegate_key); it != registry_map.end()) {
    return it->second;
  }

  zs::object file_delegate = zs::object::create_table(eng);

  set_file_delegate_methods(vm, file_delegate,
      {
          { zs::_ss("get_path"), file_get_path_impl }, //
          { zs::_ss("is_open"), file_is_open_impl }, //
          //          { zs::_ss("open"), file_open_impl }, //
          { zs::_ss("close"), file_close_impl }, //
          { zs::_ss("write"), file_write_impl }, //
          { zs::_ss("write_json"), file_write_json_impl }, //
          { zs::_ss("read"), file_read_impl }, //
          { zs::constants::get<meta_method::mt_get>(), file_get_impl }, //

      });

  return (registry_map[delegate_key] = std::move(file_delegate));
}

zs::object create_file(zs::vm_ref vm, std::string_view path, int_t openmode) {
  zs::engine* eng = vm.get_engine();

  zs::object file_obj = zs::object::create_user_data(eng, sizeof(zfile));
  zb_placement_new(file_obj._udata->data()) zfile(eng, path, openmode);

  file_obj.set_user_data_release_hook(
      [](zs::engine* eng, zs::raw_pointer_t ptr) { ((zfile*)ptr)->~zfile(); });

  file_obj._udata->set_to_string_callback(
      [](const object_base& obj, std::ostream& stream) -> zs::error_result {
        stream << zb::quoted(obj._udata->data_ref<zfile>().path);
        return {};
      });

  file_obj.set_delegate(create_file_delegate(vm));
  file_obj._udata->set_uid(zs::_sv(k_file_uid));

  return file_obj;
}
} // namespace zs::file_library.
