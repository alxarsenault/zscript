#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("sys::name", R"""(
 //zs::print(sys::uname());

bool k = true;
bool k2 = false;
var arr = 0;

float g = 21.22;
//zs::print(zs::is_float(g), zs::is_true(arr));
return [zs::is_bool(k), zs::is_bool(k2), zs::is_bool(true), zs::is_bool(32)];
)""") {
  REQUIRE(value.is_array());

  REQUIRE(value.as_array()[0] == true);
  REQUIRE(value.as_array()[1] == true);
  REQUIRE(value.as_array()[2] == true);
  REQUIRE(value.as_array()[3] == false);
  //  REQUIRE(std::filesystem::exists(value.get_string_unchecked()));
}

ZTEST_CASE("sys::exec", R"""(
var res = sys::exec("source ~/.zprofile; source ~/.zshrc; which cmake;");
return res.stdout;
)""") {
  REQUIRE(!value.get_string_unchecked().empty());
  REQUIRE(std::filesystem::exists(std::filesystem::path(std::string_view(value.get_string_unchecked()))));
}

ZTEST_CASE("sys::exec", R"""(
var res = sys::exec("echo 'Alex'");
return res.stdout;
)""") {
  REQUIRE(value == "Alex");
}

ZTEST_CASE("sys::exec", R"""(
var res = sys::exec('''echo "Bingo" 1>&2;''');
return res.stderr;
)""") {
  REQUIRE(value == "Bingo");
}

ZTEST_CASE("sys::exec", R"""(
  var res = sys::exec( ZSCRIPT_TESTS_RESOURCES_DIRECTORY + "/bash_01.sh");
  return res.stdout;
)""") {
  REQUIRE(value == "Bingo");
}
