#include "vm_utils/zvm_module.h"
#include "zvirtual_machine.h"
#include "objects/zfunction_prototype.h"
#include "lang/zpreprocessor.h"

namespace zs {

zs::error_result try_import_module_from_cache(
    zs::vm_ref vm, const zs::object& name, zs::object& output_module) {
  zs::engine* eng = vm.get_engine();
  zs::table_object& modules = vm->get_imported_modules().as_table();

  if (auto it = modules.find(name); it != modules.end()) {
    output_module = it->second;
    return {};
  }

  const zs::table_object& loaders = vm->get_module_loaders().as_table();
  if (auto it = loaders.find(name); it != loaders.end()) {
    const object& loader = it->second;

    if (!loader.is_function()) {
      return zs::errc::invalid_type;
    }

    object env = vm->create_this_table_from_root();
    if (auto err = vm->call(loader, env, output_module)) {
      return err;
    }

    output_module.as_closure().set_env(std::move(env));
    modules.emplace(name, output_module);
    return {};
  }

  return zs::errc::not_found;
}

zs::error_result import_module(zs::vm_ref vm, const zs::object& name, zs::object& output_module) {

  zs::engine* eng = vm.get_engine();

  if (zs::status_result status = try_import_module_from_cache(vm, name, output_module)) {
    return {};
  }
  else if (status != zs::errc::not_found) {
    return status;
  }

  object res_file_name;
  if (auto err = eng->resolve_file_path(name.get_string_unchecked(), res_file_name)) {
    return zs::errc::invalid_include_file;
  }

  if (zs::status_result status = try_import_module_from_cache(vm, res_file_name, output_module)) {
    return {};
  }
  else if (status != zs::errc::not_found) {
    return status;
  }

  object module_closure;
  if (auto err = compile_or_load_file(vm, res_file_name, module_closure)) {
    return err;
  }

  object env = vm->create_this_table_from_root();

  if (auto err = vm->call(module_closure, env, output_module)) {
    return err;
  }

  module_closure.as_closure().set_env(std::move(env));

  vm->get_imported_modules().as_table().emplace(res_file_name, output_module);
  return {};
}

zs::error_result compile_or_load_buffer(
    zs::vm_ref vm, zb::byte_view content, const object& filename, object& output_closure) {
  zs::engine* eng = vm.get_engine();
  if (zs::function_prototype_object::is_compiled_data(content)) {
    object fpo(zs::function_prototype_object::create(eng), false);

    if (auto err = fpo.as_proto().load(content)) {
      return err;
    }

    output_closure = zs::_c(eng, std::move(fpo), vm->get_root());

    return {};
  }

  object code_output;
  {
    zs::preprocessor preproc(eng);

    if (auto err = preproc.preprocess(std::string_view((const char*)content.data(), content.size()),
            filename.get_string_unchecked(), code_output, vm.get_virtual_machine())) {
      vm->set_error(preproc.get_error());
      return err;
    }
  }

  if (auto err = vm->compile_buffer(code_output.get_string_unchecked(), filename, output_closure, false)) {
    return err;
  }

  if (!output_closure.is_closure()) {
    return zs::errc::invalid_include_file;
  }

  return {};
}

zs::error_result compile_or_load_buffer(
    zs::vm_ref vm, zb::byte_view content, std::string_view filename, object& output_closure) {
  zs::engine* eng = vm.get_engine();
  if (zs::function_prototype_object::is_compiled_data(content)) {
    object fpo(zs::function_prototype_object::create(eng), false);

    if (auto err = fpo.as_proto().load(content)) {
      return err;
    }

    output_closure = zs::_c(eng, std::move(fpo), vm->get_root());

    return {};
  }

  object code_output;
  {
    zs::preprocessor preproc(eng);

    if (auto err = preproc.preprocess(std::string_view((const char*)content.data(), content.size()), filename,
            code_output, vm.get_virtual_machine())) {
      vm->set_error(preproc.get_error());
      return err;
    }
  }

  if (auto err = vm->compile_buffer(code_output.get_string_unchecked(), filename, output_closure, false)) {
    return err;
  }

  if (!output_closure.is_closure()) {
    return zs::errc::invalid_include_file;
  }

  return {};
}

zs::error_result compile_or_load_buffer(
    zs::vm_ref vm, zb::byte_view content, const char* filename, object& output_closure) {
  return compile_or_load_buffer(vm, content, std::string_view(filename), output_closure);
}

zs::error_result compile_or_load_file(zs::vm_ref vm, const char* filename, object& output_closure) {
  zs::engine* eng = vm.get_engine();

  zs::file_loader loader(eng);
  if (auto err = loader.open(filename)) {
    return err;
  }

  return compile_or_load_buffer(vm, loader.data(), filename, output_closure);
}

zs::error_result compile_or_load_file(zs::vm_ref vm, const object& filename, object& output_closure) {
  zs::engine* eng = vm.get_engine();

  zs::file_loader loader(eng);
  if (auto err = loader.open(filename)) {
    return err;
  }

  return compile_or_load_buffer(vm, loader.data(), filename.get_string_unchecked(), output_closure);
}

zs::error_result compile_or_load_file(zs::vm_ref vm, std::string_view filename, object& output_closure) {
  zs::engine* eng = vm.get_engine();

  zs::file_loader loader(eng);
  if (auto err = loader.open(filename)) {
    return err;
  }

  return compile_or_load_buffer(vm, loader.data(), filename, output_closure);
}

} // namespace zs.
