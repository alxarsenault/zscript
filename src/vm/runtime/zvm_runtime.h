namespace zs {

using objref_t = zb::ref_wrapper<object>;
using cobjref_t = zb::ref_wrapper<const object>;

#define ZS_DECL_RT_ACTION(name, ...) \
  template <>                        \
  zs::error_result virtual_machine::runtime_action<runtime_code::name>(__VA_ARGS__)

ZS_DECL_RT_ACTION(
    handle_error, zs::function_prototype_object* fct, zs::instruction_iterator it, zs::error_code ec);

ZS_DECL_RT_ACTION(delegate_set, objref_t obj, objref_t delegate_obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(table_set, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(table_set_if_exists, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(user_data_set, objref_t user_data_obj, cobjref_t key, cobjref_t value);

//
//
//

ZS_DECL_RT_ACTION(
    handle_error, zs::function_prototype_object* fct, zs::instruction_iterator it, zs::error_code ec) {

  size_t inst_byte_index = it.get_index(fct->_instructions);

  const line_info_op_t* last_linfo = nullptr;
  const line_info_op_t* linfo = nullptr;

  for (const line_info_op_t& line : fct->_line_info) {
    if (line.op_index <= (int_t)inst_byte_index) {
      last_linfo = linfo;
      linfo = &line;

      if (line.op_index == (int_t)inst_byte_index) {
        break;
      }
    }
    else {
      break;
    }
  }

  zs::string err_msg(_engine);
  constexpr std::string_view pre = "\n      ";
  err_msg += zs::sstrprint(_engine, "error: opcode:", it.get_opcode(), //
      pre, "error_code:", zs::error_code_to_string(ec), //
      pre, "closure name:", fct->_name, //
      pre, "closure source name:", fct->_source_name, //
      pre, "instruction index:", it.get_instruction_index(fct->_instructions), //
      pre, "instruction byte index:", inst_byte_index, //
      pre, "stack base index:", _stack.get_stack_base(), //
      pre, "stack size:", _stack.stack_size(), //
      pre, "call stack index:", _call_stack.size(), //
      pre, "call stack previous stack base:", _call_stack.back().previous_stack_base, //
      pre, "call stack previous top index:", _call_stack.back().previous_top_index);

  if (last_linfo) {
    err_msg += zs::strprint(_engine, //
        pre, " line: [ ", last_linfo->line, " : ", last_linfo->column, " ]");
  }

  zs::line_info iline;
  if (linfo) {
    iline.line = linfo->line;
    iline.column = linfo->column;
    err_msg += zs::strprint(_engine, //
        pre, " line end: [ ", linfo->line, " : ", linfo->column, " ]");
  }

  err_msg += "\n";

  _errors.emplace_back(_engine, error_source::virtual_machine, ec, err_msg,
      fct->_source_name.is_string() ? fct->_source_name.get_string_unchecked() : "", "", iline,
      zb::source_location::current());

  return ec;
}

} // namespace zs.

#include "vm/runtime/zvm_runtime_table.h"
#include "vm/runtime/zvm_runtime_user_data.h"
#include "vm/runtime/zvm_runtime_delegate.h"
