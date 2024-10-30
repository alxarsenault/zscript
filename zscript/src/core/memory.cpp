#include <zscript/core/zcore.h>

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
// MARK: reference_counted_object
//

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

object reference_counted_object::get_weak_ref(const object_base& obj) {
  if (!_weak_ref_object) {
    // Create the weak_ref object if it doesn't exists.
    _weak_ref_object = weak_ref_object::create(_engine, obj);
  }

  zbase_assert(_weak_ref_object, "Invalid weak_ref_object");
  return object(_weak_ref_object, object_type::k_weak_ref, true);
}
} // namespace zs.
