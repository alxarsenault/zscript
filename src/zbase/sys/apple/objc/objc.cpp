#include <zscript/base/sys/apple/objc/objc.h>

#if __ZBASE_APPLE__

#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <CoreFoundation/CoreFoundation.h>
#include <random>
#include <assert.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, objc)

function send_fct = &objc_msgSend;
function send_super_fct = &objc_msgSendSuper;

proto_t* get_protocol(const char* name) { return objc_getProtocol(name); }

bool add_protocol(class_t* c, proto_t* protocol) { return class_addProtocol(c, protocol); }

void register_protocol(proto_t* protocol) { objc_registerProtocol(protocol); }

proto_t* allocate_protocol(const char* name) { return objc_allocateProtocol(name); }

class_t* allocate_class(class_t* super, const char* name) { return objc_allocateClassPair(super, name, 0); }

void register_class(class_t* c) { objc_registerClassPair(c); }

void dispose_class(class_t* c) { objc_disposeClassPair(c); }

const char* get_class_name(class_t* c) { return class_getName(c); }

bool responds_to_selector(class_t* c, selector_t* sel) { return class_respondsToSelector(c, sel); }

bool conforms_to_protocol(class_t* c, proto_t* protocol) { return class_conformsToProtocol(c, protocol); }

obj_t* create_class_instance(class_t* c) { return class_createInstance(c, 0); }

obj_t* create_class_instance(class_t* c, size_t extraBytes) { return class_createInstance(c, extraBytes); }

obj_t* create_class_instance(const char* name) { return class_createInstance(get_class(name), 0); }

size_t get_class_instance_size(class_t* c) { return class_getInstanceSize(c); }

bool add_class_pointer(class_t* c, const char* name, const char* className, size_t size, size_t align) {
  std::string enc = "^{" + std::string(className) + "=}";
  return class_addIvar(c, name, size, align, enc.c_str());
}

bool add_class_variable(class_t* c, const char* name, const char* encoding, size_t size, size_t align) {
  return class_addIvar(c, name, size, align, encoding);
}

class_t* get_class(const char* name) { return objc_getClass(name); }

class_t* get_class(const obj_t* obj) { return object_getClass((obj_t*)obj); }

bool is_class(const obj_t* obj) { return object_isClass((obj_t*)obj); }

class_t* get_super_class(class_t* cls) { return class_getSuperclass(cls); }

class_t* get_super_class(const obj_t* obj) {
  if (is_class(obj)) {
    return class_getSuperclass((class_t*)obj);
  }

  return class_getSuperclass(object_getClass((obj_t*)obj));
}

class_t* get_meta_class(const char* name) { return objc_getMetaClass(name); }

selector_t* get_selector(const char* name) { return sel_registerName(name); }

function get_class_method_implementation(class_t* c, selector_t* s) {
  return class_getMethodImplementation(c, s);
}

method_t* get_class_method(const char* class_name, selector_t* sel) {
  return class_getClassMethod(get_class(class_name), sel);
}

method_t* get_class_method(objc::class_t* c, selector_t* sel) { return class_getClassMethod(c, sel); }

function get_method_implementation(method_t* method) { return method_getImplementation(method); }

bool add_class_method(class_t* c, selector_t* s, function imp, const char* types) {
  return class_addMethod(c, s, imp, types);
}

class_t* get_obj_class(const obj_t* obj) { return object_getClass((obj_t*)obj); }

void set_obj_pointer_variable(obj_t* obj, const char* name, void* value) {
  object_setInstanceVariable(obj, name, value);
}

void* get_obj_pointer_variable(const obj_t* obj, const char* name) {
  void* v = nullptr;
  object_getInstanceVariable((obj_t*)obj, name, &v);
  return v;
}

// https://gist.github.com/mikeash/7603035
void* get_obj_instance_variable(const obj_t* obj, const char* name) {
  class_t* c = get_obj_class(obj);
  assert(c);
  if (!c) {
    return nullptr;
  }

  ivar_t* iv = class_getInstanceVariable(c, name);
  assert(iv);
  if (!iv) {
    return nullptr;
  }

  return (obj_t*)obj + ivar_getOffset(iv);
}

void set_obj_instance_variable(obj_t* obj, const char* name, const void* data, size_t size) {
  void* buffer = get_obj_instance_variable(obj, name);
  assert(buffer);

  if (!buffer) {
    return;
  }

  ::memcpy(buffer, (const void*)data, size);
}

void* get_obj_indexed_variables(const obj_t* obj) { return object_getIndexedIvars((obj_t*)obj); }

std::string generate_random_alphanum_string(size_t length) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";

  static const int alphanumSize = sizeof(alphanum);

  std::string str;
  str.resize(length, 0);

  std::mt19937 rng;
  std::uniform_int_distribution<int> dist(0, alphanumSize - 1);

  for (size_t i = 0; i < str.size(); i++) {
    str[i] = alphanum[dist(rng)];
  }

  return str;
}

obj_t* get_associated_object(obj_t* obj, const void* key) { return objc_getAssociatedObject(obj, key); }

void set_associated_object(obj_t* obj, const void* key, obj_t* value, association_policy policy) {
  objc_setAssociatedObject(obj, key, value, (objc_AssociationPolicy)policy);
}

autoreleasepool::autoreleasepool()
    : m_pool(objc::msg_send<id>(objc_getClass("NSAutoreleasePool"), sel_registerName("new"))) {}

autoreleasepool::~autoreleasepool() {
  if (m_pool) {
    objc::msg_send<void>(m_pool, sel_registerName("drain"));
  }
}

obj_t* autoreleased(obj_t* object) {
  objc::msg_send<void>(object, sel_registerName("autorelease"));
  return object;
}

namespace detail {

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
} // namespace detail

exception::exception(obj_t* ns_exception) {
  if (!ns_exception) {
    _info = "name: unknown reason: unknown";
    return;
  }

  if (CFStringRef name = objc::call<CFStringRef>(ns_exception, get_selector("name"))) {
    _name = detail::cf_string_to_std_string(name);
  }

  if (CFStringRef reason = objc::call<CFStringRef>(ns_exception, get_selector("reason"))) {
    _reason = detail::cf_string_to_std_string(reason);
  }

  _info = "name: " + _name + " reason: " + _reason;
}

const char* exception::what() const noexcept { return _info.c_str(); }

ZBASE_END_SUB_NAMESPACE(apple, objc)

#endif // __ZBASE_APPLE__
