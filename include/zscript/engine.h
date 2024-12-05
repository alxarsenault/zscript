#pragma once

#include <zscript/common.h>
#include <zscript/types.h>
#include <unordered_set>

namespace zs {

class garbage_collector_rc_proxy;

class garbage_collector : zs::engine_holder {
  friend class zs::engine;
  friend class zs::garbage_collector_rc_proxy;
  using unordered_set_type = zs::unordered_set<zs::reference_counted_object*>;

  garbage_collector(zs::engine* eng);

  static void add(zs::engine* eng, zs::reference_counted_object* obj);
  static void remove(zs::engine* eng, zs::reference_counted_object* obj);

  void finalize();

  zb::aligned_type_storage<unordered_set_type> _objs;
};

class garbage_collector_rc_proxy {
  friend class zs::reference_counted_object;
  ZS_INLINE static void add(zs::engine* eng, zs::reference_counted_object* obj) {
    garbage_collector::add(eng, obj);
  }

  ZS_INLINE static void remove(zs::engine* eng, zs::reference_counted_object* obj) {
    garbage_collector::remove(eng, obj);
  }
};

class engine_rc_proxy;

/// @class engine
class engine final {
public:
  ZS_CLASS_COMMON;

  engine(allocate_t alloc_cb = ZS_DEFAULT_ALLOCATE, raw_pointer_t user_pointer = nullptr,
      raw_pointer_release_hook_t user_release = nullptr,
      stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER,
      engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER);

  ZS_INLINE engine(const config_t& conf)
      : engine(conf.alloc_callback, conf.user_pointer, conf.user_release, conf.stream_getter,
            conf.initializer) {}

  engine(const engine&) = delete;
  engine(engine&&) = delete;

  ~engine();

  engine& operator=(const engine&) = delete;
  engine& operator=(engine&&) = delete;

  ZS_CHECK void* allocate(size_t size, alloc_info_t ainfo);
  ZS_CHECK void* reallocate(void* ptr, size_t size, size_t old_size, alloc_info_t ainfo);
  void deallocate(void* ptr, alloc_info_t ainfo);

  ZS_CK_INLINE allocate_t get_allocate_callback() const noexcept { return _allocator; }

  void set_user_pointer(raw_pointer_t uptr);

  ZS_CK_INLINE raw_pointer_t get_user_pointer() const noexcept { return _user_pointer; }

  ZS_CK_INLINE raw_pointer_release_hook_t get_user_pointer_release_hook() const noexcept {
    return _user_pointer_release;
  }

  void set_stream_getter(stream_getter_t stream_getter);

  std::ostream& get_stream();

  zs::error_result add_import_directory(const std::filesystem::path& directory);

  ZS_CHECK zs::error_result resolve_file_path(std::string_view import_value, object& result);

  ZS_CHECK object& get_registry_table() noexcept;
  ZS_CHECK const object& get_registry_table() const noexcept;

  ZS_CHECK table_object& get_registry_table_object() noexcept;

  ZS_CHECK object get_registry_object(const zs::object& name) const noexcept;
  ZS_CHECK object get_registry_object(std::string_view name) const noexcept;

private:
  allocate_t _allocator;
  raw_pointer_t _user_pointer;
  raw_pointer_release_hook_t _user_pointer_release;
  stream_getter_t _stream_getter;
  engine_initializer_t _initializer;
  std::array<uint8_t, 2 * constants::k_object_size> _objects;

  friend class engine_rc_proxy;
  friend class zs::garbage_collector;
  ZS_IF_GARBAGE_COLLECTOR(zs::garbage_collector _gc);

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(int_t _global_ref_count = 0);
};

class engine_rc_proxy {
  friend class zs::reference_counted;
  friend class zs::reference_counted_object;

#if ZS_USE_ENGINE_GLOBAL_REF_COUNT
  static inline void incr_global_ref_count(engine* eng) { eng->_global_ref_count++; }
  static inline void decr_global_ref_count(engine* eng) { eng->_global_ref_count--; }
#endif // ZS_USE_ENGINE_GLOBAL_REF_COUNT.
};

} // namespace zs.
