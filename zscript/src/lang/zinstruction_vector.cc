namespace zs {

void instruction_vector::serialize(std::ostream& stream) const {
  zb::stream_print<" ">(stream, "-----------", _data.size(), "\n");
  for (auto it = begin(); it != end(); ++it) {

    switch (it.get_opcode()) {
#define ZS_DECL_OPCODE(name)                                                                                 \
  case zs::opcode::op_##name:                                                                                \
    zb::stream_print<" ">(stream, "[", it.get_index(*this), "]", it.get_ref<zs::opcode::op_##name>(), "\n"); \
    break;

#include "lang/zopcode_def.h"
#undef ZS_DECL_OPCODE

    default:
      zb::stream_print<" ">(stream, it.get_opcode(), "\n");
    }
  }
}
} // namespace zs.
