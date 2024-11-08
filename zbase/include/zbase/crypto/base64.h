//
// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <zbase/zbase.h>
#include <zbase/sys/error_code.h>
#include <string>
#include <string_view>
#include <span>
#include <vector>

ZBASE_BEGIN_SUB_NAMESPACE(base64)

/// Get the base64 encoded size.
ZB_CK_INLINE constexpr size_t encoded_size(size_t size) noexcept { return (size + 2) / 3 * 4; }

/// Get the maximum decoded size.
ZB_CK_INLINE constexpr size_t decoded_size(size_t size) noexcept { return size / 4 * 3; }

/// Encode the data as base64 into the output_data.
/// @warning output_data must have a length of at least `encoded_size()`.
ZB_CHECK size_t encode(const uint8_t* data, size_t size, char* output_data);

/// Encode the data as base64 into the OutputContainer.
/// @warning OutputContainer must be a byte container, i.e. `sizeof(Container::value_type) == 1`.
template <class OutputContainer = std::string>
ZB_CK_INLINE OutputContainer encode(const uint8_t* data, size_t size) {
  OutputContainer encoded;
  encoded.resize(encoded_size(size));

  size_t sz = encode(data, size, (char*)encoded.data());
  encoded.resize(sz);

  return encoded;
}

template <class OutputContainer, class Allocator>
ZB_CK_INLINE OutputContainer encode(const Allocator& a, const uint8_t* data, size_t size) {
  OutputContainer encoded(a);
  encoded.resize(encoded_size(size));

  size_t sz = encode(data, size, (char*)encoded.data());
  encoded.resize(sz);

  return encoded;
}

/// Encode the given container data to the OutputContainer.
/// @warning OutputContainer must be a byte container, i.e. `sizeof(Container::value_type) == 1`.
template <class OutputContainer = std::string>
ZB_CK_INLINE OutputContainer encode(std::string_view v) {
  return encode<OutputContainer>((const uint8_t*)v.data(), v.size());
}

template <class OutputContainer, class Allocator>
ZB_CK_INLINE OutputContainer encode(const Allocator& a, std::string_view v) {
  return encode<OutputContainer, Allocator>(a, (const uint8_t*)v.data(), v.size());
}

/// Decode a base64 string to the output_data.
/// @warning output_data must have a length of at least `decoded_size()`.
ZB_CHECK size_t decode(const char* data, size_t size, uint8_t* output_data);

/// Decode a base64 string to the OutputContainer.
template <class OutputContainer = std::vector<uint8_t>>
ZB_CK_INLINE OutputContainer decode(const char* data, size_t size) {
  OutputContainer decoded;
  decoded.resize(decoded_size(size));

  size_t sz = decode(data, size, (uint8_t*)decoded.data());
  decoded.resize(sz);
  return decoded;
}

template <class OutputContainer, class Allocator>
ZB_CK_INLINE OutputContainer decode(const Allocator& a, const char* data, size_t size) {
  OutputContainer decoded(a);
  decoded.resize(decoded_size(size));

  size_t sz = decode(data, size, (uint8_t*)decoded.data());
  decoded.resize(sz);
  return decoded;
}

/// Decode the base64 data from the given container to OutputContainer.
template <class OutputContainer = std::vector<uint8_t>>
ZB_CK_INLINE OutputContainer decode(std::span<const uint8_t> c) {
  return decode<OutputContainer>((const char*)c.data(), c.size());
}

template <class OutputContainer, class Allocator>
ZB_CK_INLINE OutputContainer decode(const Allocator& a, std::span<const uint8_t> c) {
  return decode<OutputContainer, Allocator>(a, (const char*)c.data(), c.size());
}

template <class OutputContainer = std::vector<uint8_t>>
ZB_CK_INLINE OutputContainer decode(std::string_view c) {
  return decode<OutputContainer>((const char*)c.data(), c.size());
}

template <class OutputContainer, class Allocator>
ZB_CK_INLINE OutputContainer decode(const Allocator& a, std::string_view c) {
  return decode<OutputContainer, Allocator>(a, (const char*)c.data(), c.size());
}
ZBASE_END_SUB_NAMESPACE(base64)
