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
    internal::zs_delete(_engine, this);
    return true;
  }

  return false;
}

//
// MARK: reference_counted_object
//
reference_counted_object::reference_counted_object(zs::engine* eng, zs::object_type objtype) noexcept
    : reference_counted(eng)
    , _obj_type(objtype) {
  ZS_IF_GARBAGE_COLLECTOR(zs::garbage_collector_rc_proxy::add(eng, this));
}

reference_counted_object::~reference_counted_object() {

  if (_weak_ref_object) {
    //    zbase_assert(_weak_ref_object->_obj._ref_counted != this,
    //    "reference_counted_object referencing itself.");

    // Reset weak reference.
    ::memset(&_weak_ref_object->_obj, 0, sizeof(object));
    _weak_ref_object->_obj._type = object_type::k_null;

    _weak_ref_object->release();
  }
}

bool reference_counted_object::release() noexcept {
  zbase_assert(_ref_count > 0, "invalid ref count");

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(engine_rc_proxy::decr_global_ref_count(_engine));

  if (--_ref_count == 0) {
    ZS_IF_GARBAGE_COLLECTOR(zs::garbage_collector_rc_proxy::remove(_engine, this));
    internal::zs_delete(_engine, this);

    return true;
  }

  return false;
}

object reference_counted_object::get_weak_ref(const object_base& obj) {
  if (!_weak_ref_object) {
    // Create the weak_ref object if it doesn't exists.
    _weak_ref_object = weak_ref_object::create(_engine, obj);
  }

  zbase_assert(_weak_ref_object, "Invalid weak_ref_object");
  return object(_weak_ref_object, true);
}
} // namespace zs.
