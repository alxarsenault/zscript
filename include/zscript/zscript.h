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

#include <zscript/common.h>
#include <zscript/object.h>
#include <zscript/engine.h>
#include <zscript/vm.h>

#include <zscript/base/container/byte.h>
#include <zscript/base/sys/file_view.h>
#include <zscript/base/strings/string_view.h>
#include <zscript/base/utility/print.h>
#include <zscript/base/utility/traits.h>

#include <filesystem>
#include <string>
#include <stdexcept>

namespace zs {

inline std::ostream& operator<<(std::ostream& stream, const zb::source_location& loc) {
  return stream << "\nsource_location:\nfile: '" << loc.file_name() << "'\nfunction: '" << loc.function_name()
                << "'\nline: " << loc.line() << "\n";
}

enum class error_source { compiler, virtual_machine };

struct error_message {
  template <class Message, class Filename, class Code>
  inline error_message(zs::engine* eng, error_source esrc, zs::error_code ec, Message&& message,
      Filename&& filename, Code&& code, zs::line_info line, const zb::source_location& loc)
      : ec(ec)
      , err_source(esrc)
      , message(std::forward<Message>(message), eng)
      , filename(std::forward<Filename>(filename), eng)
      , code(std::forward<Code>(code), eng)
      , line(line)
      , loc(loc)
      , _uid(next_uid_counter()) {}

  inline std::ostream& print(std::ostream& stream) const {
    zb::stream_print(stream, "[", _uid, "] - error: ", ec, "\nerror-source: ", err_source, "\n");

    if (!message.empty()) {
      stream << "message: " << message;

      if (!message.ends_with('\n')) {
        stream << "\n";
      }
    }

    if (!filename.empty()) {
      stream << "file: " << filename << "\n";
    }

    if (line.line != -1) {
      stream << "line: " << line.line << ":" << line.column << "\n\n";
    }
    else {
      stream << "\n";
    }

    if (!code.empty()) {
      stream << "'''" << code << "\n\n'''\n";
    }

    stream << loc;

    return stream << "\n";
  }

  zs::error_code ec;
  zs::error_source err_source;
  zs::string message;
  zs::string filename;
  zs::string code;
  zs::line_info line;
  zb::source_location loc;
  size_t _uid = 0;

  static size_t next_uid_counter() {
    static size_t __uid = 0;
    return __uid++;
  }
};

struct error_stack : zs::vector<error_message> {
  inline error_stack(zs::engine* eng)
      : zs::vector<error_message>((zs::allocator<error_message>(eng))) {}

  inline void append(const error_stack& errs) { this->insert(this->begin(), errs.begin(), errs.end()); }

  inline std::ostream& print(std::ostream& stream) const {
    for (auto it = this->rbegin(); it != this->rend(); ++it) {
      it->print(stream);
    }

    return stream;
  }
};

template <class T>
ZS_CHECK inline constexpr T* allocator<T>::allocate(size_t n) {
  if (ZBASE_UNLIKELY(n > std::allocator_traits<allocator>::max_size(*this))) {
    zs::throw_error(zs::error_code::out_of_memory);
  }

  return static_cast<T*>(
      zs::allocate(_engine, n * sizeof(T), (alloc_info_t)ZS_IF_MEMORY_PROFILER_OR(_tag, 0)));
}

template <class T>
ZS_INLINE_CXPR void allocator<T>::deallocate(T* ptr, size_t) noexcept {
  zs::deallocate(_engine, ptr, (alloc_info_t)ZS_IF_MEMORY_PROFILER_OR(_tag, 0));
}

template <class T>
ZS_INLINE constexpr void detail::unique_ptr_deleter<T>::operator()(T* ptr) const noexcept {
  static_assert(sizeof(T) >= 0, "cannot delete an incomplete type");
  static_assert(!std::is_void_v<T>, "cannot delete an incomplete type");
  internal::zs_delete(_engine, ptr);
}

bool object_table_equal_to::operator()(const object_base& lhs, const object_base& rhs) const noexcept {
  return lhs.strict_equal(rhs);
}

bool object_table_equal_to::operator()(const object_base& lhs, std::string_view rhs) const noexcept {
  return this->operator()(lhs, zs::_sv(rhs));
}

bool object_table_equal_to::operator()(std::string_view lhs, const object_base& rhs) const noexcept {
  return this->operator()(zs::_sv(lhs), rhs);
}

std::ostream& operator<<(std::ostream& stream, const zs::object_base& obj) {
  return obj.stream_to_string(stream);
}

namespace constants {

  template <meta_method MetaMethod>
  ZS_CK_INLINE zs::object get() noexcept {
    if constexpr (false) {
      zb_static_error("invalid meta_method");
      return nullptr;
    }

#define _X(name, str)                                   \
  else if constexpr (MetaMethod == meta_method::name) { \
    return zs::_sv(constants::k_##name##_string);       \
  }
    ZS_META_METHOD_ENUM(_X)
#undef _X

    else {

      zb_static_error("invalid meta_method");
      return nullptr;
    }
  }

} // namespace constants

ZS_CK_INLINE zs::object get_meta_method_name_object(meta_method mm) noexcept {
  switch (mm) {
#define _X(name, str)     \
  case meta_method::name: \
    return zs::_sv(constants::k_##name##_string);

    ZS_META_METHOD_ENUM(_X)
#undef _X
  }

  ZS_ERROR("invalid meta method");
  return nullptr;
}

namespace constants {
  inline constexpr size_t k_default_stack_size = ZS_DEFAULT_STACK_SIZE;
} // namespace constants.

//

/// Default file loader.
class default_file_loader {
public:
  ZS_INLINE default_file_loader(ZB_MAYBE_UNUSED zs::engine* eng) noexcept {}

  ZS_CHECK zs::error_result open(const char* filepath) noexcept;

  ZS_CHECK zs::error_result open(std::string_view filepath) noexcept;

  ZS_CHECK zs::error_result open(const object& filepath) noexcept;

  template <class _Allocator>
  ZS_CK_INLINE zs::error_result open(
      const std::basic_string<char, std::char_traits<char>, _Allocator>& filepath) noexcept {
    return this->open(filepath.c_str());
  }

  ZS_CK_INLINE std::string_view content() const noexcept { return _fv.str(); }

  ZS_CK_INLINE size_t size() const noexcept { return _fv.size(); }

  ZS_CK_INLINE zb::byte_view data() const noexcept { return _fv.content(); }

private:
  zb::file_view _fv;
};

using file_loader = ZS_DEFAULT_FILE_LOADER;

//
// MARK: - API
//

ZS_CHECK virtual_machine* create_virtual_machine(size_t stack_size = ZS_DEFAULT_STACK_SIZE,
    allocate_t alloc_cb = ZS_DEFAULT_ALLOCATE, raw_pointer_t user_pointer = nullptr,
    raw_pointer_release_hook_t user_release = nullptr,
    stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER,
    engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER);

ZS_CHECK virtual_machine* create_virtual_machine(zs::engine* eng, size_t stack_size = ZS_DEFAULT_STACK_SIZE);

void close_virtual_machine(virtual_machine* v);

//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// MARK: - Implementation
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <class Fct>
object object::create_native_closure(zs::engine* eng, Fct&& fct) {

  if constexpr (std::is_convertible_v<Fct, int_t (*)(virtual_machine*)>
      or std::is_convertible_v<Fct, int_t (*)(const virtual_machine*)>) {
    zb_static_error("Invalid native closure function");
    return 0;
  }
  else if constexpr (std::is_convertible_v<Fct, zs::function_t>) {
    return create_native_closure(eng, (zs::function_t)fct);
  }
  else {
    struct closure_type : native_closure {

      inline closure_type(zs::engine* eng, Fct&& fct)
          : native_closure(eng)
          , _fct(std::forward<Fct>(fct)) {}

      virtual ~closure_type() override = default;

      virtual int_t call(zs::vm_ref vm) override {
        if constexpr (std::is_invocable_v<Fct, vm_ref>) {
          return _fct(vm);
        }
        else if constexpr (std::is_invocable_v<Fct>) {
          return _fct();
        }
        else {
          zb_static_error("can't call function");
          return 0;
        }
      }

      Fct _fct;
    };

    closure_type* nc = (closure_type*)allocate(eng, sizeof(closure_type));
    nc = zb_placement_new(nc) closure_type(eng, std::forward<Fct>(fct));
    return create_native_closure(eng, (zs::native_closure*)nc);
  }
}

//
// MARK: Convert to std::string
//

//
// zs::string::operator==
//

ZS_CK_INLINE bool operator==(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) == rhs;
}

ZS_CK_INLINE bool operator==(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) == rhs;
}

//
// zs::string::operator!=
//

ZS_CK_INLINE bool operator!=(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) != (const std::string&)rhs;
}

ZS_CK_INLINE bool operator!=(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) != (const std::string&)rhs;
}

//
// zs::string::operator<
//

ZS_CK_INLINE bool operator<(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) < rhs;
}

ZS_CK_INLINE bool operator<(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) < (const std::string&)rhs;
}

//
// zs::string::operator<=
//

ZS_CK_INLINE bool operator<=(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) <= rhs;
}

ZS_CK_INLINE bool operator<=(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) <= (const std::string&)rhs;
}

//
// zs::string::operator>
//

ZS_CK_INLINE bool operator>(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) > rhs;
}

ZS_CK_INLINE bool operator>(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) > (const std::string&)rhs;
}

//
// zs::string::operator>=
//

ZS_CK_INLINE bool operator>=(const zs::string& lhs, const std::string& rhs) noexcept {
  return std::string_view(lhs) >= rhs;
}

ZS_CK_INLINE bool operator>=(const std::string& lhs, const zs::string& rhs) noexcept {
  return std::string_view(lhs) >= (const std::string&)rhs;
}
} // namespace zs.
