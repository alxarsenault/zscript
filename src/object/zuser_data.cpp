#include <zscript/zscript.h>

namespace zs {
user_data_object* user_data_object::create(zs::engine* eng, size_t size) {
  user_data_object* uobj = (user_data_object*)eng->allocate(
      sizeof(user_data_object) + sizeof(user_data_content) + size, (alloc_info_t)memory_tag::nt_user_data);
  zb_placement_new(uobj) user_data_object(eng);

  uobj->_content = (user_data_content*)uobj->_data;
  zb_placement_new(uobj->_content) user_data_content{};

  return uobj;
}

user_data_object* user_data_object::create(zs::engine* eng, size_t size, const user_data_content* content) {
  user_data_object* uobj = (user_data_object*)eng->allocate(
      sizeof(user_data_object) + size, (alloc_info_t)memory_tag::nt_user_data);
  uobj = zb_placement_new(uobj) user_data_object(eng);
  uobj->_content = (user_data_content*)content;
  return uobj;
}

user_data_object::~user_data_object() {
  const bool owns = owns_content();
  user_data_content content = *_content;
  uint8_t* d = data();

  if (owns) {
    user_data_content* ct = _content;
    ct->~user_data_content();
  }

  if (content.release_hook) {
    content.release_hook(_engine, d);
  }
}

object user_data_object::clone() const noexcept { return object((user_data_object*)this, true); }

zs::error_result user_data_object::copy_to_type(void* obj, size_t data_size, std::string_view tid) {
  if (_content->copy_fct) {
    return (*_content->copy_fct)(obj, data_size, tid, (void*)data());
  }
  return zs::error_code::invalid_type;
}

zs::error_result user_data_object::convert_to_string(std::ostream& stream) {

  if (_content->to_string_cb) {
    zs::object_base obj;
    ::memset(&obj, 0, sizeof(zs::object_base));
    obj._type = object_type::k_user_data;
    obj._udata = this;
    return _content->to_string_cb(obj, stream);
  }

  auto flgs = stream.flags();
  stream << "0x" << std::setfill('0') << std::setw(sizeof(int_t) * 2) << std::hex << (int_t)this;
  stream.flags(flgs);
  return {};
}
} // namespace zs.
