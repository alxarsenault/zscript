#include "utility/zvm_load.h"
#include "zvirtual_machine.h"
#include "object/zfunction_prototype.h"
#include "jit/zjit_compiler.h"
#include "utility/json/zjson_lexer.h"
#include "utility/json/zjson_parser.h"

namespace zs {

zs::error_result load_buffer_as_array(zs::vm_ref vm, std::string_view content, std::string_view source_name,
    zs::object& value, std::string_view sep) {
  return zs::error_code::unimplemented;
}

zs::error_result load_buffer_as_value(
    zs::vm_ref vm, std::string_view content, std::string_view source_name, zs::object& value) {

  zs::jit_compiler compiler(vm.get_engine());
  zs::object fct_state;

  zs::token_type prepended_token = token_type::tok_return;
  if (auto err
      = compiler.compile(content, source_name, fct_state, vm.get_virtual_machine(), &prepended_token)) {
    vm->set_error(compiler.get_error());
    return err;
  }

  zs::object closure_result = zs::object::create_closure(vm.get_engine(), fct_state, vm->global());

  if (!closure_result.is_closure()) {
    return zs::error_code::invalid;
  }

  zs::int_t n_params = 1;
  vm->push_global();

  // Top index = 3
  // N param = 1

  if (auto err = vm->call(closure_result, n_params, vm.stack_size() - n_params, value)) {
    return err;
  }

  return {};
}

zs::error_result load_file_as_value(
    zs::vm_ref vm, const char* filepath, std::string_view source_name, zs::object& value) {

  zs::file_loader loader(vm.get_engine());

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return zs::load_buffer_as_value(vm, loader.content(), source_name, value);
}

zs::error_result load_file_as_array(zs::vm_ref vm, const char* filepath, std::string_view source_name,
    zs::object& value, std::string_view sep) {

  zs::file_loader loader(vm.get_engine());

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return zs::load_buffer_as_array(vm, loader.content(), source_name, value, sep);
}

zs::error_result load_json_table(
    zs::vm_ref vm, std::string_view content, const object& table, object& output) {
  zs::json_parser parser(vm.get_engine());

  if (auto err = parser.parse(vm.get_virtual_machine(), content, table, output)) {
    return err;
  }
  return {};
}

//
// MARK: Json.
//

zs::error_result load_json_file(zs::vm_ref vm, const char* filepath, const object& table, object& output) {

  zs::file_loader loader(vm.get_engine());

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return zs::load_json_table(vm, loader.content(), table, output);
}

zs::error_result load_json_file(
    zs::vm_ref vm, std::string_view filepath, const object& table, object& output) {

  zs::file_loader loader(vm.get_engine());

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return zs::load_json_table(vm, loader.content(), table, output);
}
} // namespace zs.
