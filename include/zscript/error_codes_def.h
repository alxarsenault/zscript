
#define ZS_DECL_ERROR_CODE_VALUE(name) ZS_DECL_ERROR_CODE(name, #name)

//
// Valid codes.
//

ZS_DECL_ERROR_CODE_VALUE(success)
ZS_DECL_ERROR_CODE_VALUE(returned)

//
// Generic codes.
//

ZS_DECL_ERROR_CODE_VALUE(unknown)
ZS_DECL_ERROR_CODE_VALUE(invalid)
ZS_DECL_ERROR_CODE_VALUE(unimplemented)
ZS_DECL_ERROR_CODE_VALUE(inaccessible)
ZS_DECL_ERROR_CODE_VALUE(already_exists)
ZS_DECL_ERROR_CODE_VALUE(not_found)
ZS_DECL_ERROR_CODE_VALUE(inaccessible_private)
ZS_DECL_ERROR_CODE_VALUE(invalid_operation)
ZS_DECL_ERROR_CODE_VALUE(invalid_argument)
ZS_DECL_ERROR_CODE_VALUE(invalid_parameter_count)
ZS_DECL_ERROR_CODE_VALUE(invalid_parameters)
ZS_DECL_ERROR_CODE_VALUE(invalid_native_function_call)
ZS_DECL_ERROR_CODE_VALUE(invalid_name)
ZS_DECL_ERROR_CODE_VALUE(broken_pipe)

//
// Types codes.
//

ZS_DECL_ERROR_CODE_VALUE(invalid_type)
ZS_DECL_ERROR_CODE_VALUE(invalid_delegate_type)
ZS_DECL_ERROR_CODE_VALUE(invalid_parameter_type)
ZS_DECL_ERROR_CODE_VALUE(invalid_type_assignment)
ZS_DECL_ERROR_CODE_VALUE(invalid_native_array_type)
ZS_DECL_ERROR_CODE_VALUE(conversion_error)
ZS_DECL_ERROR_CODE_VALUE(not_a_table)
ZS_DECL_ERROR_CODE_VALUE(not_an_array)
ZS_DECL_ERROR_CODE_VALUE(not_a_string)
ZS_DECL_ERROR_CODE_VALUE(not_a_cstring)
ZS_DECL_ERROR_CODE_VALUE(not_a_mutable_string)
ZS_DECL_ERROR_CODE_VALUE(not_a_number)
ZS_DECL_ERROR_CODE_VALUE(not_an_integer)
ZS_DECL_ERROR_CODE_VALUE(not_a_float)
ZS_DECL_ERROR_CODE_VALUE(not_a_bool)
ZS_DECL_ERROR_CODE_VALUE(not_delegable)

//
// Filesystem codes.
//

ZS_DECL_ERROR_CODE_VALUE(open_file_error)
ZS_DECL_ERROR_CODE_VALUE(copy_file_error)
ZS_DECL_ERROR_CODE_VALUE(invalid_directory)
ZS_DECL_ERROR_CODE_VALUE(invalid_include_directory)

//
// Memory codes.
//

ZS_DECL_ERROR_CODE_VALUE(memory_error)
ZS_DECL_ERROR_CODE_VALUE(out_of_memory)
ZS_DECL_ERROR_CODE_VALUE(out_of_bounds)

//
// Compiler codes.
//

ZS_DECL_ERROR_CODE_VALUE(compile_stack_error)
ZS_DECL_ERROR_CODE_VALUE(too_many_locals)
ZS_DECL_ERROR_CODE_VALUE(identifier_expected)
ZS_DECL_ERROR_CODE_VALUE(invalid_include_syntax)
ZS_DECL_ERROR_CODE_VALUE(invalid_include_file)
ZS_DECL_ERROR_CODE_VALUE(invalid_token)
ZS_DECL_ERROR_CODE_VALUE(invalid_number)
ZS_DECL_ERROR_CODE_VALUE(invalid_comma)
ZS_DECL_ERROR_CODE_VALUE(not_a_for_colon)
// ZS_DECL_ERROR_CODE_VALUE(cant_modify_export_table)
ZS_DECL_ERROR_CODE_VALUE(duplicated_parameter_name)
ZS_DECL_ERROR_CODE_VALUE(duplicated_local_variable_name)
ZS_DECL_ERROR_CODE_VALUE(duplicated_module_tag)
ZS_DECL_ERROR_CODE_VALUE(invalid_module_tag)
ZS_DECL_ERROR_CODE_VALUE(invalid_module_info_tag)
ZS_DECL_ERROR_CODE_VALUE(duplicated_default_constructor)
ZS_DECL_ERROR_CODE_VALUE(ambiguous_constructors)
ZS_DECL_ERROR_CODE_VALUE(unexpected_struct_name)
ZS_DECL_ERROR_CODE_VALUE(missing_doc_statement)

//
// Struct codes.
//

ZS_DECL_ERROR_CODE_VALUE(cant_modify_const_member)
ZS_DECL_ERROR_CODE_VALUE(cant_modify_static_const)

//
// Graphics codes.
//

ZS_DECL_ERROR_CODE_VALUE(could_not_create_image)
ZS_DECL_ERROR_CODE_VALUE(invalid_image_type)

#undef ZS_DECL_ERROR_CODE_VALUE
