#include <zscript/zscript.h>
#include <zscript/std/zslib.h>
#include "zvirtual_machine.h"
#include "utility/zvm_module.h"
#include <zbase/strings/charconv.h>
#include <zbase/strings/unicode.h>

#if !defined(ZS_UNIX) \
    && (defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(BSD))
#define ZS_UNIX 1
#else
#deine ZS_UNIX 0
#endif

#if ZS_UNIX
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h> // defines "BSD" macro on BSD systems
#include <pwd.h>
#include <glob.h>
#include <fnmatch.h>
#endif

#ifdef BSD
#define ZS_BSD 1
#include <sys/time.h>
#include <sys/sysctl.h>
#else
#define ZS_BSD 0
#endif

#ifdef __APPLE__
#define ZS_APPLE 1
#include <mach-o/dyld.h>
#include <libkern/OSCacheControl.h>
#else
#define ZS_APPLE 0
#endif

#include <uv.h>

extern char** environ;

namespace zs {

namespace {
  template <class CharType>
  static inline std::basic_string_view<CharType> sv_from_char(const CharType& c) noexcept {
    return std::basic_string_view<CharType>(&c, 1);
  }

  enum class shell_name { bash, sh, zsh };

  const char* get_shell_path(shell_name sh) {
    switch (sh) {
    case shell_name::bash:
      return "/bin/bash";
    case shell_name::zsh:
      return "/bin/zsh";
    case shell_name::sh:
      return "/bin/sh";
    default:
      return "/bin/zsh";
    }
  }

  const char* get_shell_name(shell_name sh) {
    switch (sh) {
    case shell_name::bash:
      return "bash";
    case shell_name::zsh:
      return "zsh";
    case shell_name::sh:
      return "sh";
    default:
      return "zsh";
    }
  }

  // extern char **environ;
  //
  //
  // std::vector<std::string> get_all_env() {
  //   char **s = environ;
  //   std::vector<std::string> vals;
  //   for (; *s; s++) {
  //     vals.push_back(*s);
  //   }
  //
  //   return vals;
  // }

  zs::optional_result<int> run_command(shell_name shname, const char* cmd, std::span<const uint8_t> input,
      zs::vector<uint8_t>* output, zs::vector<uint8_t>* error) {

    const int READ_END = 0;
    const int WRITE_END = 1;

    int infd[2] = { 0, 0 };
    int outfd[2] = { 0, 0 };
    int errfd[2] = { 0, 0 };

    auto cleanup = [&]() {
      ::close(infd[READ_END]);
      ::close(infd[WRITE_END]);

      ::close(outfd[READ_END]);
      ::close(outfd[WRITE_END]);

      ::close(errfd[READ_END]);
      ::close(errfd[WRITE_END]);
    };

    auto rc = ::pipe(infd);
    if (rc < 0) {
      return zs::errc::broken_pipe;
    }

    rc = ::pipe(outfd);
    if (rc < 0) {
      ::close(infd[READ_END]);
      ::close(infd[WRITE_END]);
      return zs::errc::broken_pipe;
    }

    rc = ::pipe(errfd);
    if (rc < 0) {
      ::close(infd[READ_END]);
      ::close(infd[WRITE_END]);

      ::close(outfd[READ_END]);
      ::close(outfd[WRITE_END]);

      return zs::errc::broken_pipe;
    }

    pid_t pid = fork();

    // Parent.
    if (pid > 0) {
      // Parent does not read from stdin.
      ::close(infd[READ_END]);

      // Parent does not write to stdout.
      ::close(outfd[WRITE_END]);

      // Parent does not write to stderr.
      ::close(errfd[WRITE_END]);

      if (!input.empty()) {
        if (::write(infd[WRITE_END], input.data(), input.size()) < 0) {
          return zs::errc::broken_pipe;
        }

        // Done writing.
        ::close(infd[WRITE_END]);
      }
    }

    // Child.
    else if (pid == 0) {
      ::dup2(infd[READ_END], STDIN_FILENO);
      ::dup2(outfd[WRITE_END], STDOUT_FILENO);
      ::dup2(errfd[WRITE_END], STDERR_FILENO);

      // Child does not write to stdin.
      ::close(infd[WRITE_END]);

      // Child does not read from stdout.
      ::close(outfd[READ_END]);

      // Child does not read from stderr.
      ::close(errfd[READ_END]);

      //      std::vector<const char*> env_vec;
      //      env_vec.resize(env.size() + 1);
      //
      //      for (size_t i = 0; i < env.size(); i++) {
      //        env_vec[i] = env[i];
      //      }
      //
      //      env_vec.back() = nullptr;

      const char* shell_path = get_shell_path(shname);
      const char* sname = get_shell_name(shname);

      //     int   execve(const char * __file, char * const * __argv, char * const * __envp)
      //      ::execle(shell_path, sname, "-c", cmd, nullptr, env_vec.data());
      ::execle(shell_path, sname, "-c", cmd, nullptr, environ);

      ::exit(EXIT_SUCCESS);
    }

    // PARENT
    if (pid < 0) {
      cleanup();
      return zs::errc::broken_pipe;
    }

    int p_status = 0;
    ::waitpid(pid, &p_status, 0);

    std::array<uint8_t, 256> buffer;

    ssize_t bytes = 0;

    if (output) {
      do {
        bytes = ::read(outfd[READ_END], buffer.data(), buffer.size());
        output->insert(output->end(), buffer.data(), buffer.data() + bytes);
      } while (bytes > 0);
    }

    if (error) {
      do {
        bytes = ::read(errfd[READ_END], buffer.data(), buffer.size());
        error->insert(error->end(), buffer.data(), buffer.data() + bytes);
      } while (bytes > 0);
    }

    int status = 0;
    if (WIFEXITED(p_status)) {
      status = WEXITSTATUS(p_status);
    }

    cleanup();

    return status;
  }

  int_t zslib_exec_impl(zs::vm_ref vm) {

    const int_t nargs = vm.stack_size();

    if (nargs < 2) {
      vm->handle_error(zs::errc::invalid_parameter_count, { -1, -1 },
          "Missing command parameter in sys::exec().", ZS_DEVELOPER_SOURCE_LOCATION());
      return -1;
    }

    zs::vector<uint8_t> output(zs::allocator<uint8_t>(vm.get_engine()));
    zs::vector<uint8_t> errors(zs::allocator<uint8_t>(vm.get_engine()));

    const object& cmd_obj = vm[1];

    std::span<const uint8_t> input;
    if (nargs >= 3) {
      const object& input_obj = vm[2];
      std::string_view input_str = input_obj.get_string_unchecked();
      input = std::span<const uint8_t>((const uint8_t*)input_str.data(), input_str.size());
    }

    if (!cmd_obj.is_cstring()) {
      vm->handle_error(zs::errc::invalid_type, { -1, -1 }, "Invalid command parameter type in sys::exec().",
          ZS_DEVELOPER_SOURCE_LOCATION());
      return -1;
    }

    const char* cmd = cmd_obj.get_cstring_unchecked();

    zs::optional_result<int> res = run_command(shell_name::zsh, cmd, input, &output, &errors);

    object output_obj = zs::_t(vm.get_engine());
    table_object& tbl = output_obj.as_table();

    tbl.emplace(zs::_ss("stdout"),
        zs::_s(vm.get_engine(),
            std::string_view(
                (const char*)output.data(), output.size() - (!output.empty() and output.back() == '\n'))));

    tbl.emplace(zs::_ss("stderr"),
        zs::_s(vm.get_engine(),
            std::string_view(
                (const char*)errors.data(), errors.size() - (!errors.empty() and errors.back() == '\n'))));

    //    tbl.emplace(zs::_ss("error"), zs::object::create_error(res.error()));
    tbl.emplace(zs::_ss("status"), res.has_value() ? res.value() : -1);

    return vm.push(output_obj);
  }

  int_t zslib_sleep_ms_impl(zs::vm_ref vm) {
    zs::engine* eng = vm.get_engine();

    int_t ms_value = 0;
    if (auto err = vm[1].get_integer(ms_value)) {
      vm->handle_error(zs::errc::invalid_type, { -1, -1 }, "Invalid command parameter type in sys::sleep().",
          ZS_DEVELOPER_SOURCE_LOCATION());

      return -1;
    }

    uv_sleep((unsigned int)ms_value);
    return 0;
  }

  int_t zslib_uname_impl(zs::vm_ref vm) {
    zs::engine* eng = vm.get_engine();
    zs::object info_obj = zs::_t(eng);
    zs::table_object& info_tbl = info_obj.as_table();

    uv_utsname_t uname_info;
    uv_os_uname(&uname_info);
    info_tbl.emplace(zs::_ss("name"), zs::_s(eng, uname_info.sysname));
    info_tbl.emplace(zs::_ss("release"), zs::_s(eng, uname_info.release));
    info_tbl.emplace(zs::_ss("version"), zs::_s(eng, uname_info.version));
    info_tbl.emplace(zs::_ss("machine"), zs::_s(eng, uname_info.machine));
    return vm.push(info_obj);
  }

  int_t zslib_exit_impl(zs::vm_ref vm) {

    int_t nargs = vm.stack_size();
    if (nargs <= 1) {
      ::exit(0);
      return 0;
    }

    object val = vm[1];
    int_t ival = 0;
    if (auto err = val.get_integer(ival)) {
      vm->handle_error(zs::errc::invalid_type, { -1, -1 }, "Invalid exit code parameter type in sys::exit().",
          ZS_DEVELOPER_SOURCE_LOCATION());

      return -1;
    }

    ::exit((int)ival);

    return 0;
  }

  int_t zsys_now_impl(zs::vm_ref vm) {
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    vm.push_integer(now);
    return 1;
  }

} // namespace.

zs::object create_sys_lib(zs::vm_ref vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::table_object* sys_tbl = zs::table_object::create(eng);
  sys_tbl->reserve(16);
  sys_tbl->emplace("exec"_ss, zslib_exec_impl);
  sys_tbl->emplace("uname"_ss, zslib_uname_impl);
  sys_tbl->emplace("sleep_ms"_ss, zslib_sleep_ms_impl);
  sys_tbl->emplace("exit"_ss, zslib_exit_impl);
  sys_tbl->emplace("now"_ss, zsys_now_impl);

  return object(sys_tbl, false);
}
} // namespace zs.
