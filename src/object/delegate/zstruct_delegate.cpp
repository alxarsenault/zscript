#include "zstruct_delegate.h"
#include "zvirtual_machine.h"

namespace zs {
namespace {

#define ZS_STRUCT_SET_ARG_ERROR(fct_name) vm.set_error("Invalid array argument in array::" fct_name "().")

#define ZS_STRUCT_GET(fct_name)                                       \
  object& obj = vm[0];                                                \
  if (!obj.is_array()) {                                              \
    vm.set_error("Invalid array argument in array::" fct_name "()."); \
    return -1;                                                        \
  }                                                                   \
  array_object& arr = obj.as_array()

#define ZS_STRUCT_BEGIN_IMPL(fct_name, n_args)                             \
  if (vm.stack_size() != n_args) {                                         \
    vm.set_error("Invalid number of arguments in array::" fct_name "()."); \
    return -1;                                                             \
  }                                                                        \
  ZS_STRUCT_GET(fct_name)

} // namespace

zs::object create_struct_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  zs::table_object& t = obj.as_table();
  t.set_no_default_none();
  return obj;
}
} // namespace zs.
