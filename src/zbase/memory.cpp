#include <zscript/base/memory/memory.h>
#include <zscript/base/sys/assert.h>

ZBASE_BEGIN_NAMESPACE

void* aligned_allocate(size_t size, size_t alignment) noexcept {

  if (alignment < alignof(void*)) {
    alignment = alignof(void*);
  }
  size += size & (alignment - 1);
  zbase_assert((!(size & (alignment - 1))) && "alignment must be a multiple of sizeof(void*)");

#if __ZBASE_WINDOWS__
  void* ptr = ::_aligned_malloc(size, alignment);
#else
  void* ptr = ::aligned_alloc(alignment, size);
#endif
  return ptr;
}

void aligned_deallocate(void* ptr) noexcept {
#if __ZBASE_WINDOWS__
  ::_aligned_free(ptr);
#else
  ::free(ptr);
#endif
}

ZBASE_END_NAMESPACE
