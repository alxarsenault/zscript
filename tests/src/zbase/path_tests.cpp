#include <catch2.h>
#include <zscript/base/sys/path.h>

namespace {
TEST_CASE("zb::path") {
  zb::sys::path<> p = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/zbase/abc.txt";
  REQUIRE(p.is_file());
  REQUIRE(!p.is_directory());
  REQUIRE(p.file_size() == 3);
}
} // namespace.
