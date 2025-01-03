#include <zscript/zscript.h>

namespace zs {

const allocate_t default_allocate = [](zs::engine* eng, raw_pointer_t user_ptr, void* ptr, size_t size,
                                        size_t old_size, alloc_info_t info) -> void* {
  if (!size) {
    zbase_assert(ptr, "invalid pointer");
    ::free(ptr);
    return nullptr;
  }

  if (ptr) {
    return ::realloc(ptr, size);
  }

  zbase_assert(!ptr, "pointer should be nullptr");
  return ::malloc(size);
};

const stream_getter_t default_stream_getter
    = [](zs::engine* eng, raw_pointer_t user_ptr) -> std::ostream& { return std::cout; };

struct engine_initializer_tag {};
template <>
struct internal::proxy<engine_initializer_tag> {
  static zs::error_result init(zs::engine* eng) { return {}; }
};

const engine_initializer_t default_engine_initializer
    = [](zs::engine* eng) -> zs::error_result { return internal::proxy<engine_initializer_tag>::init(eng); };

void* allocate(zs::engine* eng, size_t sz, alloc_info_t ainfo) { return eng->allocate(sz, ainfo); }

void* reallocate(zs::engine* eng, void* ptr, size_t size, size_t old_size, alloc_info_t ainfo) {
  return eng->reallocate(ptr, size, old_size, ainfo);
}

void deallocate(zs::engine* eng, void* ptr, alloc_info_t ainfo) { eng->deallocate(ptr, ainfo); }

//
// MARK: reference_counted
//

reference_counted::reference_counted(zs::engine* eng) noexcept
    : engine_holder(eng) {

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::incr_global_ref_count(eng));
}

reference_counted::~reference_counted() {
  zbase_assert(_ref_count == 0, "~reference_counted: ref_count should be zero");
}

void reference_counted::retain() noexcept {
  _ref_count++;
  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::incr_global_ref_count(_engine));
}

bool reference_counted::release() noexcept {
  zbase_assert(_ref_count > 0, "invalid ref count");

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::decr_global_ref_count(_engine));

  if (--_ref_count == 0) {
    zs_delete(_engine, this);
    return true;
  }

  return false;
}

//
// MARK: reference_counted_object
//

namespace {

  struct reference_counted_object_proxy {};

} // namespace.

template <>
struct internal::proxy<reference_counted_object_proxy> {
  static constexpr const std::array<void (*)(zs::engine*, reference_counted_object*) noexcept,
      1 + (int)object_type::k_user_data - (int)object_type::k_long_string>
      s_reference_counted_object_destroy_array = { zs::string_object::destroy_callback,
        zs::closure_object::destroy_callback, zs::native_closure_object::destroy_callback,
        zs::weak_ref_object::destroy_callback, zs::struct_object::destroy_callback,
        zs::struct_instance_object::destroy_callback, zs::table_object::destroy_callback,
        zs::array_object::destroy_callback, zs::user_data_object::destroy_callback };

  static constexpr const std::array<object (*)(zs::engine*, const reference_counted_object*) noexcept,
      1 + (int)object_type::k_user_data - (int)object_type::k_long_string>
      s_reference_counted_object_clone_array = { zs::string_object::clone_callback,
        zs::closure_object::clone_callback, zs::native_closure_object::clone_callback,
        zs::weak_ref_object::clone_callback, zs::struct_object::clone_callback,
        zs::struct_instance_object::clone_callback, zs::table_object::clone_callback,
        zs::array_object::clone_callback, zs::user_data_object::clone_callback };

  static void destroy(zs::engine* eng, reference_counted_object* obj, object_type otype) {
    auto cb = s_reference_counted_object_destroy_array[(int)otype - (int)object_type::k_long_string];
    cb(eng, obj);
  }

  static object clone(zs::engine* eng, const reference_counted_object* obj, object_type otype) {
    auto cb = s_reference_counted_object_clone_array[(int)otype - (int)object_type::k_long_string];
    return cb(eng, obj);
  }
};

namespace {

  using ref_proxy = internal::proxy<reference_counted_object_proxy>;

} // namespace.

reference_counted_object::reference_counted_object(zs::engine* eng, object_type obj_type) noexcept
    : engine_holder(eng)
    , _ref_count(1)
    , _obj_type(obj_type) {

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::incr_global_ref_count(eng));

  ZS_IF_GARBAGE_COLLECTOR(zs::garbage_collector_rc_proxy::add(eng, this));
}

void reference_counted_object::retain() noexcept {
  _ref_count++;
  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::incr_global_ref_count(get_engine()));
}

bool reference_counted_object::release() noexcept {

  ZS_ASSERT(_ref_count > 0, "invalid ref count");

  zs::engine* eng = get_engine();
  ZS_ASSERT(eng);

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::decr_global_ref_count(eng));

  if (--_ref_count == 0) {
    ZS_IF_GARBAGE_COLLECTOR(zs::garbage_collector_rc_proxy::remove(eng, this));

    if (_weak_ref_object) {
      // Reset weak reference.
      _weak_ref_object->_obj = object();
      _weak_ref_object->release();
      _weak_ref_object = nullptr;
    }

    ref_proxy::destroy(eng, this, _obj_type);

    return true;
  }

  return false;
}

object reference_counted_object::clone() const noexcept {
  return ref_proxy::clone(get_engine(), this, _obj_type);
}

object reference_counted_object::get_weak_ref(const object_base& obj) noexcept {
  if (!_weak_ref_object) {
    // Create the weak_ref object if it doesn't exists.
    _weak_ref_object = weak_ref_object::create(get_engine(), obj);
  }

  zbase_assert(_weak_ref_object, "Invalid weak_ref_object");
  return object(_weak_ref_object, true);
}
} // namespace zs.
