#include <zapp.h>

#include <zscript.h>
#include "zvirtual_machine.h"
#include "lang/zpreprocessor.h"
#include "objects/zfunction_prototype.h"

namespace {
struct options {
  std::filesystem::path filepath;
  std::string code;
  std::string main_function;
  std::vector<std::string> args;
  std::vector<std::string> include_directories;
  std::vector<std::pair<std::string, std::string>> defines;
};

options from_zapp_options(const zapp_options* opts) {
  options ops;

  if (opts->filepath) {
    ops.filepath = opts->filepath;
  }

  if (opts->main_function) {
    ops.main_function = opts->main_function;
  }

  if (opts->code) {
    ops.code = opts->code;
  }

  for (int i = 0; i < opts->args.size; i++) {
    ops.args.push_back(opts->args.items[i]);
  }

  for (int i = 0; i < opts->include_directories.size; i++) {
    ops.include_directories.push_back(opts->include_directories.items[i]);
  }

  for (int i = 0; i < opts->defines.size; i++) {
    ops.defines.push_back({ opts->defines.items[i].name, opts->defines.items[i].value });
  }

  return ops;
}

zs::error_result compile_file(zs::vm_ref vm, const options& args, zs::object& closure) {
  zs::engine* eng = vm.get_engine();

  if (!args.include_directories.empty()) {
    for (const auto& dir : args.include_directories) {
      eng->add_import_directory(dir);
    }
  }

  if (!args.defines.empty()) {
    for (const auto& def : args.defines) {
      vm->get_root().as_table()[def.first] = zs::_s(vm, def.second);
    }
  }

  std::string code = args.code;
  std::string filename = args.filepath.c_str();

  if (code.empty() and filename.empty()) {
    zb::stream_print(std::cerr, "No filename or code was provided.\n");
    return zs::errc::invalid_argument;
  }

  if (!args.code.empty()) {
    code = args.code;

    if (filename.empty()) {
      filename = "unnamed";
    }
  }
  else {
    zs::file_loader loader(eng);

    if (auto err = loader.open(filename)) {
      zb::stream_print(std::cerr, "Could not open file '", filename, "'.\n");
      return err;
    }

    code = loader.content();
  }

  {
    zs::object code_output;
    zs::preprocessor preproc(eng);

    if (auto err = preproc.preprocess(code, filename, code_output, vm.get_virtual_machine())) {
      zb::stream_print(std::cerr, "Preprocessor error: ", preproc.get_error(), "\n");
      return err;
    }

    if (!code_output.is_string()) {
      zb::stream_print(std::cerr, "Preprocessor failed\n");
      return zs::errc::invalid_type;
    }

    code = code_output.get_string_unchecked();
  }

  if (auto err = vm->compile_buffer(code, filename, closure, true)) {
    zb::stream_print(std::cerr, "Compiler error: ", vm.get_error(), "\n");
    return err;
  }

  if (!closure.is_closure()) {
    zb::stream_print(std::cerr, "Invalid compile result type.\n");
    return zs::errc::invalid_type;
  }

  return {};
}

zs::error_result run_file(
    zs::vm_ref vm, const options& args, const zs::object& closure, zs::object& result_value) {
  zs::engine* eng = vm.get_engine();

  zs::object args_array = zs::_a(eng, args.args);

  zs::object env = vm->create_this_table_from_root();

  // Call the compiled file.
  if (auto err = vm->call(closure, { env, args_array }, result_value)) {
    zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), "\n");
    return err;
  }

  closure.as_closure().set_env(std::move(env));

  return {};
}

zs::error_result call_main(
    zs::vm_ref vm, const options& args, const zs::object& module_obj, zs::object& result_value) {
  zs::engine* eng = vm.get_engine();

  zs::object main_closure;

  if (auto err = vm->get(module_obj, zs::_s(eng, args.main_function), main_closure)) {
    zb::stream_print(std::cerr, "Could not find '", args.main_function, "' function.\n");
    return err;
  }

  if (!main_closure.is_closure()) {
    zb::stream_print(std::cerr, "The '", args.main_function, "' function value is not a closure.\n");
    return zs::errc::invalid_type;
  }

  size_t nargs = main_closure.as_closure().get_proto()._parameter_names.size();
  size_t ndefaults = main_closure.as_closure().get_proto()._default_params.size();
  size_t n_min_args = nargs - ndefaults;

  if (nargs == 1 and ndefaults == 0) {
    if (auto err = vm->call(main_closure, vm->get_root(), result_value)) {
      zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), ".\n");
      return err;
    }
  }
  else if (nargs > 1 and n_min_args <= 2) {
    zs::object args_array = zs::_a(eng, args.args);

    if (auto err = vm->call(main_closure, { vm->get_root(), args_array }, result_value)) {
      zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), ".\n");
      return err;
    }
  }
  else {
    zb::stream_print(std::cerr, "Invalid parameter count for calling '", args.main_function, "' function.\n");
    return zs::errc::invalid_parameter_count;
  }

  return {};
}
} // namespace

extern "C" {
int zapp_call_main(const zapp_options* opts) {
  options args = from_zapp_options(opts);

  zs::vm vm;

  zs::object closure;
  if (auto err = compile_file(vm, args, closure)) {
    return -1;
  }

  zs::object module_obj;
  if (auto err = run_file(vm, args, closure, module_obj)) {
    return -1;
  }

  if (args.main_function.empty()) {

    if (module_obj.is_integer()) {
      return (int)module_obj._int;
    }

    return 0;
  }

  zs::object result_value;
  if (auto err = call_main(vm, args, module_obj, result_value)) {
    return -1;
  }

  if (result_value.is_integer()) {
    return (int)result_value._int;
  }
  return 0;
}
} // extern "C".
