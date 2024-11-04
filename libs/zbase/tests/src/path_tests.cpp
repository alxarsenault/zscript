#include <ztests/ztests.h>
#include <zbase/sys/path.h>

namespace {

TEST_CASE("zb::path") {

  zb::path<> p = ZBASE_TESTS_RESOURCES_DIRECTORY "/abc.txt";
  REQUIRE(p.is_file());
  REQUIRE(!p.is_directory());
  REQUIRE(p.file_size() == 3);
}
} // namespace.
