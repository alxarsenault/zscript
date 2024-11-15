/*
 * MIT License
 *
 * Copyright (c) 2024 Alexandre Arsenault
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/**
 * @file      objc++/objc.h
 * @brief     objc
 * @version   1.0.0
 * @copyright Copyright (C) 2024, Meta-Sonic
 * @author    Alexandre Arsenault \b alx.arsenault@gmail.com
 * @date      Created 16/06/2024
 */

#include <zbase/zbase.h>

#if __ZBASE_APPLE__

#include <zbase/sys/assert.h>
#include <zbase/memory/reference_counted_ptr.h>
#include <iostream>
#include <string>
#include <string_view>
#include <exception>

struct objc_class;
struct objc_object;
struct objc_selector;
struct objc_super;
struct objc_ivar;
struct objc_method;

ZBASE_BEGIN_SUB_NAMESPACE(apple, objc)

//
// MARK: Types
//

using class_t = objc_class;
using obj_t = objc_object;
using proto_t = objc_object;
using selector_t = objc_selector;
using method_t = objc_method;
using ivar_t = objc_ivar;

struct super_t {
  /// Specifies an instance of a class.
  obj_t* receiver;

  /// Specifies the particular superclass of the instance to message.
  class_t* super_class;

  /* super_class is the first class to search */
};

using function = void (*)();
extern function send_fct;
extern function send_super_fct;

using int_t = long;
using uint_t = unsigned long;

class exception;

//
//
//

inline void retain(obj_t* obj);

inline void release(obj_t* obj);

inline void reset(obj_t*& obj);

inline uint_t retain_count(obj_t* obj);

namespace detail {
struct obj_reference_counted_handler {
  using pointer = obj_t*;

  static inline void retain(pointer ptr) noexcept { objc::retain(ptr); }

  static inline void release(pointer ptr) noexcept { objc::release(ptr); }
};
} // namespace detail.

using obj_pointer = __zb::reference_counted_ptr<obj_t, detail::obj_reference_counted_handler>;

/// Returns the class definition of a specified class.
///
/// @param name The name of the class to look up.
///
/// @return The Class object for the named class, or \c nullptr if the class is
///         not registered with the Objective-C runtime.
class_t* get_class(const char* name);

class_t* get_class(const obj_t* obj);

bool is_class(const obj_t* obj);

class_t* get_super_class(class_t* cls);

class_t* get_super_class(const obj_t* obj);

inline class_t* get_super_class(const char* name) { return get_super_class(get_class(name)); }

///
/// Returns the metaclass definition of a specified class.
///
/// @param name The name of the class to look up.
///
/// @return The \c class_t* object for the metaclass of the named class, or
///         \c nullptr if the class is not registered with the Objective-C
///         runtime.
///
/// @note If the definition for the named class is not registered, this
///       function calls the class handler callback and then checks a second
///       time to see if the class is registered. However, every class
///       definition must have a valid metaclass definition, and so the
///       metaclass definition is always returned, whether it’s valid or not.
class_t* get_meta_class(const char* name);

/// Returns the name of a class.
///
/// @param cls A class object.
///
/// @return The name of the class, or the empty string if \c cls is \c nullptr.
const char* get_class_name(class_t* cls);

///
function get_class_method_implementation(class_t* c, selector_t* s);

bool add_class_method(class_t* c, selector_t* s, function imp, const char* types);

bool add_class_pointer(class_t* c, const char* name, const char* className, size_t size, size_t align);

bool add_class_variable(class_t* c, const char* name, const char* encoding, size_t size, size_t align);

template <typename T>
inline bool add_class_variable(class_t* c, const char* name, const char* encoding);

//
// MARK: Protocol
//

proto_t* get_protocol(const char* name);

bool add_protocol(class_t* c, proto_t* protocol);

proto_t* allocate_protocol(const char* name);

void register_protocol(proto_t* protocol);

selector_t* get_selector(const char* name);

bool responds_to_selector(class_t* c, selector_t* sel);

bool conforms_to_protocol(class_t* c, proto_t* protocol);

//
//
//

obj_t* create_class_instance(class_t* c);
obj_t* create_class_instance(class_t* c, size_t extraBytes);
obj_t* create_class_instance(const char* name);

size_t get_class_instance_size(class_t* c);

class_t* get_obj_class(const obj_t* obj);

method_t* get_class_method(objc::class_t* c, selector_t* sel);

method_t* get_class_method(const char* class_name, selector_t* sel);

inline obj_t* get_class_property(const char* className, const char* propertyName);

void set_obj_pointer_variable(obj_t* obj, const char* name, void* value);

void* get_obj_pointer_variable(const obj_t* obj, const char* name);

void set_obj_instance_variable(obj_t* obj, const char* name, const void* data, size_t size);

void* get_obj_instance_variable(const obj_t* obj, const char* name);

template <typename T>
inline void set_obj_instance_variable(obj_t* obj, const char* name, const T& data);

template <typename T>
inline T* get_obj_instance_variable(const obj_t* obj, const char* name);

void* get_obj_indexed_variables(const obj_t* obj);

template <typename Type>
inline void set_ivar_pointer(obj_t* obj, const char* name, Type* value);

template <typename Type, std::enable_if_t<std::is_pointer_v<Type>, std::nullptr_t> = nullptr>
inline Type get_ivar_pointer(obj_t* obj, const char* name);

//
// MARK: Class declaration
//

///
class_t* allocate_class(class_t* super, const char* name);

///
void register_class(class_t* c);

///
void dispose_class(class_t* c);

template <class ClassType>
inline obj_t* create_object(ClassType classType);

template <class... Args, class... Params, class ClassType, class SelectorType>
inline obj_t* create_object(ClassType classType, SelectorType selector, Params&&... params);

///
function get_method_implementation(method_t* method);

//
//
//

template <class _Tp>
using null_to_obj = std::conditional_t<std::is_null_pointer_v<_Tp>, obj_t*, _Tp>;

template <typename R = void, typename... Args>
using method_ptr = R (*)(const obj_t*, selector_t*, Args...);

template <typename R = void, typename... Args>
using class_method_ptr = R (*)(class_t*, selector_t*, Args...);

template <typename R = void, typename... Args>
using super_method_ptr = R (*)(super_t*, selector_t*, Args...);

//
//
//

template <class R = void, class... Params, class SelectorType, class ObjectType>
inline R call(const ObjectType* optr, SelectorType selector, Params... params);

template <class R = void, class... Params, class SelectorType>
inline R call(const obj_t* obj, SelectorType selector, Params... params);

// template <typename R = void, typename... Args, typename... Params, typename SelectorType, typename IdType>
// inline R s_call(IdType* optr, SelectorType selector, Params&&... params);
//
// template <typename SelectorType, typename IdType, typename... Params>
// class r_calll;

// template <typename IdType, typename... ObjType, typename SelectorType>
// inline void icall(IdType* optr, SelectorType selector, ObjType... obj_type_ptr);

template <class R = void, class... Args, class... Params>
inline R call_meta(const char* className, const char* selectorName, Params&&... params);

///
template <typename Descriptor>
class class_descriptor {
public:
  class_descriptor(const char* rootName);

  class_descriptor(const class_descriptor&) = delete;

  ~class_descriptor();

  class_descriptor& operator=(const class_descriptor&) = delete;

  obj_t* create_instance() const;
  obj_t* create_instance(size_t extraBytes) const;

  template <typename ReturnType, typename... Args, typename... Params>
  static ReturnType send_superclass_message(obj_t* obj, const char* selectorName, Params&&... params);

  template <typename Type>
  inline bool add_pointer(const char* name, const char* className);

  template <auto FunctionType>
  inline bool add_method(const char* selectorName, const char* signature);

  template <void (Descriptor::*MemberFunctionPointer)(obj_t*)>
  bool add_notification_method(const char* selectorName);

  inline bool add_protocol(const char* protocolName, bool force = false);

  inline bool register_class();

  inline class_t* get_class_object() const noexcept { return m_classObject; }

private:
  class_t* m_classObject;

  template <auto FunctionType, typename ReturnType, typename... Args>
  inline bool add_member_method_impl(
      ReturnType (Descriptor::*)(Args...), selector_t* selector, const char* signature);
};

template <typename ReturnType>
inline ReturnType return_default_value() {
  return ReturnType{};
}

template <>
inline void return_default_value<void>() {}

// template <class R, class... Params, class SelectorType, class IdType>
// R call(IdType* optr, SelectorType selector, Params... params) {
//
//   static_assert(
//       std::is_same_v<SelectorType, selector_t*> || std::is_constructible_v<std::string_view, SelectorType>,
//       "");
//
//   const obj_t* obj = reinterpret_cast<const obj_t*>(optr);
//
//   selector_t* sel = [](SelectorType s) {
//     if constexpr (std::is_same_v<SelectorType, selector_t*>) {
//       return s;
//     }
//     else {
//       return get_selector(s);
//     }
//   }(std::forward<SelectorType>(selector));
//
//   function fctImpl = get_class_method_implementation(get_obj_class(obj), sel);
//   return reinterpret_cast<method_ptr<R, null_to_obj<Params>...>>(fctImpl)(obj, sel, params...);
// }

namespace detail {
template <class ClassType>
inline class_t* get_class_from_class_or_string(ClassType c) noexcept {
  static_assert(
      std::is_same_v<ClassType, class_t*> || std::is_constructible_v<std::string_view, ClassType>, "");

  if constexpr (std::is_same_v<ClassType, class_t*>) {
    return c;
  }
  else {
    return get_class(c);
  }
}

// template <class ClassType>
// inline class_t* get_class_from_class_obj_or_string(ClassType c) noexcept{
//   static_assert(
//                 std::is_same_v<ClassType, class_t*> || std::is_same_v<ClassType, obj_t*> ||
//                 std::is_constructible_v<std::string_view, ClassType>,
//       "");
//
//   if constexpr (std::is_same_v<ClassType, class_t*>) {
//     return c;
//   }
//   else if constexpr (std::is_same_v<ClassType, obj*>) {
//     return get_class(c);
//   }
//   else {
//     return get_class(c);
//   }
// }

template <class SelectorType>
inline selector_t* get_selector_from_selector_or_string(SelectorType s) noexcept {
  static_assert(
      std::is_same_v<SelectorType, selector_t*> || std::is_constructible_v<std::string_view, SelectorType>,
      "");

  if constexpr (std::is_same_v<SelectorType, selector_t*>) {
    return s;
  }
  else {
    return get_selector(s);
  }
}

template <class ObjectType>
inline const obj_t* get_obj_from_obj_or_string(const ObjectType* o) noexcept {
  if constexpr (std::is_constructible_v<std::string_view, const ObjectType*>) {
    return (const obj_t*)get_class(o);
  }
  else {
    return (const obj_t*)o;
  }
}
} // namespace detail.

//
// MARK: Traits
//

template <typename T>
constexpr bool is_basic_type = std::is_fundamental_v<T> //
    || std::is_fundamental_v<std::remove_pointer_t<T>> //
    || std::is_same_v<T, obj_t*> //
    || std::is_same_v<T, selector_t*> //
    || std::is_same_v<T, class_t*>;

template <typename T>
struct name_for_type {
  static constexpr bool value = false;
};

template <typename T>
constexpr bool has_name_for_type = !std::is_same_v< //
    std::remove_cv_t<std::remove_reference_t<decltype(name_for_type<T>::value)> //
        >,
    bool>;

template <typename T, typename... Ts, std::enable_if_t<is_basic_type<T>, std::nullptr_t> = nullptr>
inline std::string get_encoding();

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_pointer_v<T>, std::nullptr_t> = nullptr>
inline std::string get_encoding(const char* name);

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_pointer_v<T> && has_name_for_type<std::remove_pointer_t<T>>,
        std::nullptr_t>
    = nullptr>
inline std::string get_encoding();

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_class_v<T> && std::is_trivial_v<T>, std::nullptr_t>
    = nullptr>
inline std::string get_encoding(const char* name);

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_class_v<T> && std::is_trivial_v<T> && has_name_for_type<T>,
        std::nullptr_t>
    = nullptr>
inline std::string get_encoding();

template <typename T>
bool add_class_variable(class_t* c, const char* name, const char* encoding) {
  return add_class_variable(c, name, encoding, sizeof(T), alignof(T));
}

template <typename T>
void set_obj_instance_variable(obj_t* obj, const char* name, const T& data) {
  set_obj_instance_variable(obj, name, static_cast<const void*>(&data), sizeof(T));
}

template <typename T>
T* get_obj_instance_variable(const obj_t* obj, const char* name) {
  return static_cast<T*>(get_obj_instance_variable(obj, name));
}

// template <class R, class... Params, class SelectorType, class IdType>
// R call(IdType* optr, SelectorType selector, Params... params) {
//   const obj_t* obj = reinterpret_cast<const obj_t*>(optr);
//   selector_t* sel = detail::get_selector_from_selector_or_string(std::forward<SelectorType>(selector));
//   function fctImpl = get_class_method_implementation(get_obj_class(obj), sel);
//   return reinterpret_cast<method_ptr<R, null_to_obj<Params>...>>(fctImpl)(obj, sel, params...);
// }

// template <class SelectorType, class IdType, class... Params>
// class r_call {
// public:
//   inline r_call(IdType* optr, SelectorType selector, Params... params)
//       : _params(optr, selector, params...) {}
//
//   template <typename R>
//   inline operator R() const {
//     return std::apply(
//         [](IdType* optr, SelectorType selector, Params... args) {
//           return call<R, null_to_obj<Params>...>(optr, selector, args...);
//         },
//         _params);
//   }
//
// private:
//   std::tuple<IdType*, SelectorType, Params...> _params;
// };

// template <typename SelectorType, typename IdType, typename... Params>
// r_call(IdType* optr, SelectorType selector, Params... params) -> r_call<SelectorType, IdType, Params...>;
//
// template <typename SelectorType, typename... Params>
// r_call(obj_pointer optr, SelectorType selector, Params... params) -> r_call<SelectorType, obj_t,
// Params...>;
//
// template <typename R, typename... Args, typename... Params, typename SelectorType, typename IdType>
// R s_call(IdType* optr, SelectorType selector, Params&&... params) {
//   obj_t* obj = reinterpret_cast<obj_t*>(optr);
//   selector_t* sel = detail::get_selector_from_selector_or_string(std::forward<SelectorType>(selector));
//   function fctImpl = get_class_method_implementation(get_obj_class(obj), sel);
//   return reinterpret_cast<method_ptr<R, Args...>>(fctImpl)(obj, sel, std::forward<Params>(params)...);
// }

template <class R, class... Params, class SelectorType, class ObjectType>
R call(const ObjectType* optr, SelectorType selector, Params... params) {
  //  const obj_t* obj = nullptr;
  //  if constexpr(std::is_constructible_v<std::string_view, const ObjectType*>) {
  //    obj = (const obj_t*)get_class(optr);
  //  }
  //  else {
  //    obj = (const obj_t*)optr;
  //  }

  const obj_t* obj = detail::get_obj_from_obj_or_string(optr);
  return call<R, Params...>(obj, std::forward<SelectorType>(selector), std::forward<Params>(params)...);
}

template <class R, class... Params, class SelectorType>
inline R call(const obj_t* obj, SelectorType selector, Params... params) {
  selector_t* sel = detail::get_selector_from_selector_or_string(std::forward<SelectorType>(selector));
  function fctImpl = get_class_method_implementation(get_obj_class(obj), sel);
  return reinterpret_cast<method_ptr<R, null_to_obj<Params>...>>(fctImpl)(obj, sel, params...);
}

template <typename R, typename... Args, typename... Params>
R call_meta(const char* className, const char* selectorName, Params&&... params) {
  class_t* objClass = get_class(className);
  class_t* meta = get_meta_class(className);
  selector_t* selector = get_selector(selectorName);
  function fctImpl = get_class_method_implementation(meta, selector);
  return reinterpret_cast<class_method_ptr<R, Args...>>(fctImpl)(
      objClass, selector, std::forward<Params>(params)...);
}

template <class R = void, class SelectorType, class... Args, class... Params>
static inline R call_super(objc::obj_t* obj, SelectorType selector, Params&&... params) {

  if (class_t* super_cls = get_super_class(obj)) {
    using FctType = super_method_ptr<R, Args...>;
    super_t s = { obj, super_cls };
    selector_t* sel = detail::get_selector_from_selector_or_string(std::forward<SelectorType>(selector));
    return reinterpret_cast<FctType>(send_super_fct)(&s, sel, std::forward<Params>(params)...);
  }

  zbase_error("Could not create objc class");
  return return_default_value<R>();
}

template <class ClassType>
inline obj_t* create_object(ClassType classType) {
  class_t* cls = detail::get_class_from_class_or_string(std::forward<ClassType>(classType));
  return create_class_instance(cls);
}

template <class... Args, class... Params, class ClassType, class SelectorType>
inline obj_t* create_object(ClassType classType, SelectorType selector, Params&&... params) {
  class_t* cls = detail::get_class_from_class_or_string(std::forward<ClassType>(classType));
  obj_t* class_obj = create_class_instance(cls);

  if constexpr (std::is_same_v<SelectorType, std::nullptr_t>) {
    static_assert(sizeof...(Params) == 0, "invalid call");
    return class_obj;
  }
  else {
    selector_t* sel = detail::get_selector_from_selector_or_string(std::forward<SelectorType>(selector));
    return objc::call<obj_t*, Args...>(class_obj, sel, std::forward<Params>(params)...);
  }
}

///
template <class... Args>
inline obj_t* call_class_type_method(const char* class_name, const char* selector_name, Args&&... args) {
  class_t* c = get_class(class_name);

  selector_t* sel = get_selector(selector_name);
  method_t* method = get_class_method(c, sel);

  using method_type = class_method_ptr<obj_t*, Args...>;
  method_type fct = (method_type)get_method_implementation(method);

  return fct(c, sel, std::forward<Args>(args)...);
}

obj_t* get_class_property(const char* className, const char* propertyName) {
  return call_meta<obj_t*>(className, propertyName);
}

template <typename Type>
void set_ivar_pointer(obj_t* obj, const char* name, Type* value) {
  set_obj_pointer_variable(obj, name, static_cast<void*>(value));
}

template <typename Type, std::enable_if_t<std::is_pointer_v<Type>, std::nullptr_t>>
Type get_ivar_pointer(obj_t* obj, const char* name) {
  return static_cast<Type>(get_obj_pointer_variable(obj, name));
}

void retain(obj_t* obj) { call(obj, "retain"); }

uint_t retain_count(obj_t* obj) { return call<uint_t>(obj, "retainCount"); }

void release(obj_t* obj) { call(obj, "release"); }

void reset(obj_t*& obj) {
  call(obj, "release");
  obj = nullptr;
}

std::string generate_random_alphanum_string(size_t length);

//  NANO_CLANG_PUSH_WARNING("-Wold-style-cast")

template <typename Descriptor>
class_descriptor<Descriptor>::class_descriptor(const char* rootName)
    : m_classObject(allocate_class(
          get_class(Descriptor::baseName), (rootName + generate_random_alphanum_string(10)).c_str())) {

  if (!add_pointer<Descriptor>(Descriptor::valueName, Descriptor::className)) {
    std::cout << "ERROR" << std::endl;
    return;
  }

  /**
   * Creates a new class and metaclass.
   *
   * @param superclass The class to use as the new class's superclass, or \c Nil
   * to create a new root class.
   * @param name The string to use as the new class's name. The string will be
   * copied.
   * @param extraBytes The number of bytes to allocate for indexed ivars at the
   * end of the class and metaclass objects. This should usually be \c 0.
   *
   * @return The new class, or Nil if the class could not be created (for
   * example, the desired name is already in use).
   *
   * @note You can get a pointer to the new metaclass by calling \c
   * object_getClass(newClass).
   * @note To create a new class, start by calling \c objc_allocateClassPair.
   *  Then set the class's attributes with functions like \c class_addMethod and
   * \c class_addIvar. When you are done building the class, call \c
   * objc_registerClassPair. The new class is now ready for use.
   * @note Instance methods and instance variables should be added to the class
   * itself. Class methods should be added to the metaclass.
   */
  //        OBJC_EXPORT Class _Nullable
  //        objc_allocateClassPair(Class _Nullable superclass, const char *
  //        _Nonnull name,
  //                               size_t extraBytes)

  // TODO: Should not be here.
  //  register_class(m_classObject);
}

template <typename Descriptor>
class_descriptor<Descriptor>::~class_descriptor() {
  std::string kvoSubclassName = std::string("NSKVONotifying_") + get_class_name(m_classObject);

  if (get_class(kvoSubclassName.c_str()) == nullptr) {
    dispose_class(m_classObject);
  }
}

template <typename Descriptor>
bool class_descriptor<Descriptor>::register_class() {
  objc::register_class(m_classObject);
  return true;
}

template <typename Descriptor>
obj_t* class_descriptor<Descriptor>::create_instance() const {
  return create_class_instance(m_classObject);
}

template <typename Descriptor>
obj_t* class_descriptor<Descriptor>::create_instance(size_t extraBytes) const {
  return create_class_instance(m_classObject, extraBytes);
}

template <typename Descriptor>
template <typename ReturnType, typename... Args, typename... Params>
ReturnType class_descriptor<Descriptor>::send_superclass_message(
    obj_t* obj, const char* selectorName, Params&&... params) {

  if (class_t* objClass = get_class(Descriptor::baseName)) {
    using FctType = super_method_ptr<ReturnType, Args...>;

    std::pair<obj_t*, class_t*> s = { obj, objClass };
    return reinterpret_cast<FctType>(send_super_fct)(
        (super_t*)&s, get_selector(selectorName), std::forward<Params>(params)...);
  }

  //    assert(false"Could not create objc class");
  return return_default_value<ReturnType>();
}

template <typename Descriptor>
template <typename Type>
inline bool class_descriptor<Descriptor>::add_pointer(const char* name, const char* className) {
  return add_class_pointer(m_classObject, name, className, sizeof(Type*), alignof(Type*));
}

template <typename Descriptor>
template <auto FunctionType>
inline bool class_descriptor<Descriptor>::add_method(const char* selectorName, const char* signature) {
  selector_t* selector = get_selector(selectorName);

  if constexpr (std::is_member_function_pointer_v<decltype(FunctionType)>) {
    return add_member_method_impl<FunctionType>(FunctionType, selector, signature);
  }
  else {
    return add_class_method(m_classObject, selector, (function)FunctionType, signature);
  }
}

template <typename Descriptor>
template <void (Descriptor::*MemberFunctionPointer)(obj_t*)>
bool class_descriptor<Descriptor>::add_notification_method(const char* selectorName) {
  return add_class_method(
      m_classObject, get_selector(selectorName),
      (function)(method_ptr<void, obj_t*>)+[](obj_t* obj, selector_t*, obj_t* notification) {
        if (auto* p = objc::get_ivar_pointer<Descriptor*>(obj, Descriptor::valueName)) {
          (p->*MemberFunctionPointer)(notification);
        }
      },
      "v@:@");
}

template <typename Descriptor>
inline bool class_descriptor<Descriptor>::add_protocol(const char* protocolName, bool force) {

  if (proto_t* protocol = get_protocol(protocolName)) {
    return objc::add_protocol(m_classObject, protocol);
  }

  if (!force) {
    return false;
  }

  // Force protocol allocation.
  if (proto_t* protocol = allocate_protocol(protocolName)) {
    register_protocol(protocol);
    return objc::add_protocol(m_classObject, protocol);
  }

  return false;
}

template <typename Descriptor>
template <auto FunctionType, typename ReturnType, typename... Args>
inline bool class_descriptor<Descriptor>::add_member_method_impl(
    ReturnType (Descriptor::*)(Args...), selector_t* selector, const char* signature) {
  return add_class_method(
      m_classObject, selector,
      (function)(method_ptr<ReturnType, Args...>)+[](obj_t* obj, selector_t*, Args... args) {
        auto* p = objc::get_ivar_pointer<Descriptor*>(obj, Descriptor::valueName);
        return p ? (p->*FunctionType)(args...) : return_default_value<ReturnType>();
      },
      signature);
}

template <typename Result, typename Callable, typename... Args>
inline Result invoke(Callable callable, Args... args) noexcept {
  return reinterpret_cast<Result (*)(Args...)>(callable)(args...);
}

// Calls objc_msgSend.
template <typename Result = void, typename... Args>
inline Result msg_send(Args... args) noexcept {
  return invoke<Result>(send_fct, args...);
}

template <typename Result = void, typename... Args>
inline Result msg_send_super(Args... args) noexcept {
  return invoke<Result>(send_super_fct, args...);
}

enum class association_policy : uintptr_t {
  /// Specifies an unsafe unretained reference to the associated object.
  association_assign = 0,

  /// Specifies a strong reference to the associated object.
  /// The association is not made atomically.
  association_retain_nonatomic = 1,

  /// Specifies that the associated object is copied.
  /// The association is not made atomically.
  association_copy_nonatomic = 3,

  /// Specifies a strong reference to the associated object.
  /// The association is made atomically.
  association_retain = 01401,

  /// Specifies that the associated object is copied.
  /// The association is made atomically.
  association_copy = 01403
};

obj_t* get_associated_object(obj_t* obj, const void* key);
void set_associated_object(obj_t* obj, const void* key, obj_t* value, association_policy policy);

template <class T>
void set_associated_object(obj_t* obj, const void* key, T* value, association_policy policy) {
  return set_associated_object(obj, (const void*)key, (obj_t*)value, policy);
}

class autoreleasepool {
public:
  autoreleasepool();
  ~autoreleasepool();
  autoreleasepool(const autoreleasepool&) = delete;
  autoreleasepool& operator=(const autoreleasepool&) = delete;
  autoreleasepool(autoreleasepool&&) = delete;
  autoreleasepool& operator=(autoreleasepool&&) = delete;

private:
  obj_t* m_pool;
};

obj_t* autoreleased(obj_t* object);

namespace literals {
inline class_t* operator"" _cls(const char* class_name, size_t) { return get_class(class_name); }

inline selector_t* operator"" _sel(const char* selector, size_t) { return get_selector(selector); }

} // namespace literals.

inline bool add_class_method(class_t* c, const char* sel, function imp, const char* types) {
  return add_class_method(c, get_selector(sel), imp, types);
}

template <class Fct>
inline bool add_class_method(class_t* c, selector_t* s, Fct imp, const char* types) {
  return add_class_method(c, s, (function)imp, types);
}

template <class Fct>
inline bool add_class_method(class_t* c, const char* sel, Fct imp, const char* types) {
  return add_class_method(c, get_selector(sel), (function)imp, types);
}

template <typename T, typename... Ts, std::enable_if_t<is_basic_type<T>, std::nullptr_t>>
inline std::string get_encoding() {
  if constexpr (sizeof...(Ts) > 0) {
    return get_encoding<T>() + get_encoding<Ts...>();
  }
  else if constexpr (std::is_same_v<T, void>) {
    return "v";
  }
  else if constexpr (std::is_null_pointer_v<T>) {
    return "*";
  }
  else {
    using type = std::remove_cv_t<T>;

    if constexpr (std::is_same_v<type, bool>) {
      return "B";
    }

    else if constexpr (std::is_integral_v<type>) {
      constexpr const size_t size = sizeof(T);
      constexpr const bool is_signed = std::is_signed_v<type>;

      if constexpr (size == 1) {
        return is_signed ? "c" : "C";
      }
      else if constexpr (size == 2) {
        return is_signed ? "s" : "S";
      }
      else if constexpr (size == 4) {
        if constexpr (std::is_same_v<type, long> || std::is_same_v<type, unsigned long>) {
          return is_signed ? "l" : "L";
        }
        else {
          return is_signed ? "i" : "I";
        }
      }
      else if constexpr (size == 8) {
        return is_signed ? "q" : "Q";
      }
      else if constexpr (size == 16) {
        return is_signed ? "t" : "T";
      }
    }

    else if constexpr (std::is_same_v<type, float>) {
      return "f";
    }

    else if constexpr (std::is_same_v<type, double>) {
      return "d";
    }
    else if constexpr (std::is_same_v<type, long double>) {
      return "D";
    }

    else if constexpr (std::is_same_v<type, float*>) {
      return "^f";
    }
    else if constexpr (std::is_same_v<type, double*>) {
      return "^d";
    }

    else if constexpr (std::is_same_v<type, void*>) {
      return "^v";
    }

    else if constexpr (std::is_same_v<type, obj_t*>) {
      return "@";
    }

    else if constexpr (std::is_same_v<type, selector_t*>) {
      return ":";
    }

    else if constexpr (std::is_same_v<type, class_t*>) {
      return "#";
    }
  }

  return "?";
}

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_pointer_v<T>, std::nullptr_t>>
inline std::string get_encoding(const char* name) {

  if constexpr (sizeof...(Ts) == 0) {
    return "^" + std::string(name);
  }
  else {
    return "^" + std::string(name) + get_encoding<Ts...>();
  }
}

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_pointer_v<T> && has_name_for_type<std::remove_pointer_t<T>>,
        std::nullptr_t>>
inline std::string get_encoding() {
  if constexpr (sizeof...(Ts) == 0) {
    return "^" + std::string(name_for_type<std::remove_pointer_t<T>>::value);
  }
  else {
    return "^" + std::string(name_for_type<std::remove_pointer_t<T>>::value) + get_encoding<Ts...>();
  }
}

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_class_v<T> && std::is_trivial_v<T>, std::nullptr_t>>
inline std::string get_encoding(const char* name) {
  static_assert(sizeof...(Ts) != 0, "");
  return "{" + std::string(name) + "=" + get_encoding<Ts...>() + "}";
}

template <typename T, typename... Ts,
    std::enable_if_t<!is_basic_type<T> && std::is_class_v<T> && std::is_trivial_v<T> && has_name_for_type<T>,
        std::nullptr_t>>
inline std::string get_encoding() {
  static_assert(sizeof...(Ts) != 0, "");
  return "{" + std::string(name_for_type<T>::value) + "=" + get_encoding<Ts...>() + "}";
}

//
//
//

using exception_callback = void (*)(obj_t*);

namespace detail {
void handle_try_catch_wrapper(std::exception_ptr eptr);
}

class exception : public std::exception {
public:
  exception() = default;
  exception(obj_t* ns_exception);
  virtual ~exception() noexcept override = default;

  virtual const char* what() const noexcept override;

  inline const std::string& name() const noexcept { return _name; }

  inline const std::string& reason() const noexcept { return _reason; }

private:
  std::string _name;
  std::string _reason;
  std::string _info;
};

template <class Fct>
inline auto try_block(Fct&& fct) {

  std::exception_ptr eptr = nullptr;
  try {
    return fct();
  } catch (...) {
    eptr = std::current_exception();
  }

  if (eptr) {
    detail::handle_try_catch_wrapper(eptr);
  }

  using result_type = std::invoke_result_t<Fct>;
  if constexpr (!std::is_same_v<result_type, void>) {
    return result_type{};
  }
}

ZBASE_END_SUB_NAMESPACE(apple, objc)
#endif // __ZBASE_APPLE__
