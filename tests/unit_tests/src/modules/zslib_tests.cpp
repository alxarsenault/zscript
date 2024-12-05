#include "unit_tests.h"

namespace {
static std::stringstream zs_test_stream;

std::ostream& zs_test_stream_getter(zs::engine* eng, zs::raw_pointer_t data) { return zs_test_stream; }

void zs_test_init_func(zs::virtual_machine& vm) {
  zs_test_stream = std::stringstream();
  vm.get_engine()->set_stream_getter(zs_test_stream_getter);
}
} // namespace.

#define ZS_CODE_TEST_WITH_STREAM(name, code) ZS_CODE_TEST(name, code, zs_test_init_func)

ZS_CODE_TEST_WITH_STREAM("zs::print.01", R"""(zs::print("John", 32, "AAA");)""") {
  REQUIRE(zs_test_stream.str() == "John 32 AAA\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.02", R"""(zs::print();)""") { //
  REQUIRE(zs_test_stream.str() == "\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.03", R"""(zs::print("");)""") { //
  REQUIRE(zs_test_stream.str() == "\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.04", R"""(zs::print("", "");)""") {
  REQUIRE(zs_test_stream.str() == " \n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.05", R"""(zs::print<", ", "\n">("John", 32, "AAA");)""") {
  REQUIRE(zs_test_stream.str() == "John, 32, AAA\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.06", R"""(zs::print<", ", "--">("John", 32, "AAA");)""") {
  REQUIRE(zs_test_stream.str() == "John, 32, AAA--");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.07", R"""(zs::print<'π'>(32);)""") {
  REQUIRE(zs_test_stream.str() == "32\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.08", R"""(zs::print<"">(32, "AAA");)""") {
  REQUIRE(zs_test_stream.str() == "32AAA\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.09", R"""(zs::print<"-">(32, "AAA");)""") {
  REQUIRE(zs_test_stream.str() == "32-AAA\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::print.10", R"""(zs::print<'π'>(32, "AAA");)""") {
  REQUIRE(zs_test_stream.str() == "32πAAA\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::write.01", R"""(
zs::write(32, "AAA");
zs::write(33, "BBB", "\n");
)""") {
  REQUIRE(zs_test_stream.str() == "32AAA33BBB\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::write.02", R"""(
zs::write(32, "AAA");
zs::write<"", "\n">(33, "BBB");
)""") {
  REQUIRE(zs_test_stream.str() == "32AAA33BBB\n");
}

ZS_CODE_TEST_WITH_STREAM("zs::write.03", R"""(
zs::write<"--">(32, "AAA");
zs::write<'.'>(33, " ", "BBB", "\n");
)""") {
  REQUIRE(zs_test_stream.str() == "32--AAA33. .BBB.\n");
}

ZS_CODE_TEST("zs::print_to_string.01", R"""(
return zs::print_to_string("John", 32);)""") {
  REQUIRE(value == "John 32");
}

ZS_CODE_TEST("zs::print_to_string.02", R"""(
return zs::print_to_string<"">("John", 32);)""") {
  REQUIRE(value == "John32");
}

ZS_CODE_TEST("zs::print_to_string.03", R"""(
return zs::print_to_string<"-", "PP">("John", 32);)""") {
  REQUIRE(value == "John-32PP");
}

ZS_CODE_TEST("zs::write_to_string.02", R"""(
return zs::write_to_string("John", 32);)""") {
  REQUIRE(value == "John32");
}

ZS_CODE_TEST("zs::write_to_string.03", R"""(
var a = "p";
return zs::write_to_string<a>("John", 32);
)""") {
  REQUIRE(value == "Johnp32");
}

ZS_CODE_TEST("zs::write_to_string.04", R"""(
var a = {l = "8", k = "OO"};
return zs::write_to_string<a.l, a.k>("John", 32);
)""") {
  REQUIRE(value == "John832OO");
}

ZS_CODE_TEST("zs::write_to_string.06", R"""(
var a = {l = "8", k = "OO"};
var w = zs::write_to_string;
return w<a.l, a["k"]>("John", 32);
)""") {
  REQUIRE(value == "John832OO");
}

//
//
//

ZS_CODE_TEST("zs::dsddsadas.06", R"""(
var a = array<float32>(32);
return a;
)""") {
  REQUIRE(value.is_native_array());
  REQUIRE(value._na_type == zs::native_array_type::n_float);
  REQUIRE(value.as_native_array_interface().size() == 32);
}

ZS_CODE_TEST("zs::dsddsadas.32323", R"""(
var a = array<int32>(2);
return a;
)""") {
  REQUIRE(value.is_native_array());
  REQUIRE(value._na_type == zs::native_array_type::n_int32);
  REQUIRE(value.as_native_array_interface().size() == 2);
}

ZS_CODE_TEST("zs::dsddsadas.dsdsddsds", R"""(
var a = array<float32>([1.0, 2.0]);
return a;
)""") {
  REQUIRE(value.is_native_array());
  REQUIRE(value._na_type == zs::native_array_type::n_float);
  REQUIRE(value.as_native_array_interface().size() == 2);
}

ZS_CODE_TEST("zs::dsddsadas.dsddss", R"""(
var a = array<float32>([23.1, 2.0]);
return a[0];
)""") {
  REQUIRE(value == 23.1f);
}
