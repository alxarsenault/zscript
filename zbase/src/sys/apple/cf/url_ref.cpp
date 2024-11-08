#include <zbase/sys/apple/cf/url_ref.h>

#if __ZBASE_APPLE__

#include <CoreFoundation/CoreFoundation.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)

url_ref::url_ref(const char* str, bool is_dir) noexcept
    : cf::pointer<CFURLRef>(CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, (const UInt8*)str, strlen(str), is_dir)) {}

url_ref::url_ref(std::string_view str, bool is_dir) noexcept
    : cf::pointer<CFURLRef>(CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, (const UInt8*)str.data(), str.size(), is_dir)) {}

url_ref::url_ref(const std::string& str, bool is_dir) noexcept
    : cf::pointer<CFURLRef>(CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, (const UInt8*)str.c_str(), str.size(), is_dir)) {}

url_ref::url_ref(const cf::string_ref& str_ref) noexcept
    : cf::pointer<CFURLRef>(CFURLCreateWithString(kCFAllocatorDefault, str_ref.get(), nullptr)) {}

url_ref::url_ref(const cf::string_ref& str_ref, const cf::url_ref& base) noexcept
    : cf::pointer<CFURLRef>(CFURLCreateWithString(kCFAllocatorDefault, str_ref.get(), base.get())) {}

url_ref::url_ref(const char* str, bool is_dir, const cf::url_ref& base) noexcept
    : cf::pointer<CFURLRef>(CFURLCreateFromFileSystemRepresentationRelativeToBase(
        kCFAllocatorDefault, (const UInt8*)str, strlen(str), is_dir, base.get())) {}

cf::string_ref url_ref::to_string_ref() const { return cf::string_ref(CFURLGetString(this->get()), true); }

std::string url_ref::to_string() const { return this->to_string_ref(); }

url_ref url_ref::get_base() const { return url_ref(CFURLGetBaseURL(this->get()), true); }

bool url_ref::has_directory() const noexcept { return CFURLHasDirectoryPath(this->get()); }

ZBASE_END_SUB_NAMESPACE(apple, cf)

#endif // __ZBASE_APPLE__
