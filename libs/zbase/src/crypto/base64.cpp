#include <zbase/crypto/base64.h>
#include <zbase/sys/assert.h>

ZBASE_BEGIN_SUB_NAMESPACE(base64)
//
//// https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/

//
namespace {
static constexpr const char* k_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static constexpr size_t k_invalid_index = -1;

#define K k_invalid_index

static constexpr size_t k_lookup_table[128] = {
  K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, // 0 - 15
  K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, K, // 16 - 31
  K, K, K, K, K, K, K, K, K, K, K, 62, K, K, K, 63, // 32 - 47
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, K, K, K, K, K, K, // 48 - 63
  K, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, // 64 - 79
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, K, K, K, K, K, // 80 - 96
  K, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 87 - 111
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, K, K, K, K, K // 112 - 127
};

#undef K

static inline void fill_buffer(const uint8_t* data, size_t len, char* ret) noexcept {
  constexpr uint8_t k_trailing_char = '=';

  switch (len) {
  case 1: {
    *(ret + 0) = k_alphabet[((*data) & 0xFC) >> 2];
    *(ret + 1) = k_alphabet[((*data) & 0x03) << 4];
    *(ret + 2) = k_trailing_char;
    *(ret + 3) = k_trailing_char;
    return;
  }
  case 2: {
    *(ret + 0) = k_alphabet[((*data) & 0xFC) >> 2];
    *(ret + 1) = k_alphabet[(((*data) & 0x03) << 4) + ((*(data + 1) & 0xF0) >> 4)];
    *(ret + 2) = k_alphabet[(*(data + 1) & 0x0F) << 2];
    *(ret + 3) = k_trailing_char;
    return;
  }
  case 3: {
    *(ret + 0) = k_alphabet[((*data) & 0xFC) >> 2];
    *(ret + 1) = k_alphabet[(((*data) & 0x03) << 4) + ((*(data + 1) & 0xF0) >> 4)];
    *(ret + 2) = k_alphabet[((*(data + 1) & 0x0F) << 2) + ((*(data + 2) & 0xC0) >> 6)];
    *(ret + 3) = k_alphabet[*(data + 2) & 0x3F];
    return;
  }
  }

  zbase_error("Invalid size");
}

static inline void fill_buffer(const uint8_t* data, char* ret) noexcept {
  *(ret + 0) = k_alphabet[((*data) & 0xFC) >> 2];
  *(ret + 1) = k_alphabet[(((*data) & 0x03) << 4) + ((*(data + 1) & 0xF0) >> 4)];
  *(ret + 2) = k_alphabet[((*(data + 1) & 0x0F) << 2) + ((*(data + 2) & 0xC0) >> 6)];
  *(ret + 3) = k_alphabet[*(data + 2) & 0x3F];
}
} // namespace.

size_t encode(const uint8_t* data, size_t size, char* output_data) {
  if (size <= 3) {
    fill_buffer(data, size, (char*)output_data);
    return 4;
  }

  uint32_t* out_buffer = (uint32_t*)output_data;

  const size_t pos_end = size - 3;
  size_t pos = 0;
  while (pos < pos_end) {
    fill_buffer(data + pos, (char*)(out_buffer++));
    pos += 3;
  }

  const size_t end_size = size - pos;

  if (end_size > 3) {
    zbase_error("end_size should be between [1, 3]");
    return 0;
  }

  fill_buffer(data + pos, end_size, (char*)out_buffer);

  return encoded_size(size);
}

//
// Decode.
//

size_t decode(const char* data, size_t size, uint8_t* output_data) {
  zbase_assert(data, "data cannot be null");
  zbase_assert(size >= 4, "size must be at least 4");

  while (data[size - 1] == '=') {
    size--;
  }

  uint8_t* out_ptr = output_data;

  const char* data_ptr = data;
  const size_t msize = (size_t)(std::max)(long(size) - 3L, 0L);

  size_t pos = 0;
  while (pos < msize) {
    const size_t p0 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p1 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p2 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p3 = k_lookup_table[(size_t)(*data_ptr++)];

    *out_ptr++ = (uint8_t)((p0 << 2) + ((p1 & 0x30) >> 4));
    *out_ptr++ = (uint8_t)(((p1 & 0x0F) << 4) + ((p2 & 0x3C) >> 2));
    *out_ptr++ = (uint8_t)(((p2 & 0x03) << 6) + p3);

    pos += 4;
  }

  const size_t left_size = size - pos;

  if (left_size == 1 || left_size > 4) {
    zbase_error("wrong size left");
    return 0;
  }

  switch (left_size) {
  case 2: {
    size_t p0 = k_lookup_table[(size_t)(*data_ptr++)];
    size_t p1 = k_lookup_table[(size_t)(*data_ptr++)];
    *out_ptr++ = (uint8_t)((p0 << 2) + ((p1 & 0x30) >> 4));
    break;
  }

  case 3: {
    const size_t p0 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p1 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p2 = k_lookup_table[(size_t)(*data_ptr++)];
    *out_ptr++ = (uint8_t)((p0 << 2) + ((p1 & 0x30) >> 4));
    *out_ptr++ = (uint8_t)(((p1 & 0x0F) << 4) + ((p2 & 0x3C) >> 2));
    break;
  }

  case 4: {
    const size_t p0 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p1 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p2 = k_lookup_table[(size_t)(*data_ptr++)];
    const size_t p3 = k_lookup_table[(size_t)(*data_ptr++)];
    *out_ptr++ = (uint8_t)((p0 << 2) + ((p1 & 0x30) >> 4));
    *out_ptr++ = (uint8_t)(((p1 & 0x0F) << 4) + ((p2 & 0x3C) >> 2));
    *out_ptr++ = (uint8_t)(((p2 & 0x03) << 6) + p3);
    break;
  }
  }

  return out_ptr - output_data;
}
ZBASE_END_SUB_NAMESPACE(base64)
