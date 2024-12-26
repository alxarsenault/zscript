#include "unit_tests.h"

using namespace utest;
#include <zbase/sys/path.h>

ZTEST_CASE("import", R"""(
  const math = import("math");
   return 32;
)""") {
  //  REQUIRE(value.is_float());
  //  REQUIRE(value == std::pow(zb::pi<zs::float_t>, 2.0) * 2.0);
}

ZTEST_CASE("import", R"""(
const m1 = import("module_01.zs");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 678);
}

ZTEST_CASE("import", R"""(
const m1 = import("module_01");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 678);
}

ZTEST_CASE("import", R"""(
const m1 = import("subfolder/module_01");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 234);
}

ZTEST_CASE("import", R"""(
const m1 = import("subfolder.module_01");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 234);
}

ZTEST_CASE("import", R"""(
const m1 = import("module_02");
 
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("import", ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/test_01.zs") { /*zb::print(get_test_name());*/ }

ZTEST_CASE("import", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("import", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

// TEST_CASE("proto-serialize") {
//   const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/module_01.zs";
//   zs::vm vm;
//   zs::engine* eng = vm.get_engine();
//   zb::byte_vector data_buffer;
//
//   {
//     zb::file_view file;
//     REQUIRE(!file.open(filepath));
//
//     zs::object fpo;
//     zs::jit_compiler compiler(eng);
//     if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
//       FAIL(compiler.get_error());
//     }
//
//     REQUIRE(fpo.is_function_prototype());
//     //    fpo.as_proto().debug_print();
//
//     if (auto err = fpo.as_proto().save(data_buffer)) {
//       FAIL(err.message());
//     }
//
//     std::ofstream output_file;
//     output_file.open(
//         ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/module_01.zsc", std::ios_base::out | std::ios_base::binary);
//     REQUIRE(output_file.is_open());
//
//     output_file.write((const char*)data_buffer.data(), data_buffer.size());
//     output_file.close();
//   }
//
////  {
////    zb::file_view file;
////    REQUIRE(!file.open(ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/compiler_03.zsc"));
////
////    zs::function_prototype_object* fpo_ptr = zs::function_prototype_object::create(eng);
////    if (auto err = fpo_ptr->load(file.content())) {
////      FAIL(err.message());
////    }
////
////    zs::object fpo(fpo_ptr, false);
////
////    //    zb::print("--------------------------------");
////    //    fpo.as_proto().debug_print();
////
////    zs::object closure = zs::object::create_closure(eng, fpo, vm->get_root());
////
////    zs::object result;
////    if (auto err = vm->call(closure, { vm->get_root() }, result)) {
////      FAIL(vm.get_error());
////    }
////
////    //    zb::print(result);
////  }
//}
