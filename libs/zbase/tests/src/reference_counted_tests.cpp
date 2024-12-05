#include <catch2.h>
#include <zbase/memory/reference_counted_ptr.h>

TEST_CASE("reference_counted") {

  static int retain_count = 0;
  static int release_count = 0;
  struct handler {
    static inline void retain(ZBASE_MAYBE_UNUSED int* ptr) { retain_count++; }

    static inline void release(int* ptr) { release_count++; }
  };

  zb::reference_counted_ptr<int, handler> p1(new int(78));
  REQUIRE(retain_count == 0);
  REQUIRE(release_count == 0);
  REQUIRE(p1);
  REQUIRE(*p1 == 78);

  int* ptr1 = p1;
  REQUIRE(*ptr1 == 78);

  zb::reference_counted_ptr<int, handler> p2 = p1;
  REQUIRE(retain_count == 1);
  REQUIRE(release_count == 0);
  REQUIRE(p2);
  REQUIRE(*p2 == 78);

  p1.reset();
  REQUIRE(retain_count == 1);
  REQUIRE(release_count == 1);
  REQUIRE(!p1);

  p2 = nullptr;
  REQUIRE(retain_count == 1);
  REQUIRE(release_count == 2);
  REQUIRE(!p2);

  zb::reference_counted_ptr<int, handler> p3(new int(32), true);
  REQUIRE(retain_count == 2);
  REQUIRE(release_count == 2);
  REQUIRE(p3);
  REQUIRE(*p3 == 32);

  p1 = std::move(p3);
  REQUIRE(retain_count == 2);
  REQUIRE(release_count == 2);
  REQUIRE(p1);
  REQUIRE(*p1 == 32);
  REQUIRE(!p3);
}
