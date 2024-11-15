
namespace zs {

ZS_DECL_RT_ACTION(string_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  if (key->is_number()) {
    std::string_view s = obj->get_string_unchecked();
    const int_t sz = (int_t)s.size();

    int_t index = key->convert_to_integer_unchecked();

    if (index < 0) {
      index += sz;
    }

    if (index >= 0 && index < sz) {
      dest.get() = (int_t)s[index];
      return {};
    }

    return zs::error_code::out_of_bounds;
  }

  zs::object delegate_obj = _default_string_delegate;
  return runtime_action<runtime_code::table_get>(CREF(delegate_obj), key, dest);
}

ZS_DECL_RT_ACTION(string_set, objref_t obj, cobjref_t key, cobjref_t value) {

  ZS_ASSERT(obj->is_mutable_string());

  if (!key->is_number()) {
    set_error("Invalid index type in `mutable_string[]`.\n");
    return zs::error_code::invalid_type;
  }

  // Create the input char string.
  std::string_view input_char_sv;

  // This is just a buffer to convert the char32_t to a utf8 string.
  // The result value will be assign in input_char_sv.
  zs::string input_char_tmp_str((zs::string_allocator(_engine)));

  if (value->is_integer()) {
    const char32_t input_char_int = (char32_t)value->_int;
    zb::unicode::append_to(std::basic_string_view<char32_t>(&input_char_int, 1), input_char_tmp_str);
    input_char_sv = input_char_tmp_str;
  }
  else if (value->is_string()) {
    input_char_sv = value->get_string_unchecked();
  }
  else {
    set_error("Invalid char type in `mutable_string[] = value`.\n");
    return zs::error_code::invalid_type;
  }

  zs::mutable_string_object& mstr = obj->as_mutable_string();

  // Length of the mutable string in u32.
  const size_t u32_length = zb::unicode::length(mstr);

  // Length of the mutable string in u8.
  const size_t u8_length = mstr.size();

  int_t u32_input_index = key->convert_to_integer_unchecked();

  // Wrap the input index.
  if (u32_input_index < 0) {
    u32_input_index += (int_t)u32_length;
  }

  // Input index bounds checking.
  if (u32_input_index < 0 or u32_input_index >= (int_t)u32_length) {
    set_error("Out of bounds index in `mutable_string[]`.\n");
    return zs::error_code::out_of_bounds;
  }

  // Find the u8 index from the u32 input index.
  size_t u8_index = zs::string::npos;
  for (size_t i = 0, u32_index = 0; i < u8_length;
       i += zb::unicode::sequence_length(static_cast<uint8_t>(mstr[i]))) {
    if (u32_index++ == (size_t)u32_input_index) {
      u8_index = i;
      break;
    }
  }

  if (u8_index == zs::string::npos) {
    set_error("Out of bounds index in `mutable_string[]`.\n");
    return zs::error_code::out_of_bounds;
  }

  const size_t dst_char_len = zb::unicode::sequence_length(static_cast<uint8_t>(mstr[u8_index]));

  // Replace the dest char at index with the input char string.
  mstr.replace(mstr.begin() + u8_index, mstr.begin() + u8_index + dst_char_len, input_char_sv);
  return {};
}

} // namespace zs.
