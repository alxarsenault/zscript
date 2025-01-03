#include <zscript/base/sys/apple/cf/string_ref.h>

#if __ZBASE_APPLE__

#include <CoreFoundation/CoreFoundation.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)
string_ref::string_ref(const char* str) noexcept
    : cf::pointer<CFStringRef>(CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingUTF8)) {}

string_ref::string_ref(std::string_view str) noexcept
    : cf::pointer<CFStringRef>(
          CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(str.data()),
              static_cast<CFIndex>(str.size()), kCFStringEncodingUTF8, false)) {}

string_ref::string_ref(const std::string& str) noexcept
    : cf::pointer<CFStringRef>(
          CFStringCreateWithCString(kCFAllocatorDefault, str.c_str(), kCFStringEncodingUTF8)) {}

std::string string_ref::to_string() const { return cf::cf_string_to_std_string(this->get()); }

size_t string_ref::size() const noexcept { return CFStringGetLength(this->get()); }

uint16_t string_ref::get_char(size_t index) const noexcept {
  return CFStringGetCharacterAtIndex(this->get(), index);
}

ZBASE_END_SUB_NAMESPACE(apple, cf)

#endif // __ZBASE_APPLE__
