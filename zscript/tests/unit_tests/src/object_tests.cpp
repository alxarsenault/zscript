
#include <ztests/ztests.h>

#include <zscript/zscript.h>
#include <zbase/utility/print.h>

#include "lang/ztoken.h"
#include "lang/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"
#include <zbase/container/byte.h>

TEST_CASE("zs::obj") {

  zs::object obj1 = true;
  zs::object obj2;
  obj2 = true;
  REQUIRE(obj1.is_bool());
  REQUIRE(obj2.is_bool());
}

TEST_CASE("table::delegate") {
  //  zs::virtual_machine* vm = zs::create_virtual_machine(1024);
  zs::vm vm(1024);
  //  {
  zs::object tbl = zs::object::create_table(vm.get_engine());
  zs::object delegate = zs::object::create_table(vm.get_engine());

  zs::object key_john(vm.get_engine(), "john");

  delegate._table->set(key_john, zs::object(vm.get_engine(), "peter"));

  zs::object dst;
  REQUIRE(!vm->get(delegate, key_john, dst));
  REQUIRE(dst == "peter");

  // Not found in tbl.
  REQUIRE(vm->get(tbl, key_john, dst));

  // Set delegate to tbl.
  REQUIRE(!tbl.set_delegate(delegate));

  // Should be found in tbl.
  REQUIRE(!vm->get(tbl, key_john, dst));
  REQUIRE(dst == "peter");
  //  }

  //  zs::close_virtual_machine(vm);
}

// TEST_CASE("dskjdskl") {
//
//   const zs::allocate_t my_allocate = [](zs::engine* eng, zs::raw_pointer_t user_ptr, void* ptr, size_t
//   size,
//                                          size_t old_size, zs::alloc_info_t info) -> void* {
//     if (!size) {
//       zbase_assert(ptr, "invalid pointer");
//       //      zb::print("free", ptr);
//       ::free(ptr);
//       return nullptr;
//     }
//
//     if (ptr) {
//       //      zb::print("realloc", ptr, size);
//       return ::realloc(ptr, size);
//     }
//
//     zbase_assert(!ptr, "pointer should be nullptr");
//     //    zb::print("malloc",  size);
//
//     return ::malloc(size);
//   };
//
//   {
//     zs::engine eng(my_allocate);
//     zs::reference_counted_pointer<zs::table_object> ptr(zs::table_object::create(&eng));
//     REQUIRE(ptr.ref_count() == 1);
//   }
//
//   {
//     zs::engine eng(my_allocate);
//     zs::reference_counted_pointer<zs::table_object> ptr(zs::table_object::create(&eng), true);
//     REQUIRE(ptr.ref_count() == 2);
//
//     ptr->release();
//     REQUIRE(ptr.ref_count() == 1);
//   }
//
//   {
//     zs::engine eng(my_allocate);
//     zs::reference_counted_pointer<zs::table_object> ptr(zs::table_object::create(&eng));
//     REQUIRE(ptr.ref_count() == 1);
//
//     {
//       zs::reference_counted_pointer<zs::table_object> ptr2 = ptr;
//       REQUIRE(ptr.ref_count() == 2);
//     }
//
//     REQUIRE(ptr.ref_count() == 1);
//
//     {
//       zs::reference_counted_pointer<zs::table_object> ptr2 = std::move(ptr);
//       REQUIRE(ptr2.ref_count() == 1);
//
//       REQUIRE(ptr.ref_count() == 0);
//
//       ptr2 = nullptr;
//       REQUIRE(ptr2.ref_count() == 0);
//     }
//   }
// }

template <typename T>
static inline std::string int_to_hex_aligned(T i) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2);

  if constexpr (std::is_unsigned_v<T>) {
    stream << std::hex << (uint64_t)i;
  }
  else {
    stream << std::hex << (int64_t)i;
  }

  return stream.str();
}

template <typename T>
static inline std::string int_to_hex(T i) {
  std::stringstream stream;

  if constexpr (std::is_unsigned_v<T>) {
    stream << "0x" << std::hex << (uint64_t)i;
  }
  else {
    stream << "0x" << std::hex << (int64_t)i;
  }
  return stream.str();
}

TEST_CASE("zs::object::to_string") {
  zs::engine eng;
  zs::string s((zs::allocator<char>(&eng)));

  auto strv = zs::object::create_string_view("Bingo");
  auto sstr = zs::object::create_small_string("Bingo");
  auto lstr = zs::object::create_long_string(&eng, "Bingo");
  auto flt = zs::object(55.3);
  auto i = zs::object(55);
  auto f = zs::object(false);
  auto t = zs::object(true);
  auto n = zs::object(nullptr);
  auto arr = zs::object::create_array(&eng, 0);
  auto table = zs::object::create_table(&eng);

  //
  // zs::error_result to_string(zs::string& s).
  //

  REQUIRE((!strv.convert_to_string(s) && s == "Bingo"));
  REQUIRE((!sstr.convert_to_string(s) && s == "Bingo"));
  REQUIRE((!lstr.convert_to_string(s) && s == "Bingo"));
  REQUIRE((!flt.convert_to_string(s) && s == "55.30"));
  REQUIRE((!i.convert_to_string(s) && s == "55"));
  REQUIRE((!f.convert_to_string(s) && s == "false"));
  REQUIRE((!t.convert_to_string(s) && s == "true"));
  REQUIRE((!n.convert_to_string(s) && s == "null"));

  REQUIRE((!arr.convert_to_string(s) && s == std::string_view(int_to_hex_aligned(arr._value))));
  REQUIRE((!table.convert_to_string(s) && s == std::string_view(int_to_hex_aligned(table._value))));

  //
  // std::string to_string().
  //

  REQUIRE(strv.convert_to_string() == "\"Bingo\"");
  REQUIRE(sstr.convert_to_string() == "\"Bingo\"");
  REQUIRE(lstr.convert_to_string() == "\"Bingo\"");
  REQUIRE(flt.convert_to_string() == "55.3");
  REQUIRE(i.convert_to_string() == "55");
  REQUIRE(f.convert_to_string() == "false");
  REQUIRE(t.convert_to_string() == "true");
  REQUIRE(n.convert_to_string() == "null");

  REQUIRE((!arr.convert_to_string(s) && s == std::string_view(int_to_hex_aligned(arr._value))));
  REQUIRE((!table.convert_to_string(s) && s == std::string_view(int_to_hex_aligned(table._value))));

  //
  // zs::error_result to_debug_string(zs::string& s).
  //

  REQUIRE((!strv.to_debug_string(s) && s == "string_view : Bingo"));
  REQUIRE((!sstr.to_debug_string(s) && s == "small_string : Bingo"));
  REQUIRE((!lstr.to_debug_string(s) && s == "long_string : Bingo"));
  REQUIRE((!flt.to_debug_string(s) && s == "float : 55.3"));
  REQUIRE((!i.to_debug_string(s) && s == "integer : 55"));
  REQUIRE((!f.to_debug_string(s) && s == "bool : false"));
  REQUIRE((!t.to_debug_string(s) && s == "bool : true"));
  REQUIRE((!n.to_debug_string(s) && s == "null : null"));
  REQUIRE((!table.to_debug_string(s) && s == "table : " + int_to_hex_aligned(table._value)));
  REQUIRE((!arr.to_debug_string(s) && s == "array : " + int_to_hex_aligned(arr._value)));

  //
  // std::string to_debug_string().
  //

  REQUIRE(strv.to_debug_string() == "string_view : Bingo");
  REQUIRE(sstr.to_debug_string() == "small_string : Bingo");
  REQUIRE(lstr.to_debug_string() == "long_string : Bingo");
  REQUIRE(flt.to_debug_string() == "float : 55.3");
  REQUIRE(i.to_debug_string() == "integer : 55");
  REQUIRE(f.to_debug_string() == "bool : false");
  REQUIRE(t.to_debug_string() == "bool : true");
  REQUIRE(n.to_debug_string() == "null : null");
  REQUIRE(table.to_debug_string() == "table : " + int_to_hex_aligned(table._value));
  REQUIRE(arr.to_debug_string() == "array : " + int_to_hex_aligned(arr._value));
}

TEST_CASE("zs::object::string_view") {

  auto obj = zs::object::create_string_view("Bingo");

  REQUIRE(obj.is_string_view());
  REQUIRE(obj.is_string());

  std::string_view str;
  REQUIRE(!obj.get_string(str));
  REQUIRE(str == "Bingo");

  zs::engine eng;
  zs::string s((zs::allocator<char>(&eng)));

  REQUIRE(!obj.convert_to_string(s));
}

TEST_CASE("zs::can_object_type_be_false") {
  using enum zs::object_type;

  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_small_string));
  REQUIRE(zs::is_object_type_convertible_to_false(k_null));
  REQUIRE(zs::is_object_type_convertible_to_false(k_bool));
  REQUIRE(zs::is_object_type_convertible_to_false(k_integer));
  REQUIRE(zs::is_object_type_convertible_to_false(k_float));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_raw_pointer));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_long_string));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_table));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_array));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_closure));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_native_closure));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_user_data));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_class));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_instance));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_weak_ref));
  REQUIRE_FALSE(zs::is_object_type_convertible_to_false(k_function_prototype));
}

TEST_CASE("zs::get_type_mask") {
  using enum zs::object_type;

  uint32_t idx = 1;

  REQUIRE(zs::get_object_type_mask(k_small_string) == idx);

  //#define _X(name, str, exposed_name)                                                                          \
//  if (name != k_small_string) {                                                                              \
//    REQUIRE(zs::get_object_type_mask(name) == (idx <<= 1));                                                  \
//  }
  //  ZS_TYPE_ENUM(_X)
  // #undef _X

  //#define ZS_DECL_OBJECT_TYPE(name, exposed_name)                                                              \
//  if (k_##name != k_small_string) {                                                                          \
//    REQUIRE(zs::get_object_type_mask(k_##name) == (idx <<= 1));                                              \
//  }

  // #include <zscript/zobject_def.h>
  // #undef ZS_DECL_OBJECT_TYPE

#define _X(name, str, exposed_name)                         \
  if (name != k_small_string) {                             \
    REQUIRE(zs::get_object_type_mask(name) == (idx <<= 1)); \
  }

  ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
}

TEST_CASE("zs::is_ref_counted") {
  using enum zs::object_type;

  //  REQUIRE_FALSE(zs::is_object_type_ref_counted(k_small_string));
  //  REQUIRE_FALSE(zs::is_object_type_ref_counted(k_null));
  //  REQUIRE_FALSE(zs::is_object_type_ref_counted(k_bool));
  //  REQUIRE_FALSE(zs::is_object_type_ref_counted(k_integer));
  //  REQUIRE_FALSE(zs::is_object_type_ref_counted(k_float));
  //  REQUIRE_FALSE(zs::is_object_type_ref_counted(k_raw_pointer));
  //  REQUIRE(zs::is_object_type_ref_counted(k_long_string));
  //  REQUIRE(zs::is_object_type_ref_counted(k_table));
  //  REQUIRE(zs::is_object_type_ref_counted(k_array));
  //  REQUIRE(zs::is_object_type_ref_counted(k_closure));
  //  REQUIRE(zs::is_object_type_ref_counted(k_native_closure));
  //  REQUIRE(zs::is_object_type_ref_counted(k_user_data));
  //  REQUIRE(zs::is_object_type_ref_counted(k_class));
  //  REQUIRE(zs::is_object_type_ref_counted(k_instance));
  //  REQUIRE(zs::is_object_type_ref_counted(k_weak_ref));
  //  REQUIRE(zs::is_object_type_ref_counted(k_function_prototype));
}

TEST_CASE("zs::object") {
  using enum zs::object_type;

  zs::engine state;
  zs::engine* st = &state;

  {
    zs::object obj(st, "Alex");
    REQUIRE(obj.get_type() == k_small_string);
    REQUIRE(obj.get_value<std::string_view>() == "Alex");
  }

  {
    zs::object obj(32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<int>() == 32);
  }

  {
    zs::object obj(32L);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<long>() == 32);
  }

  {
    zs::object obj(32UL);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<unsigned long>() == 32UL);
  }

  {
    zs::object obj(32LL);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<long long>() == 32LL);
  }

  {
    zs::object obj(32ULL);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<unsigned long long>() == 32ULL);
  }

  {
    zs::object obj((uint32_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<uint32_t>() == 32);
  }

  {
    zs::object obj((int32_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<int32_t>() == 32);
  }

  {
    zs::object obj((int64_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<int64_t>() == 32);
  }

  {
    zs::object obj((uint64_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<uint64_t>() == 32);
  }

  {
    zs::object obj((short)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<short>() == 32);
  }

  {
    zs::object obj((unsigned short)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<unsigned short>() == 32);
  }

  {
    zs::object obj((int16_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<int16_t>() == 32);
  }

  {
    zs::object obj((uint16_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<uint16_t>() == 32);
  }

  {
    zs::object obj((int8_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<int8_t>() == 32);
  }

  {
    zs::object obj((uint8_t)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<uint8_t>() == 32);
  }

  {
    zs::object obj((char)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<char>() == 32);
  }

  {
    zs::object obj((unsigned char)32);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<unsigned char>() == 32);
  }

  {
    zs::object obj(0);
    REQUIRE(obj.get_type() == k_integer);
    REQUIRE(obj.get_value<zs::int_t>() == 0);

    zs::object obj2 = obj;
    REQUIRE(obj2.get_type() == k_integer);
  }

  {
    zs::object obj(nullptr);
    REQUIRE(obj.get_type() == k_null);

    zs::object obj2 = obj;
    REQUIRE(obj2.get_type() == k_null);
  }

  {
    zs::object obj(32.2f);
    REQUIRE(obj.get_type() == k_float);
    REQUIRE(obj.get_value<float>() == 32.2f);
  }

  {
    zs::object obj(32.2);
    REQUIRE(obj.get_type() == k_float);
    REQUIRE(obj.get_value<double>() == 32.2);
  }

  {
    zs::object obj(0.0f);
    REQUIRE(obj.get_type() == k_float);
    REQUIRE(obj.get_value<float>() == 0.0f);

    zs::object obj2 = obj;
    REQUIRE(obj2.get_type() == k_float);
    REQUIRE(obj2.get_value<float>() == 0.0f);
  }

  {
    zs::object obj(0.0);
    REQUIRE(obj.get_type() == k_float);
    REQUIRE(obj.get_value<double>() == 0.0);

    zs::object obj2 = obj;
    REQUIRE(obj2.get_type() == k_float);
    REQUIRE(obj2.get_value<double>() == 0.0);
  }

  {
    std::string_view value = "Alexandre Arsenault 12345678910";
    zs::object obj(st, value);
    REQUIRE(obj.get_type() == k_long_string);
    REQUIRE(obj.get_value<std::string_view>() == value);
    REQUIRE(obj.get_ref_count() == 1);

    {
      zs::object obj2 = obj;
      REQUIRE(obj2.get_type() == k_long_string);
      REQUIRE(obj2.get_value<std::string_view>() == value);
      REQUIRE(obj2.get_ref_count() == 2);
    }

    REQUIRE(obj.get_ref_count() == 1);
  }
}

TEST_CASE("zs::object_ptr::table") {
  zs::engine eng;
  zs::object obj = zs::object::create_table(&eng);

  obj._table->set(zs::object(&eng, "Bingo"), zs::object(&eng, "Bango"));
}

TEST_CASE("zs::weak_ref") {
  zs::vm vm;
  zs::object robj;
  //  zs_get_top((zs_vm*)vm.get_virtual_machine());

  {
    zs::object obj = zs::object::create_table(vm.get_engine());

    REQUIRE(obj.is_table());

    robj = obj.get_weak_ref();
    REQUIRE(robj.is_weak_ref());

    zs::object obj2 = robj.get_weak_ref_value();
    REQUIRE(obj2.is_table());
  }
  zs::object obj2 = robj.get_weak_ref_value();
  REQUIRE(obj2.is_null());
}

TEST_CASE("com") {
  {
    zs::object o1(312);
    zs::object o2(312);
    REQUIRE(o1 == o2);
  }
  {
    zs::object o1(312);
    zs::object o2(312.0);
    REQUIRE(o1 == o2);
  }

  {
    zs::object o1 = zs::object::create_string_view("john");
    REQUIRE(o1 == "john");

    zs::object o2 = zs::object::create_small_string("john");
    REQUIRE(o1 == o2);
    REQUIRE(o1.compare(o2) == 0);
  }
}

TEST_CASE("create_type_mask") {
  {
    uint32_t mask = zs::create_type_mask(zs::object_type::k_integer);
    REQUIRE(mask == zs::get_object_type_mask(zs::object_type::k_integer));
  }

  {
    uint32_t mask = zs::create_type_mask(zs::object_type::k_float);
    REQUIRE(mask == zs::get_object_type_mask(zs::object_type::k_float));
  }

  {
    uint32_t mask = zs::create_type_mask(zs::object_type::k_bool);
    REQUIRE(mask == zs::get_object_type_mask(zs::object_type::k_bool));
  }

  {
    uint32_t mask = zs::create_type_mask(zs::object_type::k_integer, zs::object_type::k_float);
    REQUIRE(mask
        == (zs::get_object_type_mask(zs::object_type::k_integer)
            | zs::get_object_type_mask(zs::object_type::k_float)));
  }

  {
    uint32_t mask = zs::create_type_mask(zs::object_type_mask::k_bool);
    REQUIRE(mask == zs::get_object_type_mask(zs::object_type::k_bool));
  }

  {
    uint32_t mask = zs::create_type_mask(
        zs::object_type_mask::k_integer, zs::get_object_type_mask(zs::object_type::k_float));
    REQUIRE(mask
        == (zs::get_object_type_mask(zs::object_type::k_integer)
            | zs::get_object_type_mask(zs::object_type::k_float)));
  }
}

TEST_CASE("binary") {
  {
    zs::object obj = 21232;
    zb::byte_vector vec;
    size_t sz = 0;
    REQUIRE(!obj.to_binary(vec, sz));
    REQUIRE(sz == sizeof(zs::object_base));

    zs::object obj2;
    std::memcpy(&obj2, vec.data(), sz);
    REQUIRE(obj2 == 21232);
  }
  {
    zs::object obj = 212.12;
    zb::byte_vector vec;
    size_t sz = 0;
    REQUIRE(!obj.to_binary(vec, sz));
    REQUIRE(sz == sizeof(zs::object_base));

    zs::object obj2;
    std::memcpy(&obj2, vec.data(), sz);
    REQUIRE(obj2 == 212.12);
  }

  {
    zs::engine eng;

    {
      auto arr = zs::_a(eng, 5);
      {
        auto& vec = *arr.get_array_internal_vector();
        REQUIRE(vec.size() == 5);

        for (int i = 0; i < 5; i++) {
          vec[i] = i;
        }
      }

      zb::byte_vector buff;
      size_t sz = 0;
      REQUIRE(!arr.to_binary(buff, sz));
      REQUIRE(sz == sizeof(zs::object_base) * 6);
    }
  }

  {
    zs::engine eng;

    {
      auto arr = zs::_a(eng, 5);
      {
        auto& vec = *arr.get_array_internal_vector();
        REQUIRE(vec.size() == 5);

        for (int i = 0; i < 5; i++) {
          vec[i] = i;
        }
      }

      zb::byte_vector buff;
      size_t sz = 0;
      REQUIRE(!arr.to_binary(buff, sz));
      REQUIRE(sz == sizeof(zs::object_base) * 6);

      sz = 0;
      zs::object obj2;
      REQUIRE(!zs::object::from_binary(&eng, buff, obj2, sz));
      {
        auto& vec = *obj2.get_array_internal_vector();
        REQUIRE(vec.size() == 5);

        for (int i = 0; i < 5; i++) {
          //          zb::print(vec[i].to_debug_string());
          REQUIRE(vec[i] == i);
        }
      }
    }
  }

  {
    zs::engine eng;

    {
      auto arr = zs::_a(eng, 5);
      {
        auto& vec = *arr.get_array_internal_vector();
        REQUIRE(vec.size() == 5);

        for (int i = 1; i < 5; i++) {
          vec[i] = i;
        }

        vec[0] = zs::_s(eng, "jkjkjkjkjklkjkljkljlkhjghjghgfgjfgfgfjgfh");
      }

      zb::byte_vector buff;
      size_t sz = 0;
      REQUIRE(!arr.to_binary(buff, sz));
      //      REQUIRE(sz == sizeof(zs::object)* 6);

      sz = 0;
      zs::object obj2;
      REQUIRE(!zs::object::from_binary(&eng, buff, obj2, sz));
      {
        auto& vec = *obj2.get_array_internal_vector();
        REQUIRE(vec.size() == 5);

        for (int i = 1; i < 5; i++) {
          REQUIRE(vec[i] == i);
        }

        REQUIRE(vec[0] == "jkjkjkjkjklkjkljkljlkhjghjghgfgjfgfgfjgfh");
      }
    }
  }

  {
    zs::engine eng;

    {
      auto table = zs::_t(eng);
      {
        auto& map = *table.get_table_internal_map();
        //        REQUIRE(vec.size() == 5);

        for (int i = 0; i < 5; i++) {
          map[i] = i;
        }

        map[zs::_ss("alex")] = zs::_ss("peter");
      }

      zb::byte_vector buff;
      size_t sz = 0;
      REQUIRE(!table.to_binary(buff, sz));

      sz = 0;
      zs::object obj2;
      REQUIRE(!zs::object::from_binary(&eng, buff, obj2, sz));
      {
        //        zb::print(obj2.convert_to_string());
        auto& map = *obj2.get_table_internal_map();
        //        REQUIRE(vec.size() == 5);
        //
        for (int i = 0; i < 5; i++) {
          //          zb::print(vec[i].to_debug_string());
          REQUIRE(map[i] == i);
        }
        REQUIRE(map["alex"] == "peter");
        //        }
      }
    }
  }
}

TEST_CASE("create_array_from_container") {
  zs::engine eng;

  {
    std::vector<float> a = { 1.0f, 2.2f };
    zs::object obj = zs::object::create_array(&eng, a);
    REQUIRE(obj.is_array());

    const auto& vec = *obj.get_array_internal_vector();
    REQUIRE(vec.size() == 2);

    REQUIRE(vec[0] == 1.0f);
    REQUIRE(vec[1] == 2.2f);
  }
}

TEST_CASE("native_array") {
  {
    zs::engine eng;

    //    zs::native_array_object<float>* arr =
    //    zs::native_array_object<float>::create(&eng, 2); arr->get_vector()[0]
    //    = 2.2f; arr->get_vector()[1] = 2.3f;

    zs::object obj = zs::object::create_native_array(&eng, zs::native_array_type::n_float, 2);
    //    zs::native_array_object_t* nobj = (zs::native_array_object_t*)&obj;

    //    REQUIRE(nobj->_ntype == zs::native_array_type::n_float);

    auto& vec = obj._native_array<float>()->get_vector();

    vec[0] = 2.2f;
    vec[1] = 2.3f;

    REQUIRE(obj.is_native_array());

    REQUIRE(obj._native_array<float>()->get_vector()[0] == 2.2f);
  }

  {
    zs::engine eng;
    zs::object obj = zs::object::create_native_array<float>(&eng, 2);
    //    zs::native_array_object_t* nobj = (zs::native_array_object_t*)&obj;

    //    REQUIRE(nobj->_ntype == zs::native_array_type::n_float);

    auto& vec = obj._native_array<float>()->get_vector();

    vec[0] = 2.2f;
    vec[1] = 2.3f;
  }
}

TEST_CASE("LLLLLLLL") {

  zs::engine eng;
  {
    zs::vm vm(&eng);
    zs::var d = zs::var::create_native_closure_function(
        &eng, +[](zs::vm_ref vm, zs::int_t p1, float k, std::string_view s) {
          REQUIRE(p1 == 55);
          REQUIRE(k == 89.56f);
          REQUIRE(s == "john");
          return s;
        });

    zs::int_t np = vm.stack_size();
    vm.push_root();
    vm.push_integer(55);
    vm.push_float(89.56);
    vm.push_string("john");
    zs::int_t n_params = vm.stack_size() - np;
    zs::var rval;
    REQUIRE(!vm->call(d, n_params, vm.stack_size() - n_params, rval));

    REQUIRE(rval == "john");
  }
}

TEST_CASE("KKLKKK") {

  zs::engine eng;
  {

    {

      {
        zs::vm vm(&eng);

        int ksd = 3333;
        zs::var fct = zs::var::create_native_closure_function(
            &eng, [&](zs::virtual_machine* vm, zs::int_t val, const std::string& s) {
              REQUIRE(val == 123);
              REQUIRE(s == "bacon");
              return 32 + ksd;
            });

        vm.push_root();
        vm.push_integer(123);
        vm.push_string("bacon");

        zs::var rval;
        REQUIRE(!vm->call(fct, 3, vm.stack_size() - 3, rval));

        REQUIRE(rval == ksd + 32);
      }

      {
        zs::vm vm(&eng);

        int ksd = 3333;

        zs::var fct = zs::var::create_native_closure_function(
            &eng, [&](zs::virtual_machine* vm, zs::int_t val, const std::string& s) {
              REQUIRE(val == 123);
              REQUIRE(s == "bacon");
              return 32 + ksd;
            });

        vm.push_root();
        vm.push_integer(123);
        vm.push_string("bacon");

        zs::var rval;
        REQUIRE(!vm->call(fct, 3, vm.stack_size() - 3, rval));

        REQUIRE(rval == ksd + 32);
      }

      {
        zs::vm vm(&eng);

        int ksd = 3333;

        zs::var fct = zs::var::create_native_closure_function(&eng,
            std::function<zs::int_t(zs::int_t, const std::string&)>([=](zs::int_t val, const std::string& s) {
              REQUIRE(val == 123);
              REQUIRE(s == "bacon");
              return 32 + ksd;
            }));

        vm.push_root();
        vm.push_integer(123);
        vm.push_string("bacon");

        {
          REQUIRE(vm[-1].is_string());
          zs::string s((zs::allocator<char>(&eng)));
          REQUIRE(!vm[-1].convert_to_string(s));
          REQUIRE(s == "bacon");
        }

        {
          REQUIRE(vm[-1].is_string());
          std::string s;
          REQUIRE(!vm[-1].convert_to_string(s));
          REQUIRE(s == "bacon");
        }

        zs::var rval;
        REQUIRE(!vm->call(fct, 3, vm.stack_size() - 3, rval));

        REQUIRE(rval == ksd + 32);
      }
    }

    zs::var a = zs::var::create_native_closure_function(
        &eng, +[](zs::vm_ref vm, zs::int_t val, zs::float_t fval, const zs::var& obj) {
          //          zb::print(__FUNCTION__, val, fval, obj.get_type());
          return 89;
        });

    {
      zs::vm vm(&eng);
      vm.push_root();
      vm.push_integer(33);
      vm.push_float(12.12);

      zs::var obj = zs::var::create_array(&eng, 5);
      vm.push(obj);

      zs::var rval;
      REQUIRE(!vm->call(a, 4, vm.stack_size() - 4, rval));

      REQUIRE(rval == 89);
    }

    {
      zs::vm vm(&eng);

      zs::var b = zs::var::create_native_closure_function(
          &eng, +[](zs::vm_ref vm) {
            //            zb::print(__FUNCTION__, "B");
            return 11;
          });

      vm.push_root();

      zs::var rval;
      REQUIRE(!vm->call(b, 1, vm.stack_size() - 1, rval));

      REQUIRE(rval == 11);
    }

    {
      zs::vm vm(&eng);

      zs::var c = zs::var::create_native_closure_function(
          &eng, +[]() {
            //            zb::print(__FUNCTION__, "C");
            return 11;
          });

      vm.push_root();

      zs::var rval;
      REQUIRE(!vm->call(c, 1, vm.stack_size() - 1, rval));

      REQUIRE(rval == 11);
    }

    {
      zs::vm vm(&eng);

      zs::var d = zs::var::create_native_closure_function(
          &eng,
          +[](zs::vm_ref vm, zs::int_t p1, std::vector<std::string> vec, std::string s,
               std::map<std::string, zs::var> m, std::map<std::string, zs::var> m2) {
            //            zb::print("D", "p1", p1, "\nvec.size", vec.size(),
            //            "\nVEC", vec, "\nS", s, "\nmap", m, m2);

            return p1;
          });

      zs::int_t np = vm.stack_size();
      vm.push_root();
      vm.push_integer(55);
      zs::var obj = zs::_a(&eng, 3);
      auto& vec = *obj.get_array_internal_vector();
      vec[0] = 88;
      vec[1] = 89.21;
      vec[2] = zs::_ss("kl");
      vm.push(obj);

      vm.push_string("john");
      //      vm.push_integer(55);

      zs::var tbl = zs::_o(&eng, //
          std::initializer_list<std::pair<zs::object, zs::object>>{
              { zs::_ss("john2"), 89 }, //
              { zs::_ss("john9"), 989 }, //
              { zs::_ss("joh"), zs::_ss("pljkl") }, //
              { zs::_ss("A"), obj } //
          });

      ;

      std::array<char, 3> arr = { 1, 2, 3 };

      zs::var tbl2 = zs::var(&eng, //
          std::map<std::string, zs::var>{
              { "john", 21 }, //
              { "johnll", zs::_a(&eng, arr) }, //
              { "peter", obj } //
          });
      //
      //      zb::print("--------------------------",tbl);
      //      zb::print("--------------------------",tbl2);

      vm.push(tbl);
      vm.push(tbl2);
      //      vm.push_integer(11);
      zs::int_t n_params = vm.stack_size() - np;
      //      zb::print("LLLLL", n_params);
      zs::var rval;
      REQUIRE(!vm->call(d, n_params, vm.stack_size() - n_params, rval));

      //      zb::print(rval.to_debug_string());
      REQUIRE(rval == 55);
    }
  }
}
