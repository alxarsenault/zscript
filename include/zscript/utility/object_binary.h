#pragma once

#include <zscript/zscript.h>

namespace zs {

ZS_CHECK zs::error_result get_binary_size(const object& obj, size_t& write_size);

ZS_CHECK zs::error_result to_binary(
    const object& obj, write_function_t write_function_t, size_t& write_size, void* data, uint32_t flags = 0);

ZS_CHECK zs::error_result from_binary(
    zs::engine* eng, std::span<uint8_t> buffer, object& out, size_t& offset);

template <class VectorType>
ZS_CK_INLINE zs::error_result to_binary(
    const object& obj, VectorType& buffer, size_t& write_size, uint32_t flags = 0) {
  return to_binary(
      obj,
      (write_function_t)[](const uint8_t* content, size_t size, void* udata)->zs::error_result {
        VectorType& vec = *(VectorType*)udata;

        vec.insert(vec.end(), content, content + size);
        return zs::error_code::success;
      },
      write_size, &buffer, flags);
}

} // namespace zs.
