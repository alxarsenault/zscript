#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("sys::name", R"""(
  return sys::uname();
)""") {
  REQUIRE(value.is_table());
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
