#include <zscript/core/zcore.h>
namespace zs {
user_data_object* user_data_object::create(zs::engine* eng, size_t size) {
  user_data_object* uobj = (user_data_object*)eng->allocate(
      sizeof(user_data_object) + size, (alloc_info_t)memory_tag::nt_user_data);
  uobj = zb_placement_new(uobj) user_data_object(eng);

  return uobj;
}

user_data_object::~user_data_object() {
  if (_release_hook) {
    _release_hook(_engine, data());
  }
}

zs::error_result user_data_object::copy_to_type(void* obj, size_t data_size, std::string_view tid) {
  if (_copy_fct) {
    return (*_copy_fct)(obj, data_size, tid, (void*)data());
  }
  return zs::error_code::invalid_type;
}

zs::error_result user_data_object::convert_to_string(std::ostream& stream) {

  if (_to_string_cb) {
    zs::object_base obj;
    ::memset(&obj, 0, sizeof(zs::object_base));
    obj._type = object_type::k_user_data;
    obj._udata = this;
    return _to_string_cb(obj, stream);
  }

  stream << "0x" << std::setfill('0') << std::setw(sizeof(int_t) * 2) << std::hex << (int_t)this;
  return {};
}
} // namespace zs.
