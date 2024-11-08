#include <ztests/ztests.h>
#include <zbase/strings/unicode.h>

namespace common {
inline constexpr const char* c_string();
inline constexpr const char16_t* u16_string();
inline constexpr const char32_t* u32_string();
inline constexpr const wchar_t* w_string();

inline constexpr const char8_t* u8_string();

} // namespace common.

TEST_CASE("dksldksldks") {

  std::string_view str = "Abecι";

  const char* start = str.data();
  const char* end = str.data() + str.size();

  std::vector<uint32_t> cc;

  while (start < end) {
    cc.push_back(static_cast<uint32_t>(zb::unicode::next_u8_to_u32(start)));
  }
}

TEST_CASE("CHINA") {
  std::string s = "的";
  REQUIRE(s.size() == 3);
  REQUIRE(zb::unicode::utf8_length(s.c_str(), s.size()) == 1);

  s += s;
  REQUIRE_EQ(s.size(), 6);
  REQUIRE_EQ(zb::unicode::utf8_length(s.c_str(), s.size()), 2);

  s = "香港增补字符集";
  REQUIRE_EQ(s.size(), 21);
  REQUIRE_EQ(zb::unicode::utf8_length(s.c_str(), s.size()), 7);
}

TEST_CASE("utf", "traits") {
  // is_utf_char_type.
  REQUIRE(zb::is_utf_char_type<char>::value);
  REQUIRE(zb::is_utf_char_type<const char>::value);
  REQUIRE(zb::is_utf_char_type<char8_t>::value);
  REQUIRE(zb::is_utf_char_type<const char8_t>::value);
  REQUIRE(zb::is_utf_char_type<char16_t>::value);
  REQUIRE(zb::is_utf_char_type<const char16_t>::value);
  REQUIRE(zb::is_utf_char_type<char32_t>::value);
  REQUIRE(zb::is_utf_char_type<const char32_t>::value);
  REQUIRE(zb::is_utf_char_type<wchar_t>::value);
  REQUIRE(zb::is_utf_char_type<const wchar_t>::value);

  REQUIRE_FALSE(zb::is_utf_char_type<char&>::value);
  REQUIRE_FALSE(zb::is_utf_char_type<const char&>::value);
  REQUIRE_FALSE(zb::is_utf_char_type<char*>::value);
  REQUIRE_FALSE(zb::is_utf_char_type<unsigned char>::value);
  REQUIRE_FALSE(zb::is_utf_char_type<double>::value);

  // is_utf_char_size.
  REQUIRE(zb::is_utf_char_size<sizeof(char)>::value);
  REQUIRE(zb::is_utf_char_size<sizeof(char16_t)>::value);
  REQUIRE(zb::is_utf_char_size<sizeof(char32_t)>::value);
  REQUIRE(zb::is_utf_char_size<sizeof(wchar_t)>::value);
  REQUIRE_FALSE(zb::is_utf_char_size<3>::value);
  REQUIRE_FALSE(zb::is_utf_char_size<8>::value);

  // utf_encoding_of.
  REQUIRE(zb::utf_encoding_of<char>::value == zb::char_encoding::utf8);
  REQUIRE(zb::utf_encoding_of<char8_t>::value == zb::char_encoding::utf8);
  REQUIRE(zb::utf_encoding_of<char16_t>::value == zb::char_encoding::utf16);
  REQUIRE(zb::utf_encoding_of<char32_t>::value == zb::char_encoding::utf32);
  REQUIRE(zb::utf_encoding_of<wchar_t>::value
      == (sizeof(wchar_t) == sizeof(char16_t) ? zb::char_encoding::utf16 : zb::char_encoding::utf32));

  // utf_encoding_size.
  REQUIRE(zb::utf_encoding_size<zb::char_encoding::utf8>::value == sizeof(char));
  REQUIRE(zb::utf_encoding_size<zb::char_encoding::utf16>::value == sizeof(char16_t));
  REQUIRE(zb::utf_encoding_size<zb::char_encoding::utf32>::value == sizeof(char32_t));

  // string_char_type.
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::string>, char>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::string_view>, char>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char*>, char>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<char*>, char>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char(&)[]>, char>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const std::string&>, char>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::string&&>, char>));
  REQUIRE_FALSE((std::is_same_v<zb::string_char_type_t<std::string*>, char>));
  REQUIRE_FALSE((std::is_same_v<zb::string_char_type_t<char>, char>));
  REQUIRE_FALSE((std::is_same_v<zb::string_char_type_t<char&>, char>));

  REQUIRE((std::is_same_v<zb::string_char_type_t<std::u8string>, char8_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::u8string_view>, char8_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char8_t*>, char8_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char8_t(&)[]>, char8_t>));

  REQUIRE((std::is_same_v<zb::string_char_type_t<std::u16string>, char16_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::u16string_view>, char16_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char16_t*>, char16_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char16_t(&)[]>, char16_t>));

  REQUIRE((std::is_same_v<zb::string_char_type_t<std::u32string>, char32_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::u32string_view>, char32_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char32_t*>, char32_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const char32_t(&)[]>, char32_t>));

  REQUIRE((std::is_same_v<zb::string_char_type_t<std::wstring>, wchar_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<std::wstring_view>, wchar_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const wchar_t*>, wchar_t>));
  REQUIRE((std::is_same_v<zb::string_char_type_t<const wchar_t(&)[]>, wchar_t>));
  //
}

TEST_CASE("utf utf_cvt_size") {
  std::string s = common::c_string();
  std::u8string s8 = common::u8_string();
  std::u16string s16 = common::u16_string();
  std::u32string s32 = common::u32_string();
  std::wstring sw = common::w_string();

  REQUIRE_EQ(zb::unicode::convert_size<char>(s), s.size());
  REQUIRE_EQ(zb::unicode::convert_size<char8_t>(s), s8.size());
  REQUIRE_EQ(zb::unicode::convert_size<char16_t>(s), s16.size());
  REQUIRE_EQ(zb::unicode::convert_size<char32_t>(s), s32.size());
  REQUIRE_EQ(zb::unicode::convert_size<wchar_t>(s), sw.size());

  REQUIRE_EQ(zb::unicode::convert_size<char>(s8), s.size());
  REQUIRE_EQ(zb::unicode::convert_size<char8_t>(s8), s8.size());
  REQUIRE_EQ(zb::unicode::convert_size<char16_t>(s8), s16.size());
  REQUIRE_EQ(zb::unicode::convert_size<char32_t>(s8), s32.size());
  REQUIRE_EQ(zb::unicode::convert_size<wchar_t>(s8), sw.size());

  REQUIRE_EQ(zb::unicode::convert_size<char>(s16), s.size());
  REQUIRE_EQ(zb::unicode::convert_size<char8_t>(s16), s8.size());
  REQUIRE_EQ(zb::unicode::convert_size<char16_t>(s16), s16.size());
  REQUIRE_EQ(zb::unicode::convert_size<char32_t>(s16), s32.size());
  REQUIRE_EQ(zb::unicode::convert_size<wchar_t>(s16), sw.size());

  REQUIRE_EQ(zb::unicode::convert_size<char>(s32), s.size());
  REQUIRE_EQ(zb::unicode::convert_size<char8_t>(s32), s8.size());
  REQUIRE_EQ(zb::unicode::convert_size<char16_t>(s32), s16.size());
  REQUIRE_EQ(zb::unicode::convert_size<char32_t>(s32), s32.size());
  REQUIRE_EQ(zb::unicode::convert_size<wchar_t>(s32), sw.size());

  REQUIRE_EQ(zb::unicode::convert_size<char>(sw), s.size());
  REQUIRE_EQ(zb::unicode::convert_size<char8_t>(sw), s8.size());
  REQUIRE_EQ(zb::unicode::convert_size<char16_t>(sw), s16.size());
  REQUIRE_EQ(zb::unicode::convert_size<char32_t>(sw), s32.size());
  REQUIRE_EQ(zb::unicode::convert_size<wchar_t>(sw), sw.size());
}
//
TEST_CASE("utf length") {
  std::string s = common::c_string();
  std::u8string s8 = common::u8_string();
  std::u16string s16 = common::u16_string();
  std::u32string s32 = common::u32_string();

  // zb::print(zb::string_view(s16));

  REQUIRE_EQ(zb::unicode::length(s), s32.size());
  REQUIRE_EQ(zb::unicode::length(s8), s32.size());
  REQUIRE_EQ(zb::unicode::length(s16), s32.size());
}

TEST_CASE("utf utf_cvt") {
  {
    std::string in = common::c_string();
    std::string s = zb::unicode::convert(in);
    std::u8string s8 = zb::unicode::convert(in);
    std::u16string s16 = zb::unicode::convert(in);
    std::u32string s32 = zb::unicode::convert(in);
    std::wstring sw = zb::unicode::convert(in);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  {
    std::u8string in = common::u8_string();
    std::string s = zb::unicode::convert(in);
    std::u8string s8 = zb::unicode::convert(in);
    std::u16string s16 = zb::unicode::convert(in);
    std::u32string s32 = zb::unicode::convert(in);
    std::wstring sw = zb::unicode::convert(in);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  {
    std::u16string in = common::u16_string();
    std::string s = zb::unicode::convert(in);
    std::u8string s8 = zb::unicode::convert(in);
    std::u16string s16 = zb::unicode::convert(in);
    std::u32string s32 = zb::unicode::convert(in);
    std::wstring sw = zb::unicode::convert(in);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  {
    std::u32string in = common::u32_string();
    std::string s = zb::unicode::convert(in);
    std::u8string s8 = zb::unicode::convert(in);
    std::u16string s16 = zb::unicode::convert(in);
    std::u32string s32 = zb::unicode::convert(in);
    std::wstring sw = zb::unicode::convert(in);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  {
    std::wstring in = common::w_string();
    std::string s = zb::unicode::convert(in);
    std::u8string s8 = zb::unicode::convert(in);
    std::u16string s16 = zb::unicode::convert(in);
    std::u32string s32 = zb::unicode::convert(in);
    std::wstring sw = zb::unicode::convert(in);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }
}

TEST_CASE("utf impl_copy") {
  {
    std::string in = common::c_string();
    std::string s;
    std::u8string s8;
    std::u16string s16;
    std::u32string s32;
    std::wstring sw;

    zb::unicode::append_to(in, s);

    // zb::copy(in, zb::back_inserter(s));

    zb::unicode::append_to(in, s8);
    zb::unicode::append_to(in, s16);
    zb::unicode::append_to(in, s32);
    zb::unicode::append_to(in, sw);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  // #if FST_HAS_CHAR8_T

  {
    std::u8string in = common::u8_string();
    std::string s;
    std::u8string s8;
    std::u16string s16;
    std::u32string s32;
    std::wstring sw;

    zb::unicode::append_to(in, s);
    zb::unicode::append_to(in, s8);
    zb::unicode::append_to(in, s16);
    zb::unicode::append_to(in, s32);
    zb::unicode::append_to(in, sw);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }
  // #endif

  {
    std::u16string in = common::u16_string();
    std::string s;
    std::u8string s8;
    std::u16string s16;
    std::u32string s32;
    std::wstring sw;

    zb::unicode::append_to(in, s);
    zb::unicode::append_to(in, s8);
    zb::unicode::append_to(in, s16);
    zb::unicode::append_to(in, s32);
    zb::unicode::append_to(in, sw);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  {
    std::u32string in = common::u32_string();
    std::string s;
    std::u8string s8;
    std::u16string s16;
    std::u32string s32;
    std::wstring sw;

    zb::unicode::append_to(in, s);
    zb::unicode::append_to(in, s8);
    zb::unicode::append_to(in, s16);
    zb::unicode::append_to(in, s32);
    zb::unicode::append_to(in, sw);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }

  {
    std::wstring in = common::w_string();
    std::string s;
    std::u8string s8;
    std::u16string s16;
    std::u32string s32;
    std::wstring sw;

    zb::unicode::append_to(in, s);
    zb::unicode::append_to(in, s8);
    zb::unicode::append_to(in, s16);
    zb::unicode::append_to(in, s32);
    zb::unicode::append_to(in, sw);

    REQUIRE(s == common::c_string());
    REQUIRE(s8 == common::u8_string());
    REQUIRE(s16 == common::u16_string());
    REQUIRE(s32 == common::u32_string());
    REQUIRE(sw == common::w_string());
  }
}
// //
TEST_CASE("utf u8_iterator") {

  {
    std::string s = common::c_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::string_view c : zb::unicode::iterate(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u8string s = common::u8_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::u8string_view c : zb::unicode::iterate(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u16string s = common::u16_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::u16string_view c : zb::unicode::iterate(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::wstring s = common::w_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::wstring_view c : zb::unicode::iterate(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::string s = common::c_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::string_view c : zb::unicode::iterate_as<char>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u16string s = common::u16_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::string_view c : zb::unicode::iterate_as<char>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u32string s = common::u32_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::string_view c : zb::unicode::iterate_as<char>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u32string s = common::u32_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::u16string_view c : zb::unicode::iterate_as<char16_t>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u32string s = common::u32_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::u32string_view c : zb::unicode::iterate_as<char32_t>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u32string s = common::u32_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::wstring_view c : zb::unicode::iterate_as<wchar_t>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u16string s = common::u16_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::u16string_view c : zb::unicode::iterate_as<char16_t>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::string s = common::c_string();
    size_t length = zb::unicode::length(s);

    size_t count = 0;
    for (std::u16string_view c : zb::unicode::iterate_as<char16_t>(s)) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::string s = common::c_string();
    size_t count = 0;

    for (std::string_view c : zb::unicode::iterate_as<char>(std::string_view(s).substr(0, 500))) {
      (void)c;
      count++;
    }
    REQUIRE_EQ(count, zb::unicode::length(std::string_view(s).substr(0, 500)));
  }

  {
    std::string s = common::c_string();
    size_t length = zb::unicode::length(s);
    size_t count = 0;

    for (auto it = zb::unicode::iterator(s.begin()); it != zb::unicode::iterator(s.end()); ++it) {
      count++;
    }
    REQUIRE_EQ(count, length);
  }

  {
    std::u16string s = common::u16_string();
    size_t length = zb::unicode::length(s);
    size_t count = 0;

    for (auto it = zb::unicode::iterator(s.begin()); it != zb::unicode::iterator(s.end()); ++it) {
      count++;
    }
    REQUIRE_EQ(count, length);
  }
}

// TEST_CASE("utf-string-view string_view")
// {
//     {
//         REQUIRE_EQ(zb::utf_string_view(common::c_string()).count(),
//         zb::utf_string_view(common::w_string()).count());
//         REQUIRE_EQ(zb::utf_string_view(common::c_string()).count(),
//         zb::utf_string_view(common::w_string()).to_utf32().size());
//     }

//     {
//         std::wstring a = zb::unicode::to_wide(zb::utf_string_view(common::c_string()));
//         std::wstring aa = zb::unicode::to_wide(common::c_string());
//         std::wstring b = common::w_string();
//         REQUIRE_EQ(a.size(), b.size());
//         REQUIRE_EQ(aa.size(), b.size());
//     }

//     {
//         std::string a = zb::unicode::to_utf8(common::c_string());
//         std::string b = zb::unicode::to_utf8(common::w_string());
//         REQUIRE_EQ(a.size(), b.size());
//     }

//     {
//         zb::utf_string_view a(common::c_string());

//         REQUIRE_NE(a.u8cstr(), nullptr);
//         REQUIRE_EQ(a.u16cstr(), nullptr);
//         REQUIRE_EQ(a.u32cstr(), nullptr);
//         REQUIRE_EQ(a.wcstr(), nullptr);

//         REQUIRE_FALSE(a.u8str().empty());
//         REQUIRE_FALSE(a.u16str().empty());
//         REQUIRE_FALSE(a.u32str().empty());
//         REQUIRE_FALSE(a.wstr().empty());

//         REQUIRE_FALSE(a.u8view().empty());
//         REQUIRE(a.u16view().empty());
//         REQUIRE(a.u32view().empty());
//         REQUIRE(a.wview().empty());

//         REQUIRE_EQ(a.u8str().size(), a.size());
//         REQUIRE_EQ(a.u8view().size(), a.size());
//     }

//     {
//         zb::utf_string_view a(common::w_string());

//         REQUIRE_EQ(a.u8cstr(), nullptr);
//         REQUIRE_NE(a.wcstr(), nullptr);

//         REQUIRE_FALSE(a.u8str().empty());
//         REQUIRE_FALSE(a.wstr().empty());

//         REQUIRE(a.u8view().empty());
//         REQUIRE_FALSE(a.wview().empty());

//         REQUIRE_EQ(a.wstr().size(), a.size());
//         REQUIRE_EQ(a.wview().size(), a.size());
//     }
// }

#define UTF_CONCAT1(_X, _Y) _X##_Y
#define UTF_CONCAT(_X, _Y) UTF_CONCAT1(_X, _Y)

#define UTF_STRING_TYPE(prefix) UTF_CONCAT(prefix, RAW_STRING)

// clang-format off
#define RAW_STRING "Original by Markus Kuhn, adapted for HTML by Martin Dürst.\n"\
"\n"\
"UTF-8 encoded sample plain-text file\n"\
"‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n"\
"\n"\
"Markus Kuhn [ˈmaʳkʊs kuːn] <mkuhn@acm.org> — 1999-08-20\n"\
"\n"\
"\n"\
"The ASCII compatible UTF-8 encoding of ISO 10646 and Unicode\n"\
"plain-text files is defined in RFC 2279 and in ISO 10646-1 Annex R.\n"\
"\n"\
"\n"\
"Using Unicode/UTF-8, you can write in emails and source code things such as\n"\
"\n"\
"Mathematics and Sciences:\n"\
"\n"\
"  ∮ E⋅da = Q,  n → ∞, ∑ f(i) = ∏ g(i), ∀x∈ℝ: ⌈x⌉ = −⌊−x⌋, α ∧ ¬β = ¬(¬α ∨ β),\n"\
"\n"\
"  ℕ ⊆ ℕ₀ ⊂ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂ, ⊥ < a ≠ b ≡ c ≤ d ≪ ⊤ ⇒ (A ⇔ B),\n"\
"\n"\
"  2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm\n"\
"\n"\
"Linguistics and dictionaries:\n"\
"\n"\
"  ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn\n"\
"  Y [ˈʏpsilɔn], Yen [jɛn], Yoga [ˈjoːgɑ]\n"\
"\n"\
"APL:\n"\
"\n"\
"  ((V⍳V)=⍳⍴V)/V←,V    ⌷←⍳→⍴∆∇⊃‾⍎⍕⌈\n"\
"\n"\
"Nicer typography in plain text files:\n"\
"\n"\
"  ╔══════════════════════════════════════════╗\n"\
"  ║                                          ║\n"\
"  ║   • ‘single’ and “double” quotes         ║\n"\
"  ║                                          ║\n"\
"  ║   • Curly apostrophes: “We’ve been here” ║\n"\
"  ║                                          ║\n"\
"  ║   • Latin-1 apostrophe and accents: '´`  ║\n"\
"  ║                                          ║\n"\
"  ║   • ‚deutsche‘ „Anführungszeichen“       ║\n"\
"  ║                                          ║\n"\
"  ║   • †, ‡, ‰, •, 3–4, —, −5/+5, ™, …      ║\n"\
"  ║                                          ║\n"\
"  ║   • ASCII safety test: 1lI|, 0OD, 8B     ║\n"\
"  ║                      ╭─────────╮         ║\n"\
"  ║   • the euro symbol: │ 14.95 € │         ║\n"\
"  ║                      ╰─────────╯         ║\n"\
"  ╚══════════════════════════════════════════╝\n"\
"\n"\
"Greek (in Polytonic):\n"\
"\n"\
"  The Greek anthem:\n"\
"\n"\
"  Σὲ γνωρίζω ἀπὸ τὴν κόψη\n"\
"  τοῦ σπαθιοῦ τὴν τρομερή,\n"\
"  σὲ γνωρίζω ἀπὸ τὴν ὄψη\n"\
"  ποὺ μὲ βία μετράει τὴ γῆ.\n"\
"\n"\
"  ᾿Απ᾿ τὰ κόκκαλα βγαλμένη\n"\
"  τῶν ῾Ελλήνων τὰ ἱερά\n"\
"  καὶ σὰν πρῶτα ἀνδρειωμένη\n"\
"  χαῖρε, ὦ χαῖρε, ᾿Ελευθεριά!\n"\
"\n"\
"  From a speech of Demosthenes in the 4th century BC:\n"\
"\n"\
"  Οὐχὶ ταὐτὰ παρίσταταί μοι γιγνώσκειν, ὦ ἄνδρες ᾿Αθηναῖοι,\n"\
"  ὅταν τ᾿ εἰς τὰ πράγματα ἀποβλέψω καὶ ὅταν πρὸς τοὺς\n"\
"  λόγους οὓς ἀκούω· τοὺς μὲν γὰρ λόγους περὶ τοῦ\n"\
"  τιμωρήσασθαι Φίλιππον ὁρῶ γιγνομένους, τὰ δὲ πράγματ᾿\n"\
"  εἰς τοῦτο προήκοντα,  ὥσθ᾿ ὅπως μὴ πεισόμεθ᾿ αὐτοὶ\n"\
"  πρότερον κακῶς σκέψασθαι δέον. οὐδέν οὖν ἄλλο μοι δοκοῦσιν\n"\
"  οἱ τὰ τοιαῦτα λέγοντες ἢ τὴν ὑπόθεσιν, περὶ ἧς βουλεύεσθαι,\n"\
"  οὐχὶ τὴν οὖσαν παριστάντες ὑμῖν ἁμαρτάνειν. ἐγὼ δέ, ὅτι μέν\n"\
"  ποτ᾿ ἐξῆν τῇ πόλει καὶ τὰ αὑτῆς ἔχειν ἀσφαλῶς καὶ Φίλιππον\n"\
"  τιμωρήσασθαι, καὶ μάλ᾿ ἀκριβῶς οἶδα· ἐπ᾿ ἐμοῦ γάρ, οὐ πάλαι\n"\
"  γέγονεν ταῦτ᾿ ἀμφότερα· νῦν μέντοι πέπεισμαι τοῦθ᾿ ἱκανὸν\n"\
"  προλαβεῖν ἡμῖν εἶναι τὴν πρώτην, ὅπως τοὺς συμμάχους\n"\
"  σώσομεν. ἐὰν γὰρ τοῦτο βεβαίως ὑπάρξῃ, τότε καὶ περὶ τοῦ\n"\
"  τίνα τιμωρήσεταί τις καὶ ὃν τρόπον ἐξέσται σκοπεῖν· πρὶν δὲ\n"\
"  τὴν ἀρχὴν ὀρθῶς ὑποθέσθαι, μάταιον ἡγοῦμαι περὶ τῆς\n"\
"  τελευτῆς ὁντινοῦν ποιεῖσθαι λόγον.\n"\
"\n"\
"  Δημοσθένους, Γ´ ᾿Ολυνθιακὸς\n"\
"\n"\
"Georgian:\n"\
"\n"\
"  From a Unicode conference invitation:\n"\
"\n"\
"  გთხოვთ ახლავე გაიაროთ რეგისტრაცია Unicode-ის მეათე საერთაშორისო\n"\
"  კონფერენციაზე დასასწრებად, რომელიც გაიმართება 10-12 მარტს,\n"\
"  ქ. მაინცში, გერმანიაში. კონფერენცია შეჰკრებს ერთად მსოფლიოს\n"\
"  ექსპერტებს ისეთ დარგებში როგორიცაა ინტერნეტი და Unicode-ი,\n"\
"  ინტერნაციონალიზაცია და ლოკალიზაცია, Unicode-ის გამოყენება\n"\
"  ოპერაციულ სისტემებსა, და გამოყენებით პროგრამებში, შრიფტებში,\n"\
"  ტექსტების დამუშავებასა და მრავალენოვან კომპიუტერულ სისტემებში.\n"\
"\n"\
"Russian:\n"\
"\n"\
"  From a Unicode conference invitation:\n"\
"\n"\
"  Зарегистрируйтесь сейчас на Десятую Международную Конференцию по\n"\
"  Unicode, которая состоится 10-12 марта 1997 года в Майнце в Германии.\n"\
"  Конференция соберет широкий круг экспертов по  вопросам глобального\n"\
"  Интернета и Unicode, локализации и интернационализации, воплощению и\n"\
"  применению Unicode в различных операционных системах и программных\n"\
"  приложениях, шрифтах, верстке и многоязычных компьютерных системах.\n"\
"\n"\
"Thai (UCS Level 2):\n"\
"\n"\
"  Excerpt from a poetry on The Romance of The Three Kingdoms (a Chinese\n"\
"  classic 'San Gua'):\n"\
"\n"\
"  [----------------------------|------------------------]\n"\
"    ๏ แผ่นดินฮั่นเสื่อมโทรมแสนสังเวช  พระปกเกศกองบู๊กู้ขึ้นใหม่\n"\
"  สิบสองกษัตริย์ก่อนหน้าแลถัดไป       สององค์ไซร้โง่เขลาเบาปัญญา\n"\
"    ทรงนับถือขันทีเป็นที่พึ่ง           บ้านเมืองจึงวิปริตเป็นนักหนา\n"\
"  โฮจิ๋นเรียกทัพทั่วหัวเมืองมา         หมายจะฆ่ามดชั่วตัวสำคัญ\n"\
"    เหมือนขับไสไล่เสือจากเคหา      รับหมาป่าเข้ามาเลยอาสัญ\n"\
"  ฝ่ายอ้องอุ้นยุแยกให้แตกกัน          ใช้สาวนั้นเป็นชนวนชื่นชวนใจ\n"\
"    พลันลิฉุยกุยกีกลับก่อเหตุ          ช่างอาเพศจริงหนาฟ้าร้องไห้\n"\
"  ต้องรบราฆ่าฟันจนบรรลัย           ฤๅหาใครค้ำชูกู้บรรลังก์ ฯ\n"\
"\n"\
"  (The above is a two-column text. If combining characters are handled\n"\
"  correctly, the lines of the second column should be aligned with the\n"\
"  | character above.)\n"\
"\n"\
"Ethiopian:\n"\
"\n"\
"  Proverbs in the Amharic language:\n"\
"\n"\
"  ሰማይ አይታረስ ንጉሥ አይከሰስ።\n"\
"  ብላ ካለኝ እንደአባቴ በቆመጠኝ።\n"\
"  ጌጥ ያለቤቱ ቁምጥና ነው።\n"\
"  ደሀ በሕልሙ ቅቤ ባይጠጣ ንጣት በገደለው።\n"\
"  የአፍ ወለምታ በቅቤ አይታሽም።\n"\
"  አይጥ በበላ ዳዋ ተመታ።\n"\
"  ሲተረጉሙ ይደረግሙ።\n"\
"  ቀስ በቀስ፥ ዕንቁላል በእግሩ ይሄዳል።\n"\
"  ድር ቢያብር አንበሳ ያስር።\n"\
"  ሰው እንደቤቱ እንጅ እንደ ጉረቤቱ አይተዳደርም።\n"\
"  እግዜር የከፈተውን ጉሮሮ ሳይዘጋው አይድርም።\n"\
"  የጎረቤት ሌባ፥ ቢያዩት ይስቅ ባያዩት ያጠልቅ።\n"\
"  ሥራ ከመፍታት ልጄን ላፋታት።\n"\
"  ዓባይ ማደሪያ የለው፥ ግንድ ይዞ ይዞራል።\n"\
"  የእስላም አገሩ መካ የአሞራ አገሩ ዋርካ።\n"\
"  ተንጋሎ ቢተፉ ተመልሶ ባፉ።\n"\
"  ወዳጅህ ማር ቢሆን ጨርስህ አትላሰው።\n"\
"  እግርህን በፍራሽህ ልክ ዘርጋ።\n"\
"\n"\
"Runes:\n"\
"\n"\
"  ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ ᚦᚫᛗ ᛚᚪᚾᛞᛖ ᚾᚩᚱᚦᚹᛖᚪᚱᛞᚢᛗ ᚹᛁᚦ ᚦᚪ ᚹᛖᛥᚫ\n"\
"\n"\
"  (Old English, which transcribed into Latin reads 'He cwaeth that he\n"\
"  bude thaem lande northweardum with tha Westsae.' and means 'He said\n"\
"  that he lived in the northern land near the Western Sea.')\n"\
"\n"\
"Braille:\n"\
"\n"\
"  ⡌⠁⠧⠑ ⠼⠁⠒  ⡍⠜⠇⠑⠹⠰⠎ ⡣⠕⠌\n"\
"\n"\
"  ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠙⠑⠁⠙⠒ ⠞⠕ ⠃⠑⠛⠔ ⠺⠊⠹⠲ ⡹⠻⠑ ⠊⠎ ⠝⠕ ⠙⠳⠃⠞\n"\
"  ⠱⠁⠞⠑⠧⠻ ⠁⠃⠳⠞ ⠹⠁⠞⠲ ⡹⠑ ⠗⠑⠛⠊⠌⠻ ⠕⠋ ⠙⠊⠎ ⠃⠥⠗⠊⠁⠇ ⠺⠁⠎\n"\
"  ⠎⠊⠛⠝⠫ ⠃⠹ ⠹⠑ ⠊⠇⠻⠛⠹⠍⠁⠝⠂ ⠹⠑ ⠊⠇⠻⠅⠂ ⠹⠑ ⠥⠝⠙⠻⠞⠁⠅⠻⠂\n"\
"  ⠁⠝⠙ ⠹⠑ ⠡⠊⠑⠋ ⠍⠳⠗⠝⠻⠲ ⡎⠊⠗⠕⠕⠛⠑ ⠎⠊⠛⠝⠫ ⠊⠞⠲ ⡁⠝⠙\n"\
"  ⡎⠊⠗⠕⠕⠛⠑⠰⠎ ⠝⠁⠍⠑ ⠺⠁⠎ ⠛⠕⠕⠙ ⠥⠏⠕⠝ ⠰⡡⠁⠝⠛⠑⠂ ⠋⠕⠗ ⠁⠝⠹⠹⠔⠛ ⠙⠑\n"\
"  ⠡⠕⠎⠑ ⠞⠕ ⠏⠥⠞ ⠙⠊⠎ ⠙⠁⠝⠙ ⠞⠕⠲\n"\
"\n"\
"  ⡕⠇⠙ ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠁⠎ ⠙⠑⠁⠙ ⠁⠎ ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲\n"\
"\n"\
"  ⡍⠔⠙⠖ ⡊ ⠙⠕⠝⠰⠞ ⠍⠑⠁⠝ ⠞⠕ ⠎⠁⠹ ⠹⠁⠞ ⡊ ⠅⠝⠪⠂ ⠕⠋ ⠍⠹\n"\
"  ⠪⠝ ⠅⠝⠪⠇⠫⠛⠑⠂ ⠱⠁⠞ ⠹⠻⠑ ⠊⠎ ⠏⠜⠞⠊⠊⠥⠇⠜⠇⠹ ⠙⠑⠁⠙ ⠁⠃⠳⠞\n"\
"  ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲ ⡊ ⠍⠊⠣⠞ ⠙⠁⠧⠑ ⠃⠑⠲ ⠔⠊⠇⠔⠫⠂ ⠍⠹⠎⠑⠇⠋⠂ ⠞⠕\n"\
"  ⠗⠑⠛⠜⠙ ⠁ ⠊⠕⠋⠋⠔⠤⠝⠁⠊⠇ ⠁⠎ ⠹⠑ ⠙⠑⠁⠙⠑⠌ ⠏⠊⠑⠊⠑ ⠕⠋ ⠊⠗⠕⠝⠍⠕⠝⠛⠻⠹\n"\
"  ⠔ ⠹⠑ ⠞⠗⠁⠙⠑⠲ ⡃⠥⠞ ⠹⠑ ⠺⠊⠎⠙⠕⠍ ⠕⠋ ⠳⠗ ⠁⠝⠊⠑⠌⠕⠗⠎\n"\
"  ⠊⠎ ⠔ ⠹⠑ ⠎⠊⠍⠊⠇⠑⠆ ⠁⠝⠙ ⠍⠹ ⠥⠝⠙⠁⠇⠇⠪⠫ ⠙⠁⠝⠙⠎\n"\
"  ⠩⠁⠇⠇ ⠝⠕⠞ ⠙⠊⠌⠥⠗⠃ ⠊⠞⠂ ⠕⠗ ⠹⠑ ⡊⠳⠝⠞⠗⠹⠰⠎ ⠙⠕⠝⠑ ⠋⠕⠗⠲ ⡹⠳\n"\
"  ⠺⠊⠇⠇ ⠹⠻⠑⠋⠕⠗⠑ ⠏⠻⠍⠊⠞ ⠍⠑ ⠞⠕ ⠗⠑⠏⠑⠁⠞⠂ ⠑⠍⠏⠙⠁⠞⠊⠊⠁⠇⠇⠹⠂ ⠹⠁⠞\n"\
"  ⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠁⠎ ⠙⠑⠁⠙ ⠁⠎ ⠁ ⠙⠕⠕⠗⠤⠝⠁⠊⠇⠲\n"\
"\n"\
"  (The first couple of paragraphs of \"A Christmas Carol\" by Dickens)\n"\
"\n"\
"Compact font selection example text:\n"\
"\n"\
"  ABCDEFGHIJKLMNOPQRSTUVWXYZ /0123456789\n"\
"  abcdefghijklmnopqrstuvwxyz £©µÀÆÖÞßéöÿ\n"\
"  –—‘“”„†•…‰™œŠŸž€ ΑΒΓΔΩαβγδω АБВГДабвгд\n"\
"  ∀∂∈ℝ∧∪≡∞ ↑↗↨↻⇣ ┐┼╔╘░►☺♀ ﬁ�⑀₂ἠḂӥẄɐː⍎אԱა\n"\
"\n"\
"Greetings in various languages:\n"\
"\n"\
"  Hello world, Καλημέρα κόσμε, コンニチハ\n"\
"\n"\
"Box drawing alignment tests:                                          █\n"\
"                                                                      ▉\n"\
"  ╔══╦══╗  ┌──┬──┐  ╭──┬──╮  ╭──┬──╮  ┏━━┳━━┓  ┎┒┏┑   ╷  ╻ ┏┯┓ ┌┰┐    ▊ ╱╲╱╲╳╳╳\n"\
"  ║┌─╨─┐║  │╔═╧═╗│  │╒═╪═╕│  │╓─╁─╖│  ┃┌─╂─┐┃  ┗╃╄┙  ╶┼╴╺╋╸┠┼┨ ┝╋┥    ▋ ╲╱╲╱╳╳╳\n"\
"  ║│╲ ╱│║  │║   ║│  ││ │ ││  │║ ┃ ║│  ┃│ ╿ │┃  ┍╅╆┓   ╵  ╹ ┗┷┛ └┸┘    ▌ ╱╲╱╲╳╳╳\n"\
"  ╠╡ ╳ ╞╣  ├╢   ╟┤  ├┼─┼─┼┤  ├╫─╂─╫┤  ┣┿╾┼╼┿┫  ┕┛┖┚     ┌┄┄┐ ╎ ┏┅┅┓ ┋ ▍ ╲╱╲╱╳╳╳\n"\
"  ║│╱ ╲│║  │║   ║│  ││ │ ││  │║ ┃ ║│  ┃│ ╽ │┃  ░░▒▒▓▓██ ┊  ┆ ╎ ╏  ┇ ┋ ▎\n"\
"  ║└─╥─┘║  │╚═╤═╝│  │╘═╪═╛│  │╙─╀─╜│  ┃└─╂─┘┃  ░░▒▒▓▓██ ┊  ┆ ╎ ╏  ┇ ┋ ▏\n"\
"  ╚══╩══╝  └──┴──┘  ╰──┴──╯  ╰──┴──╯  ┗━━┻━━┛           └╌╌┘ ╎ ┗╍╍┛ ┋  ▁▂▃▄▅▆▇█\n"
// clang-format on

namespace common {
inline constexpr const char* c_string() { return RAW_STRING; }
inline constexpr const char16_t* u16_string() { return UTF_STRING_TYPE(u); }
inline constexpr const char32_t* u32_string() { return UTF_STRING_TYPE(U); }
inline constexpr const wchar_t* w_string() { return UTF_STRING_TYPE(L); }

// #if FST_HAS_CHAR8_T
inline constexpr const char8_t* u8_string() { return UTF_STRING_TYPE(u8); }
// #endif
} // namespace common.
