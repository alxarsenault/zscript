
namespace zs {

namespace {
  int_t zfs_create_path(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();
    if (nargs != 2) {
      return -1;
    }

    std::string_view path_view;
    const zs::object& str_path = vm->top();

    // Create from string.
    if (str_path.is_string()) {
      path_view = str_path.get_string_unchecked();
    }

    // Create from fs.path.
    else if (str_path.is_user_data() and str_path._udata->get_uid() == path_library::k_path_uid) {
      path_view = str_path._udata->data_ref<zs::string>();
    }
    else {
      return -1;
    }

    vm->push(zs::path_library::create_path(vm, path_view));
    return 1;
  }

  /// fs.file(filepath, openmode = fs.openmode.read | fs.openmode.write);
  int_t zfs_create_file(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    if (nargs < 2) {
      vm.set_error("Invalid number of arguments in fs.file(filepath, openmode)");
      return -1;
    }

    const zs::object& filepath = vm[1];

    std::string_view path_view;

    // Create from string.
    if (filepath.is_string()) {
      path_view = filepath.get_string_unchecked();
    }

    // Create from fs.path.
    else if (filepath.is_user_data() and filepath._udata->get_uid() == path_library::k_path_uid) {
      path_view = filepath._udata->data_ref<zs::string>();
    }
    else {
      vm.set_error("Invalid path argument in fs.file(filepath, openmode)");
      return -1;
    }

    int_t openmode = file_library::open_mode_read | file_library::open_mode_write;
    if (nargs == 3) {
      const zs::object& openmode_obj = vm[2];

      if (!openmode_obj.is_integer()) {
        vm.set_error("Invalid openmode argument in fs.file(filepath, openmode)");
        return -1;
      }

      openmode = openmode_obj._int;
    }

    vm->push(zs::file_library::create_file(vm, path_view, openmode));
    return 1;
  }

  /**
   * Determines the current process working directory and returns
   * it as an absolute path. Contains a leading drive letter on
   * Windows.
   */
  static int_t zfs_pwd(zs::vm_ref vm) {
#if ZS_UNIX
    char cwd[PATH_MAX];
    if (::getcwd(cwd, PATH_MAX)) {

      vm.push_string(cwd);
      return 1;
    }
    else {
      return -1;
    }
//    throw(std::runtime_error("Failed to retrieve current working
//    directory."));
#elif defined(_WIN32)
    wchar_t cwd[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, cwd) == 0)
      throw(std::runtime_error("Failed to retrieve current working directory."));
    else
      return Path(utf16_to_utf8(std::wstring(cwd)));
#else
#error Unsupported platform.
#endif
  }

  /**
   * \note On Linux, this method accesses the `/proc` filesystem.
   *
   * This method returns the full absolute path to the currently running
   * executable.
   */
  static int_t zfs_exe_path(zs::vm_ref vm) {
#if defined(__linux__)
    char buf[PATH_MAX];
    ssize_t size = ::readlink("/proc/self/exe", buf, PATH_MAX);

    if (size < 0)
      throw(Pathie::ErrnoError(errno));

    return Path(filename_to_utf8(std::string(buf, size)));
#elif ZS_APPLE
    char buf[MAXPATHLEN] = {};
    uint32_t size = MAXPATHLEN;

    if (_NSGetExecutablePath(buf, &size) == 0) {
      // Might contain symlinks or extra slashes. Shouldn't be an issue though
      // https://stackoverflow.com/questions/799679/programmatically-retrieving-the-absolute-path-of-an-os-x-command-line-app/1024933#1024933
      vm.push_string(buf);
      return 1;
    }
    else {
      return -1;
    }
#elif ZS_BSD
    // BSD does not have /proc mounted by default. However, using raw syscalls,
    // we can figure out what would have been in /proc/curproc/file. See
    // sysctl(3) for the management info base identifiers that are used here.
    int mib[4];
    char buf[PATH_MAX];
    size_t bufsize = PATH_MAX;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1; // According to sysctl(3), -1 means the current process.

    if (sysctl(mib, 4, buf, &bufsize, NULL, 0)
        != 0) // Note this changes `bufsize' to the number of chars copied
      throw(Pathie::ErrnoError(errno));

    return Path(filename_to_utf8(std::string(buf, bufsize - 1))); // Exclude terminating NUL
#elif defined(_WIN32)
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(NULL, buf, MAX_PATH) == 0) {
      DWORD err = GetLastError();
      throw(Pathie::WindowsError(err));
    }

    std::string str = utf16_to_utf8(buf);
    return Path(str);
#else
#error Unsupported platform.
#endif
  }

  /**
   * This method returns the current user’s home directory. On UNIX
   * systems, the $HOME environment variable is consulted, whereas
   * on Windows the Windows API is queried for the directory.
   *
   * It will throw std::runtime_error if $HOME is not defined on
   * UNIX.
   */
  static int_t zfs_home_path(zs::vm_ref vm) {
#if ZS_UNIX
    char* homedir = ::getenv("HOME");
    if (!homedir) {
      return -1;
    }

    vm.push_string(homedir);
    return 1;
#elif defined(_WIN32)
    /* TODO: Switch to KNOWNFOLDERID system as explained
     * on
     *http://msdn.microsoft.com/en-us/library/windows/desktop/bb762494%28v=vs.85%29.aspx
     * and
     *http://msdn.microsoft.com/en-us/library/windows/desktop/bb762181%28v=vs.85%29.aspx
     *. Howevever, MinGW does currently (September 2014) not have
     * the new KNOWNFOLDERID declarations.
     */

    wchar_t homedir[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, homedir) != S_OK)
      throw(std::runtime_error("Home directory not defined."));

    return Path(utf16_to_utf8(homedir));
#else
#error Unsupported system.
#endif
  }

  int_t zfs_load_json_file(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    const object& path = vm[1];

    std::string_view path_str;

    if (path.is_string()) {
      path_str = path.get_string_unchecked();
    }
    else if (path.is_user_data() and path._udata->get_uid() == path_library::k_path_uid) {
      path_str = path._udata->data<zs::string>()->c_str();
    }
    else {
      return -1;
    }

    if (nargs == 2) {
      object output;

      if (auto err = vm->load_json_file(path_str, output)) {
        vm->set_error("Could not load json file.\n");
        return -1;
      }

      return vm.push(std::move(output));
    }
    else if (nargs == 3) {
      object output;
      const object& table = vm[2];
      if (auto err = vm->load_json_file(path_str, table, output)) {
        vm->set_error("Could not load json file.\n");
        return -1;
      }

      return vm.push(std::move(output));
    }
    //

    return -1;
  }

  int_t zfs_load_string_file(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    const object& path = vm[1];

    std::string_view path_str;

    if (path.is_string()) {
      path_str = path.get_string_unchecked();
    }
    else if (path.is_user_data() and path._udata->get_uid() == path_library::k_path_uid) {
      path_str = path._udata->data<zs::string>()->c_str();
    }
    else {
      return -1;
    }

    if (nargs == 2) {
      object output;

      zs::file_loader fv(vm.get_engine());
      if (auto err = fv.open(path_str)) {
        return -1;
      }

      return vm.push(zs::_s(vm.get_engine(), fv.content()));
    }
    //

    return -1;
  }
} // namespace.

zs::object create_fs_lib(zs::virtual_machine* vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object fs_module = zs::object::create_table(eng);
  zs::object_unordered_map<object>& fs_map = fs_module._table->get_map();
  fs_map.reserve(20);

  // fs.openmode.
  fs_map["mode"_ss] = zs::object::create_table(vm->get_engine(),
      {
          { "append"_ss, file_library::open_mode_append }, //
          { "end"_ss, file_library::open_mode_end }, //
          { "binary"_ss, file_library::open_mode_binary }, //
          { "read"_ss, file_library::open_mode_read }, //
          { "write"_ss, file_library::open_mode_write }, //
          { "create"_ss, file_library::open_mode_trunc }, //
          { "clear"_ss, file_library::open_mode_trunc }, //
          { "a"_ss, file_library::open_mode_append }, //
          { "r"_ss, file_library::open_mode_read }, //
          { "w"_ss, file_library::open_mode_write }, //
          { "rw"_ss, file_library::open_mode_read | file_library::open_mode_write }, //
      });

  fs_map["path"_ss] = zs::_nc(eng, zfs_create_path);
  fs_map["file"_ss] = zs::_nc(eng, zfs_create_file);
  fs_map["pwd"_ss] = zs::_nc(eng, zfs_pwd);
  fs_map["exe_path"_ss] = zs::_nc(eng, zfs_exe_path);
  fs_map["home"_ss] = zs::_nc(eng, zfs_home_path);

  fs_map["json_file"] = zs::_nf(zfs_load_json_file);
  fs_map["string_file"] = zs::_nf(zfs_load_string_file);
  return fs_module;
}
} // namespace zs.