
#include <zscript/zscript.h>
#include "zvirtual_machine.h"
//#include "zpreprocessor.h"
#include "object/zfunction_prototype.h"

zs::error_result compile_file(zs::vm_ref vm, const char* filepath, zs::object& closure) {
  zs::engine* eng = vm.get_engine();

  std::string code;
  zs::file_loader loader(eng);

  if (auto err = loader.open(filepath)) {
    zb::stream_print(std::cerr, "Could not open file '", filepath, "'.\n");
    return err;
  }

  code = loader.content();

//  {
//    zs::object code_output;
//    zs::preprocessor preproc(eng);
//
//    if (auto err = preproc.preprocess(code, filepath, code_output)) {
//      zb::stream_print(std::cerr, "Preprocessor error: ", preproc.get_error(), "\n");
//      return err;
//    }
//
//    if (!code_output.is_string()) {
//      zb::stream_print(std::cerr, "Preprocessor failed\n");
//      return zs::errc::invalid_type;
//    }
//
//    code = code_output.get_string_unchecked();
//  }

  if (auto err = vm->compile_buffer(code, filepath, closure, true)) {
    zb::stream_print(std::cerr, "Compiler error: ", vm.get_error(), "\n");
    return err;
  }

  if (!closure.is_closure()) {
    zb::stream_print(std::cerr, "Invalid compile result type.\n");
    return zs::errc::invalid_type;
  }

  return {};
}

zs::error_result run_file(zs::vm_ref vm, const zs::object& closure, zs::object& result_value) {
  zs::engine* eng = vm.get_engine();

  //  zs::object args_array = zs::_a(eng, args.args);

  zs::object env = zs::object::create_table_with_delegate(eng, vm->global());

  // Call the compiled file.
  if (auto err = vm->call(closure, env, result_value)) {
    zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), "\n");
    return err;
  }

  closure.as_closure().set_bounded_this(std::move(env));

  return {};
}

zs::error_result call_main(
    zs::vm_ref vm, int argc, char* argv[], const zs::object& module_obj, zs::object& result_value) {
  zs::engine* eng = vm.get_engine();

  zs::object main_closure;

  if (auto err = vm->get(module_obj, zs::_s(eng, "main"), main_closure)) {
    zb::stream_print(std::cerr, "Could not find 'main' function.\n");
    return err;
  }

  if (!main_closure.is_closure()) {
    zb::stream_print(std::cerr, "The 'main' function value is not a closure.\n");
    return zs::errc::invalid_type;
  }

  size_t nargs = main_closure.as_closure().get_proto()._parameter_names.size();
  size_t ndefaults = main_closure.as_closure().get_proto()._default_params.size();
  size_t n_min_args = nargs - ndefaults;

  if (nargs == 1 and ndefaults == 0) {
    if (auto err = vm->call(main_closure, vm->global(), result_value)) {
      zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), ".\n");
      return err;
    }
  }
  else if (nargs > 1 and n_min_args <= 2) {
    zs::object args_array = zs::_a(eng, std::span<const char*>((const char**)argv, argc));

    if (auto err = vm->call(main_closure, { vm->global(), args_array }, result_value)) {
      zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), ".\n");
      return err;
    }
  }
  else {
    zb::stream_print(std::cerr, "Invalid parameter count for calling 'main' function.\n");
    return zs::errc::invalid_parameter_count;
  }

  return {};
}

int main(int argc, char* argv[]) {

  if (argc < 2) {
    zb::stream_print(std::cerr, "Missing filepath.\n");
    return -1;
  }

  zs::vm vm;

  zs::object closure;
  if (auto err = compile_file(vm, argv[1], closure)) {
    return -1;
  }

  zs::object module_obj;
  if (auto err = run_file(vm, closure, module_obj)) {
    return -1;
  }

  zs::object result_value;
  if (auto err = call_main(vm, argc, argv, module_obj, result_value)) {
    return -1;
  }

  if (result_value.is_integer()) {
    return (int)result_value._int;
  }
  return 0;
}
