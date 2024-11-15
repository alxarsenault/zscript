#include <catch2.h>
#include <zbase/sys/error_code.h>

TEST_CASE("error_code") {

  zb::status_result st;
  REQUIRE(st);

  zb::error_result err = st;
  REQUIRE(!err);

  zb::error_result err2;
  REQUIRE(!err2);

  {
    zb::optional_result<std::string> res;
    REQUIRE(res);
  }

  {
    zb::optional_result<std::string> res("Banana");
    REQUIRE(res);
  }

  {
    zb::optional_result<std::string> res(zb::error_code::unknown);
    REQUIRE(!res);
  }
}

struct my_error_descriptor {
  enum class enum_type { good, bad, unknown };

  static constexpr enum_type default_value = enum_type::good;

  static inline constexpr bool is_valid(enum_type v) noexcept { return v == enum_type::good; }

  static inline constexpr const char* to_string(enum_type v) noexcept {
    switch (v) {
    case enum_type::good:
      return "good";
    case enum_type::bad:
      return "bad";
    case enum_type::unknown:
      return "unknown";
    }

    return "unknown";
  }
};

TEST_CASE("generic_error_code") {

  using my_error_result = zb::generic_error_result<my_error_descriptor>;

  my_error_result res{};
  REQUIRE(!res);

  constexpr auto fct = []() -> my_error_result { return my_error_descriptor::enum_type::bad; };

  res = fct();
  REQUIRE(res);
  REQUIRE(std::string_view(res.message()) == "bad");
}
