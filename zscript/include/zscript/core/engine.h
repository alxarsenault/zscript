#pragma once

#include <zscript/core/common.h>
#include <zscript/core/types.h>

namespace zs {

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

private:
  allocate_t _allocator;
  raw_pointer_t _user_pointer;
  raw_pointer_release_hook_t _user_pointer_release;
  stream_getter_t _stream_getter;
  engine_initializer_t _initializer;
  std::array<uint8_t, 4 * constants::k_object_size> _objects;
  int_t _global_ref_count = 0;
};
} // namespace zs.
