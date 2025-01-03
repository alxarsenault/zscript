#include "zstring_delegate.h"
#include "zvirtual_machine.h"
#include <zscript/base/strings/unicode.h>

namespace zs {

#define ZS_STRING_SET_ARG_ERROR(fct_name) vm.set_error("Invalid string argument in string::" fct_name "().")

#define ZS_STRING_GET(fct_name)                                         \
  object& obj = vm[0];                                                  \
  if (!obj.is_string()) {                                               \
    vm.set_error("Invalid string argument in string::" fct_name "()."); \
    return -1;                                                          \
  }                                                                     \
  std::string_view str = obj.get_string_unchecked()

#define ZS_STRING_BEGIN_IMPL(fct_name, n_args)                                \
  if (vm.stack_size() != n_args) {                                            \
    vm.set_error("Invalid number of arguments in string::" fct_name "().\n"); \
    return -1;                                                                \
  }                                                                           \
  ZS_STRING_GET(fct_name)

namespace {

  template <class CharType>
  inline std::basic_string_view<CharType> sv_from_char(const CharType& c) noexcept {
    return std::basic_string_view<CharType>(&c, 1);
  }

  inline char32_t get_first_letter(std::string_view str) {
    return str.empty() ? 0 : (*zb::unicode::iterate_as<char32_t>(str).begin())[0];
  }

  inline zs::string to_lower_string(zs::engine* eng, std::string_view str) {
    zs::string lower_str((zs::allocator<char>(eng)));

    for (std::u32string_view s : zb::unicode::iterate_as<char32_t>(str)) {
      zb::unicode::append_to(sv_from_char<char32_t>(zb::unicode::to_lower(s[0])), lower_str);
    }

    return lower_str;
  }

  inline zs::string to_upper_string(zs::engine* eng, std::string_view str) {
    zs::string upper_str((zs::allocator<char>(eng)));

    for (std::u32string_view s : zb::unicode::iterate_as<char32_t>(str)) {
      zb::unicode::append_to(sv_from_char<char32_t>(zb::unicode::to_upper(s[0])), upper_str);
    }

    return upper_str;
  }

  inline zs::object split_words_to_array_object(zs::engine* eng, std::string_view str) {
    constexpr std::string_view delims{ " \t\r\n" };

    zs::object arr_obj = zs::_a(eng, 0);
    zs::array_object& arr = arr_obj.as_array();

    size_t beg = 0;
    size_t pos = 0;
    while ((beg = str.find_first_not_of(delims, pos)) != std::string_view::npos) {
      pos = str.find_first_of(delims, beg + 1);
      if (std::string_view s = str.substr(beg, pos - beg); !s.empty()) {
        arr.push_back(zs::_s(eng, s));
      }
    }

    return arr_obj;
  }

  inline zs::vector<std::string_view> split_words_to_vector(zs::engine* eng, std::string_view str) {
    constexpr std::string_view delims{ " \t\r\n" };

    zs::vector<std::string_view> vec((zs::allocator<std::string_view>(eng)));

    size_t beg = 0;
    size_t pos = 0;
    while ((beg = str.find_first_not_of(delims, pos)) != std::string_view::npos) {
      pos = str.find_first_of(delims, beg + 1);
      if (std::string_view s = str.substr(beg, pos - beg); !s.empty()) {
        vec.push_back(s);
      }
    }

    return vec;
  }

  inline zs::vector<std::string_view> split_words_to_vector(
      zs::engine* eng, std::string_view str, std::string_view delims) {
    zs::vector<std::string_view> vec((zs::allocator<std::string_view>(eng)));

    size_t beg = 0;
    size_t pos = 0;
    while ((beg = str.find_first_not_of(delims, pos)) != std::string_view::npos) {
      pos = str.find_first_of(delims, beg + 1);
      if (std::string_view s = str.substr(beg, pos - beg); !s.empty()) {
        vec.push_back(s);
      }
    }

    return vec;
  }

  zs::string replace_all(zs::engine* eng, std::string_view str, std::string_view from, std::string_view to) {
    zs::string output_str((zs::allocator<char>(eng)));
    output_str.reserve(str.length());

    std::string_view::size_type last_pos = 0;
    std::string_view::size_type find_pos;

    while ((find_pos = str.find(from, last_pos)) != std::string_view::npos) {
      output_str.append(str, last_pos, find_pos - last_pos);
      output_str.append(to);
      last_pos = find_pos + from.length();
    }

    // Care for the rest after last occurrence.
    output_str.append(str.substr(last_pos));

    return output_str;
  }

  zs::string replace_first(
      zs::engine* eng, std::string_view str, std::string_view from, std::string_view to) {
    zs::string output_str((zs::allocator<char>(eng)));
    output_str.reserve(str.length());

    std::string_view::size_type last_pos = 0;

    if (std::string_view::size_type find_pos = str.find(from); find_pos != std::string_view::npos) {
      output_str.append(str, last_pos, find_pos - last_pos);
      output_str.append(to);
      last_pos = find_pos + from.length();
    }

    // Care for the rest after last occurrence.
    output_str.append(str.substr(last_pos));

    return output_str;
  }

  zs::string replace_last(zs::engine* eng, std::string_view str, std::string_view from, std::string_view to) {
    zs::string output_str((zs::allocator<char>(eng)));
    output_str.reserve(str.length());

    std::string_view::size_type last_pos = 0;

    if (std::string_view::size_type find_pos = str.rfind(from); find_pos != std::string_view::npos) {
      output_str.append(str, last_pos, find_pos - last_pos);
      output_str.append(to);
      last_pos = find_pos + from.length();
    }

    // Care for the rest after last occurrence.
    output_str.append(str.substr(last_pos));

    return output_str;
  }

  int_t string_size_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("size", 1);
    return vm.push(zb::unicode::length(str));
  }

  int_t string_ascii_size_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("ascii_size", 1);
    return vm.push(str.size());
  }

  int_t string_starts_with_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("starts_with", 2);

    const object& s = vm[1];
    if (!s.is_string()) {
      vm.set_error("Invalid second argument in string::starts_with().");
      return -1;
    }

    return vm.push(str.starts_with(s.get_string_unchecked()));
  }

  int_t string_ends_with_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("ends_with", 2);

    const object& s = vm[1];
    if (!s.is_string()) {
      vm.set_error("Invalid second argument in string::ends_with().");
      return -1;
    }

    return vm.push(str.ends_with(s.get_string_unchecked()));
  }

  int_t string_contains_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("contains", 2);

    const object& s = vm[1];
    if (!s.is_string()) {
      vm.set_error("Invalid second argument in string::contains().");
      return -1;
    }

    return vm.push(str.contains(s.get_string_unchecked()));
  }

  int_t string_to_upper_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("to_upper", 1);
    return vm.push(zs::_s(vm.get_engine(), to_upper_string(vm.get_engine(), str)));
  }

  int_t string_to_lower_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("to_lower", 1);
    return vm.push(zs::_s(vm.get_engine(), to_lower_string(vm.get_engine(), str)));
  }

  int_t string_words_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("words", 1);
    return vm.push(split_words_to_array_object(vm.get_engine(), str));
  }

  int_t string_capitalize_impl(zs::vm_ref vm) noexcept {
    zs::engine* eng = vm.get_engine();

    const int_t nargs = vm.stack_size();

    if (!zb::is_one_of(nargs, 1, 2)) {
      ZS_STRING_SET_ARG_ERROR("capitalize");
      return -1;
    }

    ZS_STRING_GET("capitalize");

    bool lower_rest = false;
    if (nargs == 2) {
      const object& lower_rest_obj = vm[1];

      if (!lower_rest_obj.is_bool()) {
        vm.set_error("Invalid second argument in string::capitalize().");
        return -1;
      }

      lower_rest = lower_rest_obj._int;
    }

    zs::string upper_str((zs::allocator<char>(eng)));
    zb::unicode::append_to(sv_from_char<char32_t>(zb::unicode::to_upper(get_first_letter(str))), upper_str);

    std::string_view rest_str = str.substr(zb::unicode::sequence_length(static_cast<uint8_t>(str[0])));
    return vm.push(zs::object::create_concat_string(
        eng, upper_str, lower_rest ? to_lower_string(eng, rest_str) : rest_str));
  }

  int_t string_decapitalize_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("decapitalize", 1);

    zs::engine* eng = vm.get_engine();
    zs::string lower_str((zs::allocator<char>(eng)));

    zb::unicode::append_to(sv_from_char<char32_t>(zb::unicode::to_lower(get_first_letter(str))), lower_str);

    return vm.push(zs::object::create_concat_string(
        eng, lower_str, str.substr(zb::unicode::sequence_length(static_cast<uint8_t>(str[0])))));
  }

  int_t string_to_camel_case_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("to_camel_case", 1);

    zs::engine* eng = vm.get_engine();
    zs::vector<std::string_view> words = split_words_to_vector(eng, str, { " \t\r\n_-" });
    zs::string output_str((zs::allocator<char>(eng)));

    const size_t words_sz = words.size();

    if (!words_sz) {
      return vm.push(zs::_ss(""));
    }

    {
      std::string_view word = words[0];
      char32_t first_letter = get_first_letter(word);
      char32_t lower_first_letter = zb::unicode::to_lower(first_letter);

      if (first_letter == lower_first_letter) {
        output_str.append(word);
      }
      else {
        zb::unicode::append_to(sv_from_char(lower_first_letter), output_str);
        output_str.append(word.substr(zb::unicode::sequence_length(static_cast<uint8_t>(word[0]))));
      }
    }

    for (size_t i = 1; i < words_sz; i++) {
      std::string_view word = words[i];
      char32_t first_letter = get_first_letter(word);
      char32_t upper_first_letter = zb::unicode::to_upper(first_letter);

      if (first_letter == upper_first_letter) {
        output_str.append(word);
      }
      else {
        zb::unicode::append_to(sv_from_char(upper_first_letter), output_str);
        output_str.append(word.substr(zb::unicode::sequence_length(static_cast<uint8_t>(word[0]))));
      }
    }

    return vm.push(zs::_s(eng, output_str));
  }

  int_t string_to_snake_case_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("to_snake_case", 1);

    zs::engine* eng = vm.get_engine();
    zs::vector<std::string_view> words = split_words_to_vector(eng, str);
    zs::string output_str((zs::allocator<char>(eng)));

    const size_t words_sz = words.size();

    if (words_sz == 1) {
      output_str.append(to_lower_string(eng, words[0]));
      return vm.push(zs::_s(eng, output_str));
    }

    for (size_t i = 0; i < words_sz - 1; i++) {
      std::string_view word = words[i];
      output_str.append(to_lower_string(eng, word));
      output_str.push_back('_');
    }

    output_str.append(to_lower_string(eng, words.back()));

    return vm.push(zs::_s(eng, output_str));
  }

  int_t string_to_title_case_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("to_title_case", 1);

    zs::engine* eng = vm.get_engine();
    zs::vector<std::string_view> words = split_words_to_vector(eng, str);
    zs::string output_str((zs::allocator<char>(eng)));

    const size_t words_sz = words.size();

    if (words_sz == 1) {
      std::string_view word = words[0];
      char32_t first_letter = get_first_letter(word);
      char32_t upper_first_letter = zb::unicode::to_upper(first_letter);

      if (first_letter == upper_first_letter) {
        output_str.append(word);
      }
      else {
        zb::unicode::append_to(sv_from_char(upper_first_letter), output_str);
        output_str.append(word.substr(zb::unicode::sequence_length(static_cast<uint8_t>(word[0]))));
      }

      return vm.push(zs::_s(eng, output_str));
    }

    for (size_t i = 0; i < words_sz - 1; i++) {
      std::string_view word = words[i];
      char32_t first_letter = get_first_letter(word);
      char32_t upper_first_letter = zb::unicode::to_upper(first_letter);

      if (first_letter == upper_first_letter) {
        output_str.append(word);
      }
      else {
        zb::unicode::append_to(sv_from_char(upper_first_letter), output_str);
        output_str.append(word.substr(zb::unicode::sequence_length(static_cast<uint8_t>(word[0]))));
      }

      output_str.push_back(' ');
    }

    {
      std::string_view word = words.back();
      char32_t first_letter = get_first_letter(word);
      char32_t upper_first_letter = zb::unicode::to_upper(first_letter);

      if (first_letter == upper_first_letter) {
        output_str.append(word);
      }
      else {
        zb::unicode::append_to(sv_from_char(upper_first_letter), output_str);
        output_str.append(word.substr(zb::unicode::sequence_length(static_cast<uint8_t>(word[0]))));
      }
    }

    return vm.push(zs::_s(eng, output_str));
  }

  int_t string_replace_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("replace", 3);
    zs::engine* eng = vm.get_engine();
    const object& from = vm[1];
    const object& to = vm[2];

    std::string_view from_str;
    zs::string from_s((zs::allocator<char>(eng)));

    std::string_view to_str;
    zs::string to_s((zs::allocator<char>(eng)));

    if (from.is_string()) {
      from_str = from.get_string_unchecked();
    }
    else if (from.is_integer()) {
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)from._int), from_s);
      from_str = from_s;
    }
    else {
      vm.set_error("Invalid first argument in string::replace().");
      return -1;
    }

    if (to.is_string()) {
      to_str = to.get_string_unchecked();
    }
    else if (to.is_integer()) {
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)to._int), to_s);
      to_str = to_s;
    }
    else {
      vm.set_error("Invalid second argument in string::replace().");
      return -1;
    }

    return vm.push(zs::_s(eng, replace_all(eng, str, from_str, to_str)));
  }

  int_t string_replace_first_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("replace_first", 3);
    zs::engine* eng = vm.get_engine();
    const object& from = vm[1];
    const object& to = vm[2];

    std::string_view from_str;
    zs::string from_s((zs::allocator<char>(eng)));

    std::string_view to_str;
    zs::string to_s((zs::allocator<char>(eng)));

    if (from.is_string()) {
      from_str = from.get_string_unchecked();
    }
    else if (from.is_integer()) {
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)from._int), from_s);
      from_str = from_s;
    }
    else {
      vm.set_error("Invalid first argument in string::replace_first().");
      return -1;
    }

    if (to.is_string()) {
      to_str = to.get_string_unchecked();
    }
    else if (to.is_integer()) {
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)to._int), to_s);
      to_str = to_s;
    }
    else {
      vm.set_error("Invalid second argument in string::replace_first().");
      return -1;
    }

    return vm.push(zs::_s(eng, replace_first(eng, str, from_str, to_str)));
  }

  int_t string_replace_last_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("replace_last", 3);

    zs::engine* eng = vm.get_engine();
    const object& from = vm[1];
    const object& to = vm[2];

    std::string_view from_str;
    zs::string from_s((zs::allocator<char>(eng)));

    std::string_view to_str;
    zs::string to_s((zs::allocator<char>(eng)));

    if (from.is_string()) {
      from_str = from.get_string_unchecked();
    }
    else if (from.is_integer()) {
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)from._int), from_s);
      from_str = from_s;
    }
    else {
      vm.set_error("Invalid first argument in string::replace_last().");
      return -1;
    }

    if (to.is_string()) {
      to_str = to.get_string_unchecked();
    }
    else if (to.is_integer()) {
      zb::unicode::append_to(sv_from_char<char32_t>((char32_t)to._int), to_s);
      to_str = to_s;
    }
    else {
      vm.set_error("Invalid second argument in string::replace_last().");
      return -1;
    }

    return vm.push(zs::_s(eng, replace_last(eng, str, from_str, to_str)));
  }

  int_t string_add_line_prefix_impl(zs::vm_ref vm) noexcept {
    ZS_STRING_BEGIN_IMPL("add_line_prefix", 2);

    zs::engine* eng = vm.get_engine();
    const object& from = vm[1];

    zs::string out_str((zs::string_allocator(eng)));
    std::string_view prefix_str = from.get_string_unchecked();

    std::string_view::size_type pos = 0;
    std::string_view::size_type prev = 0;
    while ((pos = str.find("\n", prev)) != std::string_view::npos) {
      std::string_view line = str.substr(prev, pos - prev);
      prev = pos + 1;

      out_str.append(prefix_str);
      out_str.append(line);
      out_str.push_back('\n');
    }

    if (std::string_view line = str.substr(prev); !line.empty()) {
      out_str.append(prefix_str);
      out_str.append(line);
      out_str.push_back('\n');
    }

    return vm.push_string(out_str);
  }

  static int_t string_delegate_get_impl(zs::vm_ref vm) noexcept {
    const object& key = vm[1];

    if (key.is_number()) {
      std::string_view s = vm[0].get_string_unchecked();
      const int_t sz = (int_t)s.size();

      int_t index = key.convert_to_integer_unchecked();

      if (index < 0) {
        index += sz;
      }

      if (index >= 0 && index < sz) {
        return vm.push((int_t)s[index]);
      }

      vm->set_error("Out of bounds\n");
      return -1;
    }

    return vm.push(zs::none{});
  }

} // namespace

// https://github.com/panzerdp/voca/blob/master/src/case/title_case.js
zs::object create_string_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();
  tbl.reserve(20);
  tbl["size"] = _nf(string_size_impl);
  tbl["ascii_size"] = _nf(string_ascii_size_impl);
  tbl["starts_with"] = _nf(string_starts_with_impl);
  tbl["ends_with"] = _nf(string_ends_with_impl);
  tbl["contains"] = _nf(string_contains_impl);
  tbl["to_upper"] = _nf(string_to_upper_impl);
  tbl["to_lower"] = _nf(string_to_lower_impl);
  tbl["words"] = _nf(string_words_impl);
  tbl["capitalize"] = _nf(string_capitalize_impl);
  tbl["decapitalize"] = _nf(string_decapitalize_impl);
  tbl["to_camel_case"] = _nf(string_to_camel_case_impl);
  tbl["to_snake_case"] = _nf(string_to_snake_case_impl);
  tbl["to_title_case"] = _nf(string_to_title_case_impl);
  tbl["replace"] = _nf(string_replace_impl);
  tbl["replace_first"] = _nf(string_replace_first_impl);
  tbl["replace_last"] = _nf(string_replace_last_impl);
  tbl["add_line_prefix"] = _nf(string_add_line_prefix_impl);
  tbl[zs::_sv(constants::k_mt_get_string)] = _nf(string_delegate_get_impl);

  tbl.set_no_default_none();
  return obj;
}
} // namespace zs.
