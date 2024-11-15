#pragma once

#include <zscript.h>
#include "bytecode/zopcode.h"

namespace zs {

template <opcode Op, class = void>
struct instruction_t {
  opcode op = Op;
};

namespace constants {
  inline constexpr size_t k_biggest_instruction_name_size = zb::maximum(0
#define ZS_DECL_OPCODE(name, INST_TYPES) , std::string_view(#name).size()
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE
  );

} // namespace constants.

#define __ZS_INSTRUCTION_TYPE(type, name) type name;
#define __ZS_INSTRUCTION_PARAM_TYPE(type, name) , type name
#define __ZS_INSTRUCTION_INIT_MEMBER(type, name) , name(name)

#define __ZS_INSTRUCTION_PRINT_TYPE(type, name) , " ", #name, ":", inst.name
#define __ZS_INSTRUCTION_DEBUG_PRINT_TYPE(type, name) , " <", #type, " ", #name, "> = ", name

#define ZS_DECLARE_INSTRUCTION(name, TYPES_MACRO)                                                   \
  ZBASE_PACKED_START                                                                                \
  template <>                                                                                       \
  struct instruction_t<ZS_OPCODE_ENUM_VALUE(name)> {                                                \
    static constexpr zs::opcode s_code = ZS_OPCODE_ENUM_VALUE(name);                                \
    inline constexpr instruction_t() noexcept = default;                                            \
                                                                                                    \
    inline constexpr instruction_t(zs::opcode op TYPES_MACRO(__ZS_INSTRUCTION_PARAM_TYPE)) noexcept \
        : op(op) TYPES_MACRO(__ZS_INSTRUCTION_INIT_MEMBER) {                                        \
      zbase_assert(op == s_code, "invalid opcode expected op_" #name);                              \
    }                                                                                               \
                                                                                                    \
    inline std::ostream& debug_print(std::ostream& stream) const {                                  \
      return zb::stream_print(stream,                                                               \
          zb::left_aligned_t(#name, zs::constants::k_biggest_instruction_name_size)                 \
              TYPES_MACRO(__ZS_INSTRUCTION_DEBUG_PRINT_TYPE));                                      \
    }                                                                                               \
                                                                                                    \
    inline friend std::ostream& operator<<(std::ostream& stream, const instruction_t& inst) {       \
      return zb::stream_print(stream, #name TYPES_MACRO(__ZS_INSTRUCTION_PRINT_TYPE));              \
    }                                                                                               \
                                                                                                    \
    zs::opcode op;                                                                                  \
    TYPES_MACRO(__ZS_INSTRUCTION_TYPE)                                                              \
  };                                                                                                \
  ZBASE_PACKED_END

#define ZS_DECL_OPCODE(name, INST_TYPES) ZS_DECLARE_INSTRUCTION(name, INST_TYPES)
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE

namespace constants {
  inline constexpr size_t k_instruction_count = (size_t)opcode::count;

  inline constexpr std::array<uint8_t, k_instruction_count> k_instruction_sizes = {
#define ZS_DECL_OPCODE(name, INST_TYPES) (uint8_t)sizeof(instruction_t<ZS_OPCODE_ENUM_VALUE(name)>),
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE
  };

  inline constexpr size_t k_biggest_instruction_size = zb::maximum(
#define ZS_DECL_OPCODE(name, INST_TYPES) sizeof(instruction_t<ZS_OPCODE_ENUM_VALUE(name)>),
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE
      0);

} // namespace constants.

template <opcode Op>
ZS_CK_INLINE_CXPR size_t get_instruction_size() noexcept {
  return sizeof(instruction_t<Op>);
}

ZS_CK_INLINE_CXPR size_t get_instruction_size(opcode op) noexcept {
  zbase_assert((size_t)op < constants::k_instruction_count, "out of bounds instruction");
  return constants::k_instruction_sizes[(size_t)op];
}

} // namespace zs.
