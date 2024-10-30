#include <ztests/ztests.h>
#include <zbase/sys/file_view.h>



namespace {
TEST_CASE("zb::file_view") {

  zb::file_view file;

  zb::error_result err = file.open(ZBASE_TESTS_RESOURCES_DIRECTORY "/A.txt");

  REQUIRE(!err);

  REQUIRE(file.str() == "A");
}
} // namespace.
