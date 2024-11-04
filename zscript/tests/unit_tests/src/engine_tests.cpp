
#include <ztests/ztests.h>
#include <zscript/zscript.h>
#include <zbase/utility/print.h>
#include "lang/ztoken.h"
#include "lang/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

template <class T>
using has_my_function_impl = decltype(std::declval<T&>().has_my_function());

template <class T>
using has_my_function = zb::has_members<T, has_my_function_impl>;

template <class T>
using has_x_value = decltype(T::x);

template <class T>
using has_x = zb::has_members<T, has_x_value>;

template <class T>
using has_value_type_def = typename T::value_type;

template <class T>
using has_value_type = zb::has_members<T, has_value_type_def>;

//   template <class T>
//   using has_static_size_impl = decltype(T::size());
//
//   template <class T>
//   using has_static_size = zb::has_members<T, has_static_size_impl>;

ZBASE_DECL_USING_DECLTYPE(has_static_size_impl, size());
ZBASE_DECL_HAS_MEMBER(has_static_size, has_static_size_impl);

// ZBASE_DECL_USING_TYPE(meta_value_type, value_type);
// ZBASE_DECL_HAS_MEMBER(has_value_type, meta_value_type);

// ZBASE_DECL_USING_DECLTYPE(meta_size, size());

// template <class T>
// auto has_size_impl(int) -> decltype(std::declval<T&>().size(), void(),
// zb::true_t{});
//
// template <class T>
//  zb::false_t has_size_impl(...);
//
// template <class T>
// using has_size = decltype(has_size_impl<T>(0));

template <class T>
using meta_size = decltype(std::declval<T&>().size());

template <class T>
using has_size = zb::has_members<T, meta_size>;

// ZBASE_DECL_USING_DECLTYPE(meta_size, std::declval<T&>().size());
// ZBASE_DECL_HAS_MEMBER(has_size, meta_size);

// template <class T>
// using has_size = has_size_impl<T>();

// template <typename T>
// auto has_static_size(int) -> decltype(T::size(), void(), __zb::true_t{});

// template <typename T>
//__zb::false_t has_static_size(...);
//
// ZBASE_DECL_HAS_MEMBER(has_size, meta_size);

struct BAH {
  static inline int size() { return 0; }
};

TEST_CASE("zs::engine") {
  zs::engine eng;
  REQUIRE(zs::object::create_null().is_null());

  REQUIRE(has_value_type<std::vector<int>>::value);
  REQUIRE(has_size<std::vector<int>>::value);

  REQUIRE(!has_size<std::pair<int, int>>::value);
  REQUIRE(!has_static_size<std::vector<int>>::value);
  REQUIRE(has_static_size<BAH>::value);
}
