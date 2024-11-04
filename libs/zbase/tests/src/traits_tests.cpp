#include <ztests/ztests.h>
#include <zbase/utility/traits.h>

template <class _CharT, class Str>
inline constexpr bool is_basic_sv_conv1(Str str) {
  return zb::is_basic_string_view_convertible<_CharT, Str>::value;
}

template <class _CharT, class Str>
inline constexpr bool is_basic_sv_conv2(const Str& str) {
  return zb::is_basic_string_view_convertible<_CharT, Str>::value;
}

template <class _CharT, class Str>
inline constexpr bool is_basic_sv_conv3(Str&& str) {
  return zb::is_basic_string_view_convertible<_CharT, Str>::value;
}

template <class _CharT, size_t N>
inline constexpr bool is_basic_sv_conv4(const _CharT (&str)[N]) {
  return zb::is_basic_string_view_convertible<_CharT, decltype(str)>::value;
}

template <class _CharT>
struct sallocator : std::allocator<_CharT> {
  using base_allocator = std::allocator<_CharT>;
  //  using base_allocator::difference_type;
  //  using base_allocator::is_always_equal;
  //  using base_allocator::propagate_on_container_move_assignment;
  //  using base_allocator::size_type;
  //  using base_allocator::value_type;
};

TEST_CASE("zb::traits") {
  // char.
  REQUIRE(zb::is_basic_string_view_convertible_v<char, const char*>);
  REQUIRE(zb::is_basic_string_view_convertible_v<char, char*>);
  REQUIRE(zb::is_basic_string_view_convertible_v<char, std::string_view>);
  REQUIRE(zb::is_basic_string_view_convertible_v<char, std::string>);
  REQUIRE(zb::is_basic_string_view_convertible_v<char,
      std::basic_string<char, std::char_traits<char>, sallocator<char>>>);
  REQUIRE(zb::is_basic_string_view_convertible_v<char, char[]>);
  REQUIRE(zb::is_basic_string_view_convertible_v<char, char[10]>);

  REQUIRE(zb::is_string_view_convertible_v<const char*>);
  REQUIRE(zb::is_string_view_convertible_v<char*>);
  REQUIRE(zb::is_string_view_convertible_v<std::string_view>);
  REQUIRE(zb::is_string_view_convertible_v<std::string>);

  // wchar_t.
  REQUIRE(zb::is_basic_string_view_convertible_v<wchar_t, const wchar_t*>);
  REQUIRE(zb::is_basic_string_view_convertible_v<wchar_t, wchar_t*>);
  REQUIRE(zb::is_basic_string_view_convertible_v<wchar_t, std::wstring_view>);
  REQUIRE(zb::is_basic_string_view_convertible_v<wchar_t, std::wstring>);
  REQUIRE(zb::is_basic_string_view_convertible_v<wchar_t,
      std::basic_string<wchar_t, std::char_traits<wchar_t>, sallocator<wchar_t>>>);

  REQUIRE(zb::is_wstring_view_convertible_v<const wchar_t*>);
  REQUIRE(zb::is_wstring_view_convertible_v<wchar_t*>);
  REQUIRE(zb::is_wstring_view_convertible_v<std::wstring_view>);
  REQUIRE(zb::is_wstring_view_convertible_v<std::wstring>);

  REQUIRE(is_basic_sv_conv1<char>("john"));
  REQUIRE(is_basic_sv_conv2<char>("john"));
  REQUIRE(is_basic_sv_conv3<char>("john"));
  REQUIRE(is_basic_sv_conv4<char>("john"));

  REQUIRE(is_basic_sv_conv1<wchar_t>(L"john"));
  REQUIRE(is_basic_sv_conv2<wchar_t>(L"john"));
  REQUIRE(is_basic_sv_conv3<wchar_t>(L"john"));
  REQUIRE(is_basic_sv_conv4<wchar_t>(L"john"));
}
