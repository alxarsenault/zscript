#pragma once

#include <zscript/common.h>
#include <zscript/types.h>

#include <zbase/crypto/hash.h>
#include <zbase/container/small_vector.h>
#include <zbase/container/span.h>
#include <zbase/container/vector.h>
#include <zbase/strings/string_view.h>

#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace zs {

//
// MARK: Allocation interface.
//

/// Allocate memory.
void* allocate(zs::engine* eng, size_t sz, alloc_info_t ainfo = alloc_info_t{});

/// Reallocate memory.
void* reallocate(
    zs::engine* eng, void* ptr, size_t size, size_t old_size, alloc_info_t ainfo = alloc_info_t{});

/// Deallocate memory.
void deallocate(zs::engine* eng, void* ptr, alloc_info_t ainfo = alloc_info_t{});

enum class memory_tag {
  nt_unknown,
  nt_engine,
  nt_vm,
  nt_array,
  nt_table,
  nt_struct,
  nt_class,
  nt_string,
  nt_user_data,
  nt_native_closure,
  nt_weak_ptr,
  nt_capture,
  nt_allocator
};

struct internal {
  template <class T, class... Args>
  ZS_INLINE static T* zs_new(zs::engine* eng, Args&&... args) {
    T* ptr = (T*)zs::allocate(eng, sizeof(T));
    ptr = zb_placement_new(ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  template <memory_tag Tag, class T, class... Args>
  ZS_INLINE static T* zs_new(zs::engine* eng, Args&&... args) {
    T* ptr = (T*)zs::allocate(eng, sizeof(T), (alloc_info_t)Tag);
    ptr = zb_placement_new(ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  template <class T>
  ZS_INLINE static void zs_delete(zs::engine* eng, T* ptr) {
    ptr->~T();
    zs::deallocate(eng, ptr);
  }

  template <class T>
  ZS_INLINE static void zs_delete(zs::engine* eng, T* ptr, memory_tag tag) {
    ptr->~T();
    zs::deallocate(eng, ptr, (alloc_info_t)tag);
  }

  template <memory_tag Tag, class T>
  ZS_INLINE static void zs_delete(zs::engine* eng, T* ptr) {
    ptr->~T();
    zs::deallocate(eng, ptr, (alloc_info_t)Tag);
  }

  template <class T>
  struct proxy {};

  template <class T>
  struct test_helper {};
};

template <class T, class... Args>
ZS_INLINE static T* zs_new(zs::engine* eng, Args&&... args) {
  return internal::zs_new<T, Args...>(eng, std::forward<Args>(args)...);
}

template <memory_tag Tag, class T, class... Args>
ZS_INLINE T* zs_new(zs::engine* eng, Args&&... args) {
  return internal::zs_new<Tag, T, Args...>(eng, std::forward<Args>(args)...);
}

template <class T>
ZS_INLINE void zs_delete(zs::engine* eng, T* ptr) {
  internal::zs_delete<T>(eng, ptr);
}

template <memory_tag Tag, class T>
ZS_INLINE void zs_delete(zs::engine* eng, T* ptr) {
  internal::zs_delete<Tag, T>(eng, ptr);
}

class engine_holder {
public:
  ZS_INLINE_CXPR engine_holder(zs::engine* eng) noexcept
      : _engine(eng) {}

  ZS_CK_INLINE_CXPR zs::engine* get_engine() const noexcept { return _engine; }

protected:
  zs::engine* _engine;
};

class reference_counted : public engine_holder {
public:
  ZS_CLASS_COMMON;

  reference_counted(zs::engine* eng) noexcept;

  reference_counted(reference_counted&&) = delete;
  reference_counted(const reference_counted&) = delete;
  reference_counted& operator=(reference_counted&&) = delete;
  reference_counted& operator=(const reference_counted&) = delete;

  void retain() noexcept;
  virtual bool release() noexcept;

  ZS_CK_INLINE size_t ref_count() const noexcept { return _ref_count; }

protected:
  virtual ~reference_counted() = 0;

  size_t _ref_count = 1;
};

namespace detail {
  template <class T>
  struct unique_ptr_deleter : public engine_holder {
    static_assert(!std::is_function_v<T>, "unique_ptr_deleter cannot be instantiated for function types");

    ZS_INLINE unique_ptr_deleter(zs::engine* eng)
        : engine_holder(eng) {}

    template <class U>
    ZS_INLINE_CXPR unique_ptr_deleter(
        const unique_ptr_deleter<U>&, std::enable_if_t<std::is_convertible_v<U*, T*>>* = 0) noexcept {}

    ZS_INLINE_CXPR void operator()(T* ptr) const noexcept;
  };
} // namespace detail.

//
// MARK: Allocator
//

template <class T>
class allocator : public engine_holder {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::false_type;

  ZS_INLINE_CXPR allocator(zs::engine* eng) noexcept
      : engine_holder(eng) ZS_IF_MEMORY_PROFILER(, _tag(memory_tag::nt_allocator)) {}

  ZS_INLINE_CXPR allocator(zs::engine* eng, memory_tag tag) noexcept
      : engine_holder(eng) ZS_IF_MEMORY_PROFILER(, _tag(tag)) {}

  inline constexpr allocator(const allocator&) noexcept = default;
  inline constexpr allocator(allocator&&) noexcept = default;

  template <class U>
  ZS_INLINE_CXPR allocator(const allocator<U>& a) noexcept
      : engine_holder(a.get_engine()) ZS_IF_MEMORY_PROFILER(, _tag(a._tag)) {}

  inline constexpr allocator& operator=(const allocator&) noexcept = default;
  inline constexpr allocator& operator=(allocator&&) noexcept = default;

  ZS_CK_INLINE_CXPR T* allocate(size_t n);

  ZS_INLINE_CXPR void deallocate(T* ptr, size_t) noexcept;

  ZS_INLINE_CXPR bool operator==(const allocator& a) const noexcept { return _engine == a.get_engine(); }

  template <class U>
  ZS_CK_INLINE_CXPR bool operator==(const allocator<U>& a) const noexcept {
    return _engine == a.get_engine();
  }

  template <class U>
  struct rebind {
    using other = allocator<U>;
  };

private:
  template <class U>
  friend class allocator;

  ZS_IF_MEMORY_PROFILER(memory_tag _tag);
};

/// unique_ptr.
template <class T>
using unique_ptr = std::unique_ptr<T, zs::detail::unique_ptr_deleter<T>>;

using string_allocator = zs::allocator<char>;

/// string.
using string = std::basic_string<char, std::char_traits<char>, zs::string_allocator>;

/// vector.
template <class T>
using vector = zb::vector<T, zs::allocator<T>>;

/// small_vector.
template <class _T, size_t _Size>
using small_vector = zb::small_vector<_T, _Size, zs::allocator<_T>>;

/// unordered_map_allocator.
template <class Key, class Value>
using unordered_map_allocator = zs::allocator<std::pair<const Key, Value>>;

/// unordered_map.
template <class Key, class T, class Hash = zb::rapid_hasher<Key>, class Pred = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, T, Hash, Pred, zs::unordered_map_allocator<Key, T>>;

/// unordered_set.
template <class T, class Hash = zb::rapid_hasher<T>, class Pred = std::equal_to<T>>
using unordered_set = std::unordered_set<T, Hash, Pred, zs::allocator<T>>;

using ostringstream = std::basic_ostringstream<char, std::char_traits<char>, zs::string_allocator>;

namespace detail {
  template <class T>
  inline T create_string_stream(zs::engine* eng) {
    if constexpr (std::is_constructible_v<T, std::ios_base::openmode, zs::string_allocator>) {
      return T(std::ios_base::out, zs::string_allocator(eng));
    }
    else {
      return T(zs::string("", zs::string_allocator(eng)), std::ios_base::out);
    }
  }
} // namespace detail.

inline zs::ostringstream create_string_stream(zs::engine* eng) {
  return detail::create_string_stream<zs::ostringstream>(eng);
}

using unordered_object_map_allocator = zs::allocator<std::pair<const object, object>>;

struct object_table_hash {
  using is_transparent = void;

  ZS_CHECK size_t operator()(const object_base& obj) const noexcept;
  ZS_CHECK size_t operator()(std::string_view s) const noexcept;
  ZS_CK_INLINE size_t operator()(const std::string& s) const noexcept {
    return operator()(std::string_view(s));
  }
  ZS_CK_INLINE size_t operator()(const char* s) const noexcept { return operator()(std::string_view(s)); }
};

struct object_table_equal_to {
  using is_transparent = void;

  ZS_CK_INLINE bool operator()(const object_base& lhs, const object_base& rhs) const noexcept;
  ZS_CK_INLINE bool operator()(const object_base& lhs, std::string_view rhs) const noexcept;
  ZS_CK_INLINE bool operator()(std::string_view lhs, const object_base& rhs) const noexcept;
};

template <class T>
class object_unordered_map : public zs::unordered_map<object, T, object_table_hash, object_table_equal_to> {
public:
  using base_type = zs::unordered_map<object, T, object_table_hash, object_table_equal_to>;

  using base_type::base_type;
  using base_type::operator[];

  ZS_CK_INLINE object& operator[](std::string_view s);
  ZS_CK_INLINE object& operator[](const char* s);

  template <size_t N>
  ZS_CK_INLINE object& operator[](const char (&s)[N]) noexcept;
};

using object_unordered_set = zs::unordered_set<object, object_table_hash, object_table_equal_to>;

class reference_counted_object : public reference_counted {
public:
  ZS_OBJECT_CLASS_COMMON;

  reference_counted_object(zs::engine* eng, zs::object_type objtype) noexcept;

  reference_counted_object(reference_counted_object&&) = delete;
  reference_counted_object(const reference_counted_object&) = delete;
  reference_counted_object& operator=(reference_counted_object&&) = delete;
  reference_counted_object& operator=(const reference_counted_object&) = delete;

  virtual bool release() noexcept override;
  virtual object clone() const noexcept = 0;

protected:
  virtual ~reference_counted_object() override;

private:
  friend class weak_ref_object;

  weak_ref_object* _weak_ref_object = nullptr;

public:
  object_type _obj_type;

private:
  object get_weak_ref(const object_base& obj);
};

template <zb::separator Separator = " ", class... Args>
ZS_INLINE std::ostream& print(const Args&... args) {
  return zb::print<Separator>(args...);
}

template <zb::separator Separator = "", class... Args>
ZS_INLINE zs::string strprint(zs::engine* eng, const Args&... args) {
  zs::ostringstream stream(zs::create_string_stream(eng));
  zb::stream_print<Separator>(stream, args...);
  return zs::string(stream.str(), zs::string_allocator(eng));
}

template <class... Args>
ZS_INLINE zs::string sstrprint(zs::engine* eng, const Args&... args) {
  zs::ostringstream stream(zs::create_string_stream(eng));
  zb::stream_print<" ">(stream, args...);
  return zs::string(stream.str(), zs::string_allocator(eng));
}
} // namespace zs.
