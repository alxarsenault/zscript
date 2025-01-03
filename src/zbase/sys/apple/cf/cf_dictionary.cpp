#include <zscript/base/sys/apple/cf/dictionary.h>

#if __ZBASE_APPLE__

#include <CoreFoundation/CoreFoundation.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)

cf::pointer<CFDictionaryRef> create_dictionary(const void** keys, const void** values, size_t size) noexcept {
  return cf::pointer<CFDictionaryRef>(CFDictionaryCreate(kCFAllocatorDefault, keys, values, size,
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
}

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__
