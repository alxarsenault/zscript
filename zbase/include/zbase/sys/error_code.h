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
#include <zbase/utility/traits.h>
#include <zbase/utility/enum.h>
#include <zbase/utility/print.h>
#include <ostream>
#include <optional>

ZBASE_BEGIN_NAMESPACE

namespace generic_error_result_detail {

ZBASE_DECL_USING_DECLTYPE(meta_is_valid, is_valid);
ZBASE_DECL_HAS_MEMBER(has_is_valid, meta_is_valid);

ZBASE_DECL_USING_DECLTYPE(meta_default_value, default_value);
ZBASE_DECL_HAS_MEMBER(has_default_value, meta_default_value);

ZBASE_DECL_USING_DECLTYPE(meta_to_string, to_string);
ZBASE_DECL_HAS_MEMBER(has_to_string, meta_to_string);

template <class T, class = void>
struct get_enum_type {
  using type = typename T::enum_type;
};

template <class T>
struct get_enum_type<T, std::enable_if_t<std::is_enum_v<T>>> {
  using type = T;
};
} // namespace generic_error_result_detail.

template <class ErrorDescriptor>
struct generic_error_result;

template <class ErrorDescriptor>
struct generic_status_result {
  using descriptor = ErrorDescriptor;
  using enum_type = typename generic_error_result_detail::get_enum_type<descriptor>::type;
  using message_type = std::conditional_t<generic_error_result_detail::has_to_string<descriptor>::value,
      const char*, std::string_view>;

  enum_type code = default_value();

  ZB_ALWAYS_INLINE constexpr generic_status_result() noexcept = default;
  ZB_ALWAYS_INLINE constexpr generic_status_result(const generic_status_result&) noexcept = default;
  ZB_ALWAYS_INLINE constexpr generic_status_result(generic_status_result&&) noexcept = default;

  ZB_ALWAYS_INLINE constexpr generic_status_result(generic_error_result<ErrorDescriptor> ec) noexcept;

  ZB_ALWAYS_INLINE constexpr generic_status_result(enum_type c) noexcept
      : code(c) {}

  ZB_ALWAYS_INLINE ~generic_status_result() noexcept = default;

  ZB_ALWAYS_INLINE constexpr generic_status_result& operator=(const generic_status_result&) noexcept
      = default;
  ZB_ALWAYS_INLINE constexpr generic_status_result& operator=(generic_status_result&&) noexcept = default;

  static ZB_ALWAYS_INLINE constexpr enum_type default_value() noexcept {
    if constexpr (generic_error_result_detail::has_default_value<descriptor>::value) {
      return descriptor::default_value;
    }
    else {
      return (enum_type)0;
    }
  }

  /// Returns false on error.
  [[nodiscard]] ZB_ALWAYS_INLINE constexpr explicit operator bool() const noexcept { return is_valid(); }

  ///
  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool is_valid() const noexcept {
    if constexpr (generic_error_result_detail::has_is_valid<descriptor>::value) {
      return descriptor::is_valid(code);
    }
    else {
      return code == default_value();
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator==(generic_status_result c) const noexcept {
    return code == c.code;
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator!=(generic_status_result c) const noexcept {
    return code != c.code;
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator==(
      generic_error_result<ErrorDescriptor> ec) const noexcept;

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator!=(
      generic_error_result<ErrorDescriptor> ec) const noexcept;
  //

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr message_type message() const noexcept {
    if constexpr (generic_error_result_detail::has_to_string<descriptor>::value) {
      return descriptor::to_string(code);
    }
    else {
      return __zb::enum_name(code);
    }
  }

  friend ZB_ALWAYS_INLINE std::ostream& operator<<(
      std::ostream& stream, const __zb::generic_status_result<ErrorDescriptor>& err) {
    return stream << err.message();
  }
};

template <class ErrorDescriptor>
struct generic_error_result {
  using descriptor = ErrorDescriptor;
  using enum_type = typename generic_error_result_detail::get_enum_type<descriptor>::type;
  using message_type = std::conditional_t<generic_error_result_detail::has_to_string<descriptor>::value,
      const char*, std::string_view>;

  enum_type code = default_value();

  ZB_ALWAYS_INLINE constexpr generic_error_result() noexcept = default;
  ZB_ALWAYS_INLINE constexpr generic_error_result(const generic_error_result&) noexcept = default;
  ZB_ALWAYS_INLINE constexpr generic_error_result(generic_error_result&&) noexcept = default;

  ZB_ALWAYS_INLINE constexpr generic_error_result(generic_status_result<descriptor> err) noexcept
      : code(err.code) {}

  ZB_ALWAYS_INLINE constexpr generic_error_result(enum_type c) noexcept
      : code(c) {}

  ZB_ALWAYS_INLINE ~generic_error_result() noexcept = default;

  ZB_ALWAYS_INLINE constexpr generic_error_result& operator=(const generic_error_result&) noexcept = default;
  ZB_ALWAYS_INLINE constexpr generic_error_result& operator=(generic_error_result&&) noexcept = default;

  static ZB_ALWAYS_INLINE constexpr enum_type default_value() noexcept {
    if constexpr (generic_error_result_detail::has_default_value<descriptor>::value) {
      return descriptor::default_value;
    }
    else {
      return (enum_type)0;
    }
  }

  /// Returns true on error.
  [[nodiscard]] ZB_ALWAYS_INLINE constexpr explicit operator bool() const noexcept { return !is_valid(); }

  ///
  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool is_valid() const noexcept {
    if constexpr (generic_error_result_detail::has_is_valid<descriptor>::value) {
      return descriptor::is_valid(code);
    }
    else {
      return code == default_value();
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator==(generic_error_result c) const noexcept {
    return code == c.code;
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator!=(generic_error_result c) const noexcept {
    return code != c.code;
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator==(enum_type c) const noexcept { return code == c; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool operator!=(enum_type c) const noexcept { return code != c; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr message_type message() const noexcept {
    if constexpr (generic_error_result_detail::has_to_string<descriptor>::value) {
      return descriptor::to_string(code);
    }
    else {
      return __zb::enum_name(code);
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr operator enum_type() const noexcept { return code; }

  friend ZB_ALWAYS_INLINE std::ostream& operator<<(
      std::ostream& stream, const __zb::generic_error_result<ErrorDescriptor>& err) {
    return stream << err.message();
  }
};

template <class ErrorDescriptor>
constexpr generic_status_result<ErrorDescriptor>::generic_status_result(
    generic_error_result<ErrorDescriptor> err) noexcept
    : code(err.code) {}

template <class ErrorDescriptor>
constexpr bool generic_status_result<ErrorDescriptor>::operator==(
    generic_error_result<ErrorDescriptor> er) const noexcept {
  return code == er.code;
}

template <class ErrorDescriptor>
constexpr bool generic_status_result<ErrorDescriptor>::operator!=(
    generic_error_result<ErrorDescriptor> er) const noexcept {
  return code != er.code;
}

template <class ErrorDescriptor, class T>
class generic_optional_result {
public:
  using descriptor = ErrorDescriptor;
  using enum_type = typename generic_error_result_detail::get_enum_type<descriptor>::type;
  using value_type = T;

  ZB_ALWAYS_INLINE constexpr generic_optional_result()
      : generic_optional_result(value_type{}) {}

  ZB_ALWAYS_INLINE constexpr generic_optional_result(const value_type& value)
      : _value(value) {}

  ZB_ALWAYS_INLINE constexpr generic_optional_result(value_type&& value)
      : _value(std::forward<value_type>(value)) {}

  ZB_ALWAYS_INLINE constexpr generic_optional_result(enum_type err) noexcept
      : _error(err) {}

  ZB_ALWAYS_INLINE constexpr generic_optional_result(enum_type err, const value_type& value) noexcept
      : _value(value)
      , _error(err) {}

  ZB_ALWAYS_INLINE constexpr generic_optional_result(enum_type err, value_type&& value) noexcept
      : _value(std::forward<value_type>(value))
      , _error(err) {}

  template <class U,
      std::enable_if_t<
          std::is_convertible_v<U, value_type> && !std::is_same_v<std::remove_cvref_t<U>, value_type>, int>
      = 0>
  ZB_ALWAYS_INLINE constexpr generic_optional_result(U&& value)
      : _value(std::forward<U>(value)) {}

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr explicit operator bool() const noexcept { return is_valid(); }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool is_valid() const noexcept {
    return has_value() && !has_error();
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool has_value() const noexcept {
    if constexpr (std::is_trivial_v<value_type>) {
      return !has_error();
    }
    else {
      return _value.has_value();
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool has_error() const noexcept {
    if constexpr (generic_error_result_detail::has_is_valid<descriptor>::value) {
      return !descriptor::is_valid(_error);
    }
    else {
      return _error != default_error_value();
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr value_type& value() {
    if constexpr (std::is_trivial_v<value_type>) {
      return _value;
    }
    else {
      return _value.value();
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr const value_type& value() const {
    if constexpr (std::is_trivial_v<value_type>) {
      return _value;
    }
    else {
      return _value.value();
    }
  }

  ZB_INLINE constexpr value_type* operator->() noexcept { return &value(); }
  ZB_INLINE constexpr const value_type* operator->() const noexcept { return &value(); }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr enum_type error() const noexcept { return _error; }

  template <class _U>
  inline void set_value(_U&& v) {
    _value = std::forward<_U>(v);
  }

  inline void set_error(enum_type e) { _error = e; }

  friend ZB_ALWAYS_INLINE std::ostream& operator<<(
      std::ostream& stream, const __zb::generic_optional_result<ErrorDescriptor, T>& result) {

    if (result) {

      __zb::stream_print(stream, result.value());
      return stream;
    }
    return stream << __zb::generic_error_result<ErrorDescriptor>(result.error());
  }

private:
  std::conditional_t<std::is_trivial_v<value_type>, value_type, std::optional<value_type>> _value;
  enum_type _error = default_error_value();

  static ZB_ALWAYS_INLINE constexpr enum_type default_error_value() noexcept {
    if constexpr (generic_error_result_detail::has_default_value<descriptor>::value) {
      return descriptor::default_value;
    }
    else {
      return (enum_type)0;
    }
  }
};

template <class ErrorDescriptor>
class generic_optional_result<ErrorDescriptor, void> {
public:
  using descriptor = ErrorDescriptor;
  using enum_type = typename generic_error_result_detail::get_enum_type<descriptor>::type;
  using value_type = void;

  ZB_ALWAYS_INLINE constexpr generic_optional_result() noexcept = default;

  ZB_ALWAYS_INLINE constexpr generic_optional_result(enum_type err) noexcept
      : _error(err) {}

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr explicit operator bool() const noexcept { return is_valid(); }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool is_valid() const noexcept { return !has_error(); }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool has_value() const noexcept { return false; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr bool has_error() const noexcept {
    if constexpr (generic_error_result_detail::has_is_valid<descriptor>::value) {
      return !descriptor::is_valid(_error);
    }
    else {
      return _error != default_error_value();
    }
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr enum_type error() const noexcept { return _error; }

private:
  enum_type _error = default_error_value();

  static ZB_ALWAYS_INLINE constexpr enum_type default_error_value() noexcept {
    if constexpr (generic_error_result_detail::has_default_value<descriptor>::value) {
      return descriptor::default_value;
    }
    else {
      return (enum_type)0;
    }
  }
};

//
//
//

inline constexpr int32_t k_custom_error_code_first_index = 5000;

#define ZBASE_ERROR_CODE_ENUM(X)                                                  \
  X(success, "success", = 0)                                                      \
  X(done, "done", = 1)                                                            \
  X(unknown, "unknown", = -1)                                                     \
  X(invalid, "invalid", = k_custom_error_code_first_index)                        \
                                                                                  \
  /* Script */                                                                    \
                                                                                  \
  X(conversion_error, "conversion_error", )                                       \
  X(memory_error, "memory_error", )                                               \
  X(invalid_type, "invalid_type", )                                               \
  X(null_type, "null_type", )                                                     \
                                                                                  \
  /* Number */                                                                    \
                                                                                  \
  X(zero_division, "zero_division", )                                             \
  X(zero_modulus, "zero_modulus", )                                               \
  X(invalid_modulus, "invalid_modulus", )                                         \
                                                                                  \
  /* Custom */                                                                    \
                                                                                  \
  X(result_out_of_range, "result_out_of_range", )                                 \
  X(unreachable, "unreachable", )                                                 \
  X(address_family_not_supported, "address_family_not_supported", )               \
  X(address_in_use, "address_in_use", )                                           \
  X(address_not_available, "address_not_available", )                             \
  X(already_connected, "already_connected", )                                     \
  X(argument_list_too_long, "argument_list_too_long", )                           \
  X(argument_out_of_domain, "argument_out_of_domain", )                           \
  X(bad_address, "bad_address", )                                                 \
  X(bad_file_descriptor, "bad_file_descriptor", )                                 \
  X(bad_message, "bad_message", )                                                 \
  X(broken_pipe, "broken_pipe", )                                                 \
  X(connection_aborted, "connection_aborted", )                                   \
  X(connection_already_in_progress, "connection_already_in_progress", )           \
  X(connection_refused, "connection_refused", )                                   \
  X(connection_reset, "connection_reset", )                                       \
  X(cross_device_link, "cross_device_link", )                                     \
  X(destination_address_required, "destination_address_required", )               \
  X(device_or_resource_busy, "device_or_resource_busy", )                         \
  X(directory_not_empty, "directory_not_empty", )                                 \
  X(executable_format_error, "executable_format_error", )                         \
  X(file_exists, "file_exists", )                                                 \
  X(file_too_large, "file_too_large", )                                           \
  X(filename_too_long, "filename_too_long", )                                     \
  X(function_not_supported, "function_not_supported", )                           \
  X(host_unreachable, "host_unreachable", )                                       \
  X(identifier_removed, "identifier_removed", )                                   \
  X(illegal_byte_sequence, "illegal_byte_sequence", )                             \
  X(inappropriate_io_control_operation, "inappropriate_io_control_operation", )   \
  X(interrupted, "interrupted", )                                                 \
  X(invalid_argument, "invalid_argument", )                                       \
  X(invalid_seek, "invalid_seek", )                                               \
  X(io_error, "io_error", )                                                       \
  X(is_a_directory, "is_a_directory", )                                           \
  X(message_size, "message_size", )                                               \
  X(network_down, "network_down", )                                               \
  X(network_reset, "network_reset", )                                             \
  X(network_unreachable, "network_unreachable", )                                 \
  X(no_buffer_space, "no_buffer_space", )                                         \
  X(no_lock_available, "no_lock_available", )                                     \
  X(no_message_available, "no_message_available", )                               \
  X(no_message, "no_message", )                                                   \
  X(no_such_device_or_address, "no_such_device_or_address", )                     \
  X(no_such_device, "no_such_device", )                                           \
  X(no_such_file_or_directory, "no_such_file_or_directory", )                     \
  X(not_a_directory, "not_a_directory", )                                         \
  X(not_enough_memory, "not_enough_memory", )                                     \
  X(not_supported, "not_supported", )                                             \
  X(cancelled, "cancelled", )                                                     \
  X(permission_denied, "permission_denied", )                                     \
                                                                                  \
  /* Custom */                                                                    \
                                                                                  \
  X(already_created, "already_created", )                                         \
  X(invalid_file_format, "invalid_file_format", )                                 \
  X(invalid_file_content, "invalid_file_content", )                               \
  X(invalid_channel_size, "invalid_channel_size", )                               \
  X(invalid_audio_format, "invalid_audio_format", )                               \
  X(empty_data, "empty_data", )                                                   \
  X(name_exists, "name_exists", )                                                 \
  X(runtime_error, "runtime error", )                                             \
  X(syntax_error, "syntax error", )                                               \
  X(script_error, "script error", )                                               \
                                                                                  \
  /* Audio */                                                                     \
                                                                                  \
  X(not_running, "not_running", )                                                 \
  X(unsupported_property, "unsupported_property", )                               \
  X(unknown_property, "unknown_property", )                                       \
  X(invalid_property_size, "invalid_property_size", )                             \
  X(illegal_operation, "illegal_operation", )                                     \
  X(invalid_object, "invalid_object", )                                           \
  X(invalid_device, "invalid_device", )                                           \
  X(invalid_stream, "invalid_stream", )                                           \
  X(not_ready, "not_ready", )                                                     \
  X(unsupported_format, "unsupported_format", )                                   \
                                                                                  \
  /* HTTP */                                                                      \
                                                                                  \
  X(bad_url, "bad_url", )                                                         \
  X(timed_out, "timed_out", )                                                     \
  X(unsupported_url, "unsupported_url", )                                         \
  X(cannot_find_host, "cannot_find_host", )                                       \
  X(cannot_connect_to_host, "cannot_connect_to_host", )                           \
  X(network_connection_lost, "network_connection_lost", )                         \
  X(dns_lookup_failed, "dns_lookup_failed", )                                     \
  X(http_too_many_redirects, "http_too_many_redirects", )                         \
  X(resource_unavailable, "resource_unavailable", )                               \
  X(not_connected_to_internet, "not_connected_to_internet", )                     \
  X(redirect_to_non_existent_location, "redirect_to_non_existent_location", )     \
  X(bad_server_response, "bad_server_response", )                                 \
  X(user_cancelled_authentication, "user_cancelled_authentication", )             \
  X(user_authentication_required, "user_authentication_required", )               \
  X(zero_byte_resource, "zero_byte_resource", )                                   \
  X(cannot_decode_raw_data, "cannot_decode_raw_data", )                           \
  X(cannot_decode_content_data, "cannot_decode_content_data", )                   \
  X(cannot_parse_response, "cannot_parse_response", )                             \
  X(app_transport_security_requires_secure_connection,                            \
      "app_transport_security_requires_secure_connection", )                      \
  X(file_does_not_exist, "file_does_not_exist", )                                 \
  X(file_is_directory, "file_is_directory", )                                     \
  X(no_permissions_to_read_file, "no_permissions_to_read_file", )                 \
  X(data_length_exceeds_maximum, "data_length_exceeds_maximum", )                 \
  X(file_outside_safe_area, "file_outside_safe_area", )                           \
  X(secure_connection_failed, "secure_connection_failed", )                       \
  X(server_certificate_has_bad_date, "server_certificate_has_bad_date", )         \
  X(server_certificate_untrusted, "server_certificate_untrusted", )               \
  X(server_certificate_has_unknown_root, "server_certificate_has_unknown_root", ) \
  X(server_certificate_not_yet_valid, "server_certificate_not_yet_valid", )       \
  X(client_certificate_rejected, "client_certificate_rejected", )                 \
  X(client_certificate_required, "client_certificate_required", )                 \
  X(cannot_load_from_network, "cannot_load_from_network", )                       \
                                                                                  \
  /* Custom */                                                                    \
                                                                                  \
  X(read_error, "read error", )                                                   \
  X(data_overflow, "data overflow", )                                             \
  X(invalid_data, "invalid ata", )                                                \
  X(invalid_pointer, "invalid pointer", )                                         \
  X(buffer_too_small, "buffer too small", )                                       \
  X(end_of_file, "end_of_file", )                                                 \
  X(not_found, "not_found", )

enum class error_code : int32_t {
#define _X(name, str, idx) name idx,
  ZBASE_ERROR_CODE_ENUM(_X)
#undef _X
};

using errc = error_code;

[[nodiscard]] inline constexpr const char* status_message(__zb::error_code code) noexcept {
  switch (code) {
#define _X(name, str, idx)     \
  case __zb::error_code::name: \
    return str;
    ZBASE_ERROR_CODE_ENUM(_X)
#undef _X
  }

  return "unknown";
}

struct error_result_descriptor {
  using enum_type = __zb::error_code;
  static constexpr enum_type default_value = enum_type::success;
  static ZB_ALWAYS_INLINE constexpr bool is_valid(enum_type v) noexcept { return v == enum_type::success; }

  static ZB_ALWAYS_INLINE constexpr const char* to_string(enum_type code) noexcept {
    return __zb::status_message(code);
  }
};

using error_result = __zb::generic_error_result<__zb::error_result_descriptor>;
using status_result = __zb::generic_status_result<__zb::error_result_descriptor>;

template <class T>
using optional_result = __zb::generic_optional_result<__zb::error_result_descriptor, T>;

using noresult = __zb::optional_result<void>;

template <class Exception>
inline void throw_exception(const char* msg) {
  // #ifdef AASLIB_USE_EXCEPTION
  throw Exception(msg);
  // #else
  //         fprintf(stderr, "ERROR: %s\n", msg);
  //         std::abort();
  // #endif // AASLIB_USE_EXCEPTION
}

ZBASE_END_NAMESPACE

template <class CharT, class CharTraits>
inline std::basic_ostream<CharT, CharTraits>& operator<<(
    std::basic_ostream<CharT, CharTraits>& stream, __zb::error_code sc) {
  return stream << __zb::status_result(sc).message();
}

// template <class CharT, class CharTraits>
// inline std::basic_ostream<CharT, CharTraits>& operator<<(
//     std::basic_ostream<CharT, CharTraits>& stream, __zb::error_result err) {
//   return stream << err.message();
// }

template <class CharT, class CharTraits>
inline std::basic_ostream<CharT, CharTraits>& operator<<(
    std::basic_ostream<CharT, CharTraits>& stream, __zb::status_result st) {
  return stream << st.message();
}
