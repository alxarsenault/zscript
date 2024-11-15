#include <catch2.h>
#include <zbase/utility/math.h>

namespace {
TEST_CASE("Equation") {

  REQUIRE(zb::iexp2(0) == 1);
  REQUIRE(zb::iexp2(1) == 2);
  REQUIRE(zb::iexp2(2) == 4);
  REQUIRE(zb::iexp2(3) == 8);
  REQUIRE(zb::iexp2(4) == 16);
}

TEST_CASE("iexp2") {
  REQUIRE(zb::iexp2(0) == 1);
  REQUIRE(zb::iexp2(1) == 2);
  REQUIRE(zb::iexp2(2) == 4);
  REQUIRE(zb::iexp2(3) == 8);
  REQUIRE(zb::iexp2(4) == 16);
}

TEST_CASE("log2_of_power_of_two") {
  for (uint32_t i = 0; i < 31; i++) {
    REQUIRE(zb::log2_of_power_of_two((uint32_t)zb::iexp2(i)) == (int32_t)i);
  }
}

TEST_CASE("is_power_of_two") {
  REQUIRE(zb::is_power_of_two(1));
  REQUIRE(zb::is_power_of_two(2));
  REQUIRE(zb::is_power_of_two(4));

  REQUIRE_FALSE(zb::is_power_of_two(0));
  REQUIRE_FALSE(zb::is_power_of_two(3));
  REQUIRE_FALSE(zb::is_power_of_two(5));
  REQUIRE_FALSE(zb::is_power_of_two(9));

  for (uint64_t i = 0; i < std::numeric_limits<uint64_t>::digits - 1; i++) {
    REQUIRE(zb::is_power_of_two(uint64_t(1) << i));
  }
}

TEST_CASE("next_power_of_two") {
  REQUIRE(zb::next_power_of_two(0) == 1);
  REQUIRE(zb::next_power_of_two(1) == 2);
  REQUIRE(zb::next_power_of_two(2) == 4);
  REQUIRE(zb::next_power_of_two(3) == 4);
  REQUIRE(zb::next_power_of_two(4) == 8);
  REQUIRE(zb::next_power_of_two(5) == 8);

  for (uint64_t i = 0; i < std::numeric_limits<uint64_t>::digits - 2; i++) {
    const uint64_t value = uint64_t(1) << i;
    const uint64_t next = uint64_t(1) << (i + 1);
    REQUIRE(zb::next_power_of_two(value) == next);
  }
}

TEST_CASE("round_to_power_of_two") {
  REQUIRE(zb::round_to_power_of_two(0) == 1);
  REQUIRE(zb::round_to_power_of_two(1) == 1);
  REQUIRE(zb::round_to_power_of_two(2) == 2);
  REQUIRE(zb::round_to_power_of_two(3) == 4);
  REQUIRE(zb::round_to_power_of_two(4) == 4);
  REQUIRE(zb::round_to_power_of_two(5) == 8);

  for (uint64_t i = 0; i < std::numeric_limits<uint64_t>::digits - 2; i++) {
    const uint64_t value = uint64_t(1) << i;
    const uint64_t next = uint64_t(1) << (i + 1);
    REQUIRE(zb::round_to_power_of_two(value) == value);
    REQUIRE(zb::round_to_power_of_two(value + 1) == next);
  }
}

TEST_CASE("will_next_power_of_two_overflow") {
  for (uint64_t i = 0; i < std::numeric_limits<uint64_t>::digits - 2; i++) {
    REQUIRE_FALSE(zb::will_next_power_of_two_overflow(uint64_t(1) << i));
  }

  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<int8_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<uint8_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<int16_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<uint16_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<int32_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<uint32_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<int64_t>::max()));
  REQUIRE(zb::will_next_power_of_two_overflow(std::numeric_limits<uint64_t>::max()));

  REQUIRE(zb::will_next_power_of_two_overflow(uint64_t(1) << uint64_t(63)));
  REQUIRE_FALSE(zb::will_next_power_of_two_overflow(uint64_t(1) << uint64_t(62)));
}

TEST_CASE("is_power_of_two_multiple_of") {
  // 0.
  REQUIRE(zb::is_multiple_of_power_of_two(0, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(0, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(0, 4));
  REQUIRE(zb::is_multiple_of_power_of_two(0, 8));

  // 1.
  REQUIRE(zb::is_multiple_of_power_of_two(1, 1));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(1, 2));

  // 2.
  REQUIRE(zb::is_multiple_of_power_of_two(2, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(2, 2));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(2, 4));

  // 4.
  REQUIRE(zb::is_multiple_of_power_of_two(4, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(4, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(4, 4));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(4, 8));

  // 6.
  REQUIRE(zb::is_multiple_of_power_of_two(6, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(6, 2));

  // 8.
  REQUIRE(zb::is_multiple_of_power_of_two(8, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(8, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(8, 4));
  REQUIRE(zb::is_multiple_of_power_of_two(8, 8));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(8, 16));

  // 12.
  REQUIRE(zb::is_multiple_of_power_of_two(12, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(12, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(12, 4));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(12, 8));

  // 16.
  REQUIRE(zb::is_multiple_of_power_of_two(16, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(16, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(16, 4));
  REQUIRE(zb::is_multiple_of_power_of_two(16, 8));
  REQUIRE(zb::is_multiple_of_power_of_two(16, 16));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(16, 32));

  // 24.
  REQUIRE(zb::is_multiple_of_power_of_two(24, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(24, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(24, 4));
  REQUIRE(zb::is_multiple_of_power_of_two(24, 8));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(24, 16));

  // 32.
  REQUIRE(zb::is_multiple_of_power_of_two(32, 1));
  REQUIRE(zb::is_multiple_of_power_of_two(32, 2));
  REQUIRE(zb::is_multiple_of_power_of_two(32, 4));
  REQUIRE(zb::is_multiple_of_power_of_two(32, 8));
  REQUIRE(zb::is_multiple_of_power_of_two(32, 16));
  REQUIRE(zb::is_multiple_of_power_of_two(32, 32));
  REQUIRE_FALSE(zb::is_multiple_of_power_of_two(32, 64));
}

TEST_CASE("round_up") {
  REQUIRE(zb::round_up(1, 20) == 20);
  REQUIRE(zb::round_up(10, 20) == 20);
  REQUIRE(zb::round_up(19, 20) == 20);
  REQUIRE(zb::round_up(20, 20) == 20);
  REQUIRE(zb::round_up(21, 20) == 40);

  REQUIRE(zb::round_up(0, 8) == 0);
  REQUIRE(zb::round_up(1, 8) == 8);
  REQUIRE(zb::round_up(2, 8) == 8);
  REQUIRE(zb::round_up(3, 8) == 8);
  REQUIRE(zb::round_up(4, 8) == 8);
  REQUIRE(zb::round_up(5, 8) == 8);
  REQUIRE(zb::round_up(6, 8) == 8);
  REQUIRE(zb::round_up(7, 8) == 8);
  REQUIRE(zb::round_up(8, 8) == 8);
  REQUIRE(zb::round_up(9, 8) == 16);

  REQUIRE(zb::round_up_to_multiple_of_power_of_two(0, 8) == 0);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(1, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(2, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(3, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(4, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(5, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(6, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(7, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(8, 8) == 8);
  REQUIRE(zb::round_up_to_multiple_of_power_of_two(9, 8) == 16);
}

} // namespace.
