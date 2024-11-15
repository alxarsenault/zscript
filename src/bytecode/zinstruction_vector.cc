namespace zs {

void instruction_vector::serialize(std::ostream& stream) const {
  zb::stream_print<" ">(stream, "-----------", _data.size(), "\n");
  size_t index = 0;

  for (auto it = begin(); it != end(); ++it) {

    switch (it.get_opcode()) {
#define ZS_DECL_OPCODE(name, INST_TYPES)                                                                 \
  case ZS_OPCODE_ENUM_VALUE(name):                                                                       \
    zb::stream_print<" ">(                                                                               \
        stream, index++, "[", it.get_index(*this), "]", it.get_ref<ZS_OPCODE_ENUM_VALUE(name)>(), "\n"); \
    break;

#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE

    default:
      zb::stream_print<" ">(stream, zs::opcode_to_string(it.get_opcode()), "\n");
    }
  }
}

void instruction_vector::debug_print(std::ostream& stream) const {

  size_t count = 0;
  for (auto it = begin(); it != end(); ++it) {
    count++;
  }

  zb::stream_print(stream, "size: ", _data.size(), "\ncount: ", count, "\n");

  const int index_padding = count >= 100 ? 3 : 2;
  const int offset_padding = _data.size() >= 100 ? 3 : 2;

  size_t index = 0;
  for (auto it = begin(); it != end(); ++it) {
    switch (it.get_opcode()) {
#define ZS_DECL_OPCODE(name, INST_TYPES)                                                          \
  case ZS_OPCODE_ENUM_VALUE(name):                                                                \
    zb::stream_print(stream, "index: ", zb::padded_number_t(index++, index_padding, ' '),         \
        " | offset: ", zb::padded_number_t(it.get_index(*this), offset_padding),                  \
        " | size: ", zb::padded_number_t(zs::get_instruction_size<ZS_OPCODE_ENUM_VALUE(name)>()), \
        " | name: ");                                                                             \
    it.get_ref<ZS_OPCODE_ENUM_VALUE(name)>().debug_print(stream);                                 \
    stream << "\n";                                                                               \
    break;

#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE

    default:
      zb::stream_print<" ">(stream, zs::opcode_to_string(it.get_opcode()), "\n");
    }
  }
}
} // namespace zs.
