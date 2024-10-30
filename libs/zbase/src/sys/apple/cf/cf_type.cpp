#include <zbase/sys/apple/cf/type.h>

#if __ZBASE_APPLE__

#include <CoreFoundation/CoreFoundation.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)
void retain(const void* obj) noexcept { CFRetain(obj); }

void release(const void* obj) noexcept { CFRelease(obj); }

size_t ref_count(const void* obj) noexcept { return CFGetRetainCount(obj); }

std::string cf_string_to_std_string(CFStringRef s) {
  // Check if we can get a C string pointer directly from the CFString.
  if (const char* cstr = CFStringGetCStringPtr(s, kCFStringEncodingUTF8)) {
    // If available, directly create a std::string from the C string.
    return std::string(cstr);
  }

  // If the direct C string pointer is not available, determine the length
  // needed.
  CFIndex len = 0;
  CFRange range = CFRangeMake(0, CFStringGetLength(s));
  CFStringGetBytes(s, range, kCFStringEncodingUTF8, '?', false, nullptr, 0, &len);

  if (len <= 0) {
    return std::string();
  }

  // Resize the std::string to the required length.
  std::string outputStr(len, 0);

  // Fill the std::string with the bytes from the CFString.
  CFStringGetBytes(s, range, kCFStringEncodingUTF8, '?', false, (UInt8*)outputStr.data(), len, &len);

  // Adjust the size of the std::string to match the actual length.
  // This should normally never happen.
  if (len != (CFIndex)outputStr.size()) {
    outputStr.resize(len);
  }

  // Return the converted std::string.
  return outputStr;
}

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__
