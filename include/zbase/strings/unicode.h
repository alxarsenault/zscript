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

#pragma once

#include <zbase/zbase.h>
#include <zbase/utility/traits.h>
#include <ostream>
#include <string>
#include <string_view>

#include <string.h>
#include <ctype.h>
#include <wchar.h>

ZBASE_BEGIN_NAMESPACE

///
namespace unicode {

template <typename CharT, std::enable_if_t<sizeof(CharT) == sizeof(char), std::nullptr_t> = nullptr>
size_t utf8_length(const CharT* str, size_t size) noexcept;

template <typename CharT, std::enable_if_t<sizeof(CharT) == sizeof(char16_t), std::nullptr_t> = nullptr>
size_t utf16_length(const CharT* str, size_t size) noexcept;

template <typename CharT, std::enable_if_t<sizeof(CharT) == sizeof(char32_t), std::nullptr_t> = nullptr>
size_t utf32_length(const CharT*, size_t size) noexcept;

inline constexpr const uint16_t k_lead_surrogate_min = 0xD800u;
inline constexpr const uint16_t k_lead_offset = k_lead_surrogate_min - (0x10000 >> 10);
inline constexpr const uint16_t k_trail_surrogate_min = 0xDC00u;

/// Maximum valid value for a Unicode code point.
inline constexpr const uint32_t k_code_point_max = 0x0010FFFFu;
inline constexpr const uint32_t k_surrogate_offset
    = 0x10000u - (k_lead_surrogate_min << 10) - k_trail_surrogate_min;

inline constexpr const uint8_t bom[] = { 0xef, 0xbb, 0xbf };

template <class _U8CharT>
inline uint8_t cast_8(_U8CharT c) noexcept {
  return static_cast<uint8_t>(c);
}

template <class _U16CharT>
inline uint16_t cast_16(_U16CharT c) noexcept {
  return static_cast<uint16_t>(c);
}

template <class _U8CharT>
inline bool is_trail(_U8CharT c) noexcept {
  return (cast_8(c) >> 6) == 0x2;
}

inline constexpr bool is_surrogate(char16_t uc) noexcept {
  return ((uint32_t)uc - (uint32_t)k_lead_surrogate_min) < 2048u;
}

// Lead.
inline constexpr bool is_high_surrogate(char16_t uc) noexcept {
  return (uc & 0xFFFFFC00) == k_lead_surrogate_min;
}

// Trail.
inline constexpr bool is_low_surrogate(char16_t uc) noexcept {
  return (uc & 0xFFFFFC00) == k_trail_surrogate_min;
}

template <class _U32CharT>
inline constexpr bool is_valid_code_point(_U32CharT cp) noexcept {
  return cp <= k_code_point_max && !is_surrogate(cp);
}

inline constexpr size_t sequence_length(uint8_t lead) noexcept {
  return (size_t)(lead < 0x80   ? 1
          : (lead >> 5) == 0x6  ? 2
          : (lead >> 4) == 0xE  ? 3
          : (lead >> 3) == 0x1E ? 4
                                : 0);
}

inline constexpr size_t u16_sequence_length(char16_t c) noexcept {
  return (size_t)(is_high_surrogate(c) ? 2 : 1);
}

template <typename u8_iterator>
inline constexpr bool starts_with_bom(u8_iterator it, u8_iterator end) noexcept {
  return (((it != end) && (cast_8(*it++)) == bom[0]) && ((it != end) && (cast_8(*it++)) == bom[1])
      && ((it != end) && (cast_8(*it)) == bom[2]));
}

template <typename u8_iterator>
inline u8_iterator append_u32_to_u8(uint32_t cp, u8_iterator it) noexcept {
  using ctype = __zb::output_iterator_value_type_t<u8_iterator>;

  // 1 byte.
  if (cp < 0x80) {
    *it++ = static_cast<ctype>(cp);
  }
  // 2 bytes.
  else if (cp < 0x800) {
    *it++ = static_cast<ctype>((cp >> 6) | 0xC0);
    *it++ = static_cast<ctype>((cp & 0x3F) | 0x80);
  }
  // 3 bytes.
  else if (cp < 0x10000) {
    *it++ = static_cast<ctype>((cp >> 12) | 0xE0);
    *it++ = static_cast<ctype>(((cp >> 6) & 0x3F) | 0x80);
    *it++ = static_cast<ctype>((cp & 0x3F) | 0x80);
  }
  // 4 bytes.
  else {
    *it++ = static_cast<ctype>((cp >> 18) | 0xF0);
    *it++ = static_cast<ctype>(((cp >> 12) & 0x3F) | 0x80);
    *it++ = static_cast<ctype>(((cp >> 6) & 0x3F) | 0x80);
    *it++ = static_cast<ctype>((cp & 0x3F) | 0x80);
  }

  return it;
}

inline size_t code_point_size_u8(uint32_t cp) noexcept {
  // 1 byte.
  if (cp < 0x80) {
    return 1;
  }
  // 2 bytes.
  else if (cp < 0x800) {
    return 2;
  }
  // 3 bytes.
  else if (cp < 0x10000) {
    return 3;
  }
  // 4 bytes.
  else {
    return 4;
  }
}

template <typename u8_iterator>
inline uint32_t next_u8_to_u32(u8_iterator& it) noexcept {
  uint32_t cp = cast_8(*it);

  using difference_type = typename std::iterator_traits<u8_iterator>::difference_type;
  difference_type length = static_cast<difference_type>(sequence_length(static_cast<uint8_t>(*it)));

  switch (length) {
  case 1:
    break;
  case 2:
    it++;
    cp = ((cp << 6) & 0x7FF) + ((*it) & 0x3F);
    break;
  case 3:
    ++it;
    cp = ((cp << 12) & 0xFFFF) + ((cast_8(*it) << 6) & 0xFFF);
    ++it;
    cp += (*it) & 0x3F;
    break;
  case 4:
    ++it;
    cp = ((cp << 18) & 0x1FFFFF) + ((cast_8(*it) << 12) & 0x3FFFF);
    ++it;
    cp += (cast_8(*it) << 6) & 0xFFF;
    ++it;
    cp += (*it) & 0x3F;
    break;
  }
  ++it;
  return cp;
}

inline uint32_t next_u8_to_u32_s(const char* it) noexcept {
  uint32_t cp = cast_8(*it);

  using difference_type = typename std::iterator_traits<const char*>::difference_type;
  difference_type length = static_cast<difference_type>(sequence_length(static_cast<uint8_t>(*it)));

  switch (length) {
  case 1:
    break;
  case 2:
    it++;
    cp = ((cp << 6) & 0x7FF) + ((*it) & 0x3F);
    break;
  case 3:
    ++it;
    cp = ((cp << 12) & 0xFFFF) + ((cast_8(*it) << 6) & 0xFFF);
    ++it;
    cp += (*it) & 0x3F;
    break;
  case 4:
    ++it;
    cp = ((cp << 18) & 0x1FFFFF) + ((cast_8(*it) << 12) & 0x3FFFF);
    ++it;
    cp += (cast_8(*it) << 6) & 0xFFF;
    ++it;
    cp += (*it) & 0x3F;
    break;
  }
  //  ++it;
  return cp;
}

template <typename octet_iterator>
uint32_t prior_u8_to_u32(octet_iterator& it) noexcept {
  while (is_trail(*(--it)))
    ;

  octet_iterator temp = it;
  return next_u8_to_u32(temp);
}

template <typename u16_iterator, typename u8_iterator>
u16_iterator u8_to_u16(u8_iterator start, u8_iterator end, u16_iterator outputIt) noexcept {
  while (start < end) {
    uint32_t cp = next_u8_to_u32(start);

    if (cp > 0xFFFF) { // make a surrogate pair
      *outputIt++ = cast_16((cp >> 10) + k_lead_offset);
      *outputIt++ = cast_16((cp & 0x3FF) + k_trail_surrogate_min);
    }
    else {
      *outputIt++ = cast_16(cp);
    }
  }

  return outputIt;
}

template <typename u8_iterator>
size_t u8_to_u16_length(u8_iterator start, u8_iterator end) noexcept {
  size_t count = 0;
  while (start < end) {
    uint32_t cp = next_u8_to_u32(start);
    count += (cp > 0xFFFF) ? 2 : 1;
  }

  return count;
}

template <typename u32_iterator, typename u8_iterator>
u32_iterator u8_to_u32(u8_iterator start, u8_iterator end, u32_iterator outputIt) noexcept {
  using ctype = __zb::output_iterator_value_type_t<u32_iterator>;

  while (start < end) {
    *outputIt++ = static_cast<ctype>(next_u8_to_u32(start));
  }

  return outputIt;
}

template <typename u8_iterator>
size_t u8_to_u32_length(u8_iterator start, u8_iterator end) noexcept {

  size_t count = 0;
  while (start < end) {
    switch (sequence_length(static_cast<uint8_t>(*start++))) {
    case 1:
      break;
    case 2:
      ++start;
      break;
    case 3:
      ++start;
      ++start;
      break;
    case 4:
      ++start;
      ++start;
      ++start;
      break;
    }
    count++;
  }

  return count;
}
template <typename u16_iterator, typename u8_iterator>
u8_iterator u16_to_u8(u16_iterator start, u16_iterator end, u8_iterator outputIt) noexcept {
  while (start != end) {
    uint32_t cp = cast_16(*start++);
    //

    // Take care of surrogate pairs first.
    if (is_high_surrogate(static_cast<char16_t>(cp))) {
      cp = (cp << 10) + static_cast<uint32_t>(cast_16(*start++)) + k_surrogate_offset;
    }

    outputIt = append_u32_to_u8(cp, outputIt);
  }

  return outputIt;
}

template <typename u16_iterator, typename u32_iterator>
u32_iterator u16_to_u32(u16_iterator start, u16_iterator end, u32_iterator outputIt) noexcept {
  using ctype = __zb::output_iterator_value_type_t<u32_iterator>;

  while (start != end) {
    uint32_t cp = cast_16(*start++);

    // Take care of surrogate pairs first.
    if (is_high_surrogate(static_cast<char16_t>(cp))) {
      cp = (cp << 10) + static_cast<uint32_t>(cast_16(*start++)) + k_surrogate_offset;
    }

    *outputIt++ = static_cast<ctype>(cp);
  }

  return outputIt;
}

template <typename u16_iterator>
size_t u16_to_u8_length(u16_iterator start, u16_iterator end) noexcept {
  size_t count = 0;
  while (start != end) {
    uint32_t cp = cast_16(*start++);

    // Take care of surrogate pairs first.
    if (is_high_surrogate(static_cast<char16_t>(cp))) {
      cp = (cp << 10) + static_cast<uint32_t>(cast_16(*start++)) + k_surrogate_offset;
    }

    count += code_point_size_u8(cp);
  }

  return count;
}

template <typename u16_iterator>
size_t u16_to_u32_length(u16_iterator start, u16_iterator end) noexcept {
  size_t count = 0;

  while (start != end) {
    uint32_t cp = cast_16(*start++);

    // Take care of surrogate pairs first.
    if (is_high_surrogate(static_cast<char16_t>(cp))) {
      cp = (cp << 10) + static_cast<uint32_t>(cast_16(*start++)) + k_surrogate_offset;
    }

    count++;
  }

  return count;
}

template <typename u8_iterator, typename u32_iterator>
u8_iterator u32_to_u8(u32_iterator start, u32_iterator end, u8_iterator outputIt) noexcept {

  while (start != end) {
    outputIt = append_u32_to_u8(static_cast<uint32_t>(*start++), outputIt);
  }

  return outputIt;
}

template <typename u16_iterator, typename u32_iterator>
u16_iterator u32_to_u16(u32_iterator start, u32_iterator end, u16_iterator outputIt) noexcept {
  while (start != end) {
    uint32_t cp = static_cast<uint32_t>(*start++);

    using value_type = __zb::output_iterator_value_type_t<u16_iterator>;

    if (cp <= 0x0000FFFF) {
      // UTF-16 surrogate values are illegal in UTF-32
      // 0xFFFF or 0xFFFE are both reserved values.
      if (cp >= 0xD800 && cp <= 0xDFFF) {
        *outputIt++ = 0x0000FFFD;
      }
      else {
        // BMP character.
        *outputIt++ = static_cast<value_type>(cp);
      }
    }
    else if (cp > 0x0010FFFF) {
      // U+10FFFF is the largest code point of Unicode character set.
      *outputIt++ = static_cast<value_type>(0x0000FFFD);
    }
    else {
      // c32 is a character in range 0xFFFF - 0x10FFFF.
      cp -= 0x0010000UL;
      *outputIt++ = static_cast<value_type>(((cp >> 10) + 0xD800));
      *outputIt++ = static_cast<value_type>(((cp & 0x3FFUL) + 0xDC00));
    }
  }

  return outputIt;
}

template <typename u32_iterator>
size_t u32_to_u8_length(u32_iterator start, u32_iterator end) noexcept {
  size_t count = 0;

  while (start != end) {
    count += code_point_size_u8(static_cast<uint32_t>(*start++));
  }

  return count;
}

template <typename u32_iterator>
size_t u32_to_u16_length(u32_iterator start, u32_iterator end) noexcept {
  size_t count = 0;

  while (start != end) {
    uint32_t cp = static_cast<uint32_t>(*start++);
    count += (cp <= 0x0000FFFF || cp > 0x0010FFFF) ? 1 : 2;
  }

  return count;
}

template <typename CharT, std::enable_if_t<sizeof(CharT) == sizeof(char), std::nullptr_t>>
size_t utf8_length(const CharT* str, size_t size) noexcept {
  size_t dist = 0;

  for (size_t i = 0; i < size; i += sequence_length(static_cast<uint8_t>(str[i]))) {
    dist++;
  }

  return dist;
}

template <typename CharT, std::enable_if_t<sizeof(CharT) == sizeof(char16_t), std::nullptr_t>>
size_t utf16_length(const CharT* str, size_t size) noexcept {
  size_t dist = 0;

  for (size_t i = 0; i < size; i++) {
    if (is_high_surrogate(cast_16(str[i]))) {
      i++;
    }

    dist++;
  }

  return dist;
}

template <typename CharT, std::enable_if_t<sizeof(CharT) == sizeof(char32_t), std::nullptr_t>>
size_t utf32_length(const CharT*, size_t size) noexcept {
  return size;
}

template <typename CharT, class SType,
    std::enable_if_t<is_utf_string_type<SType>::value && is_utf_char_type<CharT>::value, std::nullptr_t>
    = nullptr>
inline size_t convert_size(const SType& str) noexcept;

//
template <class SType, std::enable_if_t<is_utf_string_type<SType>::value, std::nullptr_t> = nullptr>
class convert;

//
template <typename _StringType, typename SType,
    std::enable_if_t<__zb::is_utf_basic_string_type<_StringType>::value && is_utf_string_type<SType>::value,
        std::nullptr_t>
    = nullptr>
inline _StringType convert_as(const SType& str) noexcept;

//
template <class SType, class _OutputContainer,
    std::enable_if_t<is_utf_string_type<SType>::value && __zb::is_contiguous_container_v<_OutputContainer>,
        std::nullptr_t>
    = nullptr>
inline void append_to(const SType& str, _OutputContainer& c_output) noexcept;

///
///
///
template <class SType, class OutputIt,
    std::enable_if_t<is_utf_string_type<SType>::value, std::nullptr_t> = nullptr>
inline OutputIt copy(const SType& str, OutputIt outputIt) noexcept;

///
///
///
template <class SType, __zb::enable_if_utf_string_type_t<SType> = nullptr>
inline size_t length(const SType& str) noexcept;

template <typename CharT, typename SType,
    std::enable_if_t<is_utf_string_type<SType>::value && is_utf_char_type<CharT>::value, std::nullptr_t>>
inline size_t convert_size(const SType& str) noexcept {
  using input_char_type = string_char_type_t<SType>;
  constexpr char_encoding input_encoding = __zb::utf_encoding_of<input_char_type>::value;

  using output_char_type = CharT;
  constexpr char_encoding output_encoding = __zb::utf_encoding_of<output_char_type>::value;

  std::basic_string_view<input_char_type> input_view(str);

  if constexpr (input_encoding == char_encoding::utf8) {
    if constexpr (output_encoding == char_encoding::utf8) {
      return input_view.size();
    }
    else if constexpr (output_encoding == char_encoding::utf16) {
      return u8_to_u16_length(input_view.begin(), input_view.end());
    }
    else if constexpr (output_encoding == char_encoding::utf32) {
      return u8_to_u32_length(input_view.begin(), input_view.end());
    }
    else {
      return 0;
    }
  }
  else if constexpr (input_encoding == char_encoding::utf16) {
    if constexpr (output_encoding == char_encoding::utf8) {
      return u16_to_u8_length(input_view.begin(), input_view.end());
    }
    else if constexpr (output_encoding == char_encoding::utf16) {
      return input_view.size();
    }
    else if constexpr (output_encoding == char_encoding::utf32) {
      return u16_to_u32_length(input_view.begin(), input_view.end());
    }
    else {
      return 0;
    }
  }
  else if constexpr (input_encoding == char_encoding::utf32) {
    if constexpr (output_encoding == char_encoding::utf8) {
      return u32_to_u8_length(input_view.begin(), input_view.end());
    }
    else if constexpr (output_encoding == char_encoding::utf16) {
      return u32_to_u16_length(input_view.begin(), input_view.end());
    }
    else if constexpr (output_encoding == char_encoding::utf32) {
      return input_view.size();
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
}

//
template <class SType, class _OutputContainer,
    std::enable_if_t<is_utf_string_type<SType>::value && __zb::is_contiguous_container_v<_OutputContainer>,
        std::nullptr_t>>
inline void append_to(const SType& str, _OutputContainer& c_output) noexcept {
  using input_char_type = string_char_type_t<SType>;
  constexpr char_encoding input_encoding = __zb::utf_encoding_of<input_char_type>::value;

  using output_char_type = __zb::container_value_type_t<_OutputContainer>;
  constexpr char_encoding output_encoding = __zb::utf_encoding_of<output_char_type>::value;

  std::basic_string_view<input_char_type> input_view(str);
  const size_t input_size = input_view.size();

  if constexpr (input_encoding == output_encoding) {
    const size_t output_size = c_output.size();
    c_output.resize(output_size + input_size);
    ::memmove(c_output.data() + output_size, input_view.data(), input_size * sizeof(output_char_type));
    return;
  }
  else if constexpr (input_encoding == char_encoding::utf8) {
    // u8 -> u16
    if constexpr (output_encoding == char_encoding::utf16) {
      const size_t output_size = c_output.size();
      const size_t conv_size = u8_to_u16_length(input_view.begin(), input_view.end());
      c_output.resize(output_size + conv_size);
      output_char_type* output_it = c_output.data() + output_size;
      auto cit_begin = input_view.begin();
      auto cit_end = input_view.end();

      while (cit_begin < cit_end) {
        uint32_t cp = next_u8_to_u32(cit_begin);

        if (cp > 0xFFFF) { // make a surrogate pair
          *output_it++ = cast_16((cp >> 10) + k_lead_offset);
          *output_it++ = cast_16((cp & 0x3FF) + k_trail_surrogate_min);
        }
        else {
          *output_it++ = cast_16(cp);
        }
      }

      return;
    }

    // u8 ->u32
    else if constexpr (output_encoding == char_encoding::utf32) {
      const size_t output_size = c_output.size();
      const size_t conv_size = u8_to_u32_length(input_view.begin(), input_view.end());
      c_output.resize(output_size + conv_size);
      output_char_type* output_it = c_output.data() + output_size;
      auto cit_begin = input_view.begin();
      auto cit_end = input_view.end();
      while (cit_begin < cit_end) {
        *output_it++ = static_cast<output_char_type>(next_u8_to_u32(cit_begin));
      }
      return;
    }
  }
  else if constexpr (input_encoding == char_encoding::utf16) {
    // u16 -> u8
    if constexpr (output_encoding == char_encoding::utf8) {
      const size_t output_size = c_output.size();
      const size_t conv_size = u16_to_u8_length(input_view.begin(), input_view.end());
      c_output.resize(output_size + conv_size);
      output_char_type* output_it = c_output.data() + output_size;

      auto cit_begin = input_view.begin();
      auto cit_end = input_view.end();
      while (cit_begin != cit_end) {
        uint32_t cp = cast_16(*cit_begin++);

        // Take care of surrogate pairs first.
        if (is_high_surrogate(static_cast<char16_t>(cp))) {
          cp = (cp << 10) + static_cast<uint32_t>(cast_16(*cit_begin++)) + k_surrogate_offset;
        }

        output_it = append_u32_to_u8(cp, output_it);
      }
      return;
    }

    // u16 -> u32
    else if constexpr (output_encoding == char_encoding::utf32) {
      const size_t output_size = c_output.size();
      const size_t conv_size = u16_to_u32_length(input_view.begin(), input_view.end());
      c_output.resize(output_size + conv_size);
      output_char_type* output_it = c_output.data() + output_size;
      auto cit_begin = input_view.begin();
      auto cit_end = input_view.end();
      while (cit_begin != cit_end) {
        uint32_t cp = cast_16(*cit_begin++);

        // Take care of surrogate pairs first.
        if (is_high_surrogate(static_cast<char16_t>(cp))) {
          cp = (cp << 10) + static_cast<uint32_t>(cast_16(*cit_begin++)) + k_surrogate_offset;
        }

        *output_it++ = static_cast<output_char_type>(cp);
      }
      return;
    }
  }
  else if constexpr (input_encoding == char_encoding::utf32) {
    // u32 -> u8
    if constexpr (output_encoding == char_encoding::utf8) {
      const size_t output_size = c_output.size();
      const size_t conv_size = u32_to_u8_length(
          input_view.begin(), input_view.end()); // utf_cvt_size<output_char_type>(input_view);
      c_output.resize(output_size + conv_size);
      output_char_type* output_it = c_output.data() + output_size;

      auto cit_begin = input_view.begin();
      auto cit_end = input_view.end();

      while (cit_begin != cit_end) {
        output_it = append_u32_to_u8(static_cast<uint32_t>(*cit_begin++), output_it);
      }

      return;
    }

    // u32 -> u16
    else if constexpr (output_encoding == char_encoding::utf16) {
      const size_t output_size = c_output.size();
      const size_t conv_size = u32_to_u16_length(input_view.begin(), input_view.end());
      c_output.resize(output_size + conv_size);
      output_char_type* output_it = c_output.data() + output_size;

      auto cit_begin = input_view.begin();
      auto cit_end = input_view.end();

      while (cit_begin != cit_end) {
        uint32_t cp = static_cast<uint32_t>(*cit_begin++);

        if (cp <= 0x0000FFFF) {
          // UTF-16 surrogate values are illegal in UTF-32
          // 0xFFFF or 0xFFFE are both reserved values.
          if (cp >= 0xD800 && cp <= 0xDFFF) {
            *output_it++ = 0x0000FFFD;
          }
          else {
            // BMP character.
            *output_it++ = static_cast<output_char_type>(cp);
          }
        }
        else if (cp > 0x0010FFFF) {
          // U+10FFFF is the largest code point of Unicode character set.
          *output_it++ = static_cast<output_char_type>(0x0000FFFD);
        }
        else {
          // c32 is a character in range 0xFFFF - 0x10FFFF.
          cp -= 0x0010000UL;
          *output_it++ = static_cast<output_char_type>(((cp >> 10) + 0xD800));
          *output_it++ = static_cast<output_char_type>(((cp & 0x3FFUL) + 0xDC00));
        }
      }

      return;
    }
  }
}

template <class SType, class OutputIt,
    std::enable_if_t<__zb::is_utf_string_type<SType>::value, std::nullptr_t>>
inline OutputIt copy(const SType& str, OutputIt outputIt) noexcept {

  using input_char_type = string_char_type_t<SType>;
  constexpr char_encoding input_encoding = __zb::utf_encoding_of<input_char_type>::value;

  using output_char_type = __zb::output_iterator_value_type_t<OutputIt>;
  constexpr char_encoding output_encoding = __zb::utf_encoding_of<output_char_type>::value;

  std::basic_string_view<input_char_type> input_view(str);

  if constexpr (input_encoding == char_encoding::utf8) {
    if constexpr (output_encoding == char_encoding::utf8) {
      const size_t _size = input_view.size();
      for (size_t i = 0; i < _size; i++) {
        *outputIt++ = static_cast<output_char_type>(input_view[i]);
      }
      return outputIt;
    }
    else if constexpr (output_encoding == char_encoding::utf16) {
      return u8_to_u16(input_view.begin(), input_view.end(), outputIt);
    }
    else if constexpr (output_encoding == char_encoding::utf32) {
      return u8_to_u32(input_view.begin(), input_view.end(), outputIt);
    }
  }
  else if constexpr (input_encoding == char_encoding::utf16) {
    if constexpr (output_encoding == char_encoding::utf8) {
      return u16_to_u8(input_view.begin(), input_view.end(), outputIt);
    }
    else if constexpr (output_encoding == char_encoding::utf16) {
      for (size_t i = 0; i < input_view.size(); i++) {
        *outputIt++ = static_cast<output_char_type>(input_view[i]);
      }
      return outputIt;
    }
    else if constexpr (output_encoding == char_encoding::utf32) {
      return u16_to_u32(input_view.begin(), input_view.end(), outputIt);
    }
  }
  else if constexpr (input_encoding == char_encoding::utf32) {
    if constexpr (output_encoding == char_encoding::utf8) {
      return u32_to_u8(input_view.begin(), input_view.end(), outputIt);
    }
    else if constexpr (output_encoding == char_encoding::utf16) {
      return u32_to_u16(input_view.begin(), input_view.end(), outputIt);
    }
    else if constexpr (output_encoding == char_encoding::utf32) {
      for (size_t i = 0; i < input_view.size(); i++) {
        *outputIt++ = static_cast<output_char_type>(input_view[i]);
      }
      return outputIt;
    }
  }
}

template <class SType, __zb::enable_if_utf_string_type_t<SType>>
inline size_t length(const SType& str) noexcept {
  using input_char_type = __zb::string_char_type_t<SType>;
  constexpr char_encoding input_encoding = __zb::utf_encoding_of<input_char_type>::value;

  std::basic_string_view<input_char_type> input_view(str);

  if constexpr (input_encoding == char_encoding::utf8) {
    return utf8_length(input_view.data(), input_view.size());
  }
  else if constexpr (input_encoding == char_encoding::utf16) {
    return utf16_length(input_view.data(), input_view.size());
  }
  else if constexpr (input_encoding == char_encoding::utf32) {
    return utf32_length(input_view.data(), input_view.size());
  }
  else {
    return 0;
  }
}

///
///
///
template <class SType, std::enable_if_t<__zb::is_utf_string_type<SType>::value, std::nullptr_t>>
class convert {
public:
  using input_char_type = __zb::string_char_type_t<SType>;
  static constexpr char_encoding input_encoding = __zb::utf_encoding_of<input_char_type>::value;

  inline convert(const SType& str) noexcept
      : _input_view(str) {}

  template <typename _StringType,
      std::enable_if_t<__zb::is_utf_basic_string_type<_StringType>::value, std::nullptr_t> = nullptr>
  inline operator _StringType() const noexcept {
    return convert_as<_StringType>(_input_view);
  }

private:
  std::basic_string_view<input_char_type> _input_view;
};

template <class SType>
convert(const SType&) -> convert<SType>;

template <typename _StringType, typename SType,
    std::enable_if_t<__zb::is_utf_basic_string_type<_StringType>::value
            && __zb::is_utf_string_type<SType>::value,
        std::nullptr_t>>
inline _StringType convert_as(const SType& str) noexcept {
  _StringType out;
  append_to(str, out);
  return out;
}

//----------------------------------------------------------------------------------------------------------------------
///
///
///
template <typename CharT, typename SType>
inline auto iterate_as(const SType& str) noexcept;

///
///
///
template <typename SType, std::enable_if_t<__zb::is_utf_string_type<SType>::value, std::nullptr_t> = nullptr>
inline auto iterate(const SType& str) noexcept;

///
///
///
template <class IteratorType>
class iterator;

///
///
///
template <class CharT, class SType, class IteratorType,
    std::enable_if_t<__zb::is_utf_string_type<SType>::value, std::nullptr_t> = nullptr>
class basic_iterator;

namespace detail {
  template <char_encoding Encoding = char_encoding::utf8>
  struct iterator_sequence_length {
    template <typename T>
    static inline constexpr size_t length(T lead) noexcept {
      return sequence_length(static_cast<uint8_t>(lead));
    }
  };

  template <>
  struct iterator_sequence_length<char_encoding::utf16> {
    template <typename T>
    static inline constexpr size_t length(T c) noexcept {
      return u16_sequence_length(static_cast<char16_t>(c));
    }
  };

  template <>
  struct iterator_sequence_length<char_encoding::utf32> {
    template <typename T>
    static inline constexpr size_t length(T) noexcept {
      return 1;
    }
  };

  template <typename InputCharT, typename OutputChart, typename = void>
  struct base_iterator {};

  template <typename InputCharT, typename OutputCharT>
  struct base_iterator<InputCharT, OutputCharT,
      std::enable_if_t<__zb::utf_encoding_of<InputCharT>::value
          == __zb::utf_encoding_of<OutputCharT>::value>> {
    using input_char_type = InputCharT;
    using output_char_type = OutputCharT;

    using input_view_type = std::basic_string_view<input_char_type>;
    using output_view_type = std::basic_string_view<output_char_type>;

    using it_seq_length = detail::iterator_sequence_length<__zb::utf_encoding_of<InputCharT>::value>;

    template <typename Iterator>
    static inline output_view_type get(Iterator it) noexcept {
      return output_view_type(
          reinterpret_cast<const output_char_type*>(&(it[0])), it_seq_length::length(*it));
    }

    template <typename Iterator>
    static inline void advance(Iterator& it) noexcept {
      using it_diff_type = typename std::iterator_traits<Iterator>::difference_type;
      std::advance(it, static_cast<it_diff_type>(it_seq_length::length(*it)));
    }
  };

  template <typename InputCharT, typename OutputCharT>
  struct base_iterator<InputCharT, OutputCharT,
      std::enable_if_t<__zb::utf_encoding_of<InputCharT>::value != utf_encoding_of<OutputCharT>::value>> {

    using input_char_type = InputCharT;
    using output_char_type = OutputCharT;

    using input_view_type = std::basic_string_view<input_char_type>;
    using output_view_type = std::basic_string_view<output_char_type>;

    using it_seq_length = detail::iterator_sequence_length<utf_encoding_of<InputCharT>::value>;

    template <typename Iterator>
    inline output_view_type get(Iterator it) const noexcept {
      return output_view_type(_data.begin(),
          static_cast<size_t>(std::distance(_data.begin(),
              __zb::unicode::copy(input_view_type(&it[0], it_seq_length::length(*it)), _data.begin()))));
    }

    template <typename Iterator>
    inline void advance(Iterator& it) noexcept {
      using it_diff_type = typename std::iterator_traits<Iterator>::difference_type;
      std::advance(it, static_cast<it_diff_type>(it_seq_length::length(*it)));
    }

    mutable std::array<output_char_type,
        utf_encoding_to_max_char_count<__zb::utf_encoding_of<OutputCharT>::value>::value>
        _data;
  };

  template <class IteratorType>
  using iterator_value_type = std::remove_cvref_t<decltype(std::declval<IteratorType>()[0])>;

  template <class IteratorType>
  using iterator_base_type = basic_iterator<iterator_value_type<IteratorType>,
      std::basic_string_view<iterator_value_type<IteratorType>>, IteratorType>;

  template <typename IteratorT>
  class iterator_range {
    IteratorT _begin_iterator;
    IteratorT _end_iterator;

  public:
    template <typename Container>
    inline iterator_range(Container&& c) noexcept
        : _begin_iterator(c.begin())
        , _end_iterator(c.end()) {}

    inline iterator_range(IteratorT begin_iterator, IteratorT end_iterator) noexcept
        : _begin_iterator(std::move(begin_iterator))
        , _end_iterator(std::move(end_iterator)) {}

    inline IteratorT begin() const noexcept { return _begin_iterator; }
    inline IteratorT end() const noexcept { return _end_iterator; }
    inline bool empty() const noexcept { return _begin_iterator == _end_iterator; }
  };

} // namespace detail.

// NANO_UNICODE_CLANG_PUSH_WARNING("-Wpadded")

template <class CharT, class SType, class IteratorType,
    std::enable_if_t<is_utf_string_type<SType>::value, std::nullptr_t>>
class basic_iterator : private detail::base_iterator<__zb::string_char_type_t<SType>, CharT> {
  using base_type = detail::base_iterator<__zb::string_char_type_t<SType>, CharT>;
  using output_view_type = typename base_type::output_view_type;

  template <class T>
  friend class iterator;

public:
  typedef ptrdiff_t difference_type;
  typedef std::forward_iterator_tag iterator_category;

  inline basic_iterator() noexcept = default;

  inline explicit basic_iterator(IteratorType it) noexcept
      : _it(it) {}

  inline IteratorType base() const noexcept { return _it; }

  inline output_view_type operator*() const noexcept { return base_type::get(_it); }

  inline bool operator==(const basic_iterator& rhs) const noexcept { return (_it == rhs._it); }

  inline bool operator!=(const basic_iterator& rhs) const noexcept { return !(operator==(rhs)); }

  inline basic_iterator& operator++() noexcept {
    base_type::advance(_it);
    return *this;
  }

  inline basic_iterator operator++(int) noexcept {
    basic_iterator temp = *this;
    base_type::advance(_it);
    return temp;
  }

private:
  IteratorType _it;
};

template <typename CharT, typename SType>
inline auto iterate_as(const SType& str) noexcept {
  return detail::iterator_range<basic_iterator<CharT, SType, typename SType::const_iterator>>(str);
}

template <typename SType, std::enable_if_t<is_utf_string_type<SType>::value, std::nullptr_t>>
inline auto iterate(const SType& str) noexcept {
  return detail::iterator_range<
      basic_iterator<string_char_type_t<SType>, SType, typename SType::const_iterator>>(str);
}

template <class IteratorType>
class iterator : private detail::iterator_base_type<IteratorType> {
  using base_type = detail::iterator_base_type<IteratorType>;

public:
  typedef ptrdiff_t difference_type;
  typedef std::forward_iterator_tag iterator_category;

  inline iterator() noexcept = default;

  inline explicit iterator(IteratorType it) noexcept
      : _it(it) {}

  inline IteratorType base() const noexcept { return _it; }

  inline typename base_type::output_view_type operator*() const noexcept { return base_type::get(_it); }

  inline bool operator==(const iterator& rhs) const noexcept { return (_it == rhs._it); }

  inline bool operator!=(const iterator& rhs) const noexcept { return !(operator==(rhs)); }

  inline iterator& operator++() noexcept {
    base_type::advance(_it);
    return *this;
  }

  inline iterator operator++(int) noexcept {
    iterator temp = *this;
    base_type::advance(_it);
    return temp;
  }

private:
  IteratorType _it;
};

template <class IteratorType>
iterator(IteratorType) -> iterator<IteratorType>;

uint32_t to_upper(uint32_t c) noexcept;
uint32_t to_lower(uint32_t c) noexcept;

inline size_t u32_to_u8_index(std::string_view str, size_t u32_index) noexcept {

  // Length of the string in u32.
  const size_t u32_length = length(str);

  // Length of the string in u8.
  const size_t u8_length = str.size();

  // Find the u8 index from the u32 input index.
  for (size_t i = 0, u32_i = 0; i < u8_length; i += sequence_length(str[i])) {
    if (u32_i++ == u32_index) {
      return i;
    }
  }

  return std::string_view::npos;
}

inline size_t u32_index_to_u8_index(std::string_view str, size_t u32_index, size_t u32_length) noexcept {

  // Length of the string in u8.
  const size_t u8_length = str.size();

  // Find the u8 index from the u32 input index.
  for (size_t i = 0, u32_i = 0; i < u8_length; i += sequence_length(str[i])) {
    if (u32_i++ == u32_index) {
      return i;
    }
  }

  return std::string_view::npos;
}

} // namespace unicode
ZBASE_END_NAMESPACE
