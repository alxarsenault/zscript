#include <zscript/zscript.h>
#include <zscript/std/zio.h>
#include "zvirtual_machine.h"
#include "utility/zvm_module.h"
#include <zscript/base/strings/charconv.h>
#include <zscript/base/strings/unicode.h>
#include "utility/zparameter_stream.h"

namespace zs {
namespace {
  inline constexpr object k_stream_uid = _sv("__stream_object__");
  inline constexpr object k_stream_type_id = _sv("stream");
  inline constexpr object k_stream_reg_id = _sv("__stream_delegate__");

  int_t zio_print_internal(
      zs::vm_ref vm, std::ostream& stream, std::string_view sep = "", std::string_view endl = "") {
    int_t nargs = vm.stack_size();

    if (nargs == 1) {
      if (!endl.empty()) {
        stream << endl;
      }
      return 0;
    }

    const object* objs = vm.stack_base_pointer() + 1;

    zs::string separator(sep, zs::string_allocator(vm.get_engine()));
    zs::string end_line(endl, zs::string_allocator(vm.get_engine()));

    for (int_t i = 1; i < nargs - 1; i++) {
      (objs++)->stream(stream);
      stream << separator;
    }

    (objs++)->stream(stream);

    if (!end_line.empty()) {
      stream << end_line;
    }

    return 0;
  }

  int_t zio_print_impl(zs::vm_ref vm) {
    return zio_print_internal(vm, vm.get_engine()->get_stream(), " ", "\n");
  }

  int_t zio_write_impl(zs::vm_ref vm) { return zio_print_internal(vm, vm.get_engine()->get_stream()); }

  int_t zio_meta_call_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    zs::print("DDDSDDAADAD", ps.size());

    //    const object& obj = *ps;
    stream_object* sobj = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<stream_parameter>(sobj), -1);

    return vm.push(32);
  }

  int_t zio_meta_lshift_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    //    zs::print("zio_meta_lshift_impl", ps.size());

    const object& obj = *ps;
    stream_object* sobj = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<stream_parameter>(sobj), -1);

    if (sobj->_ostream) {
      if (ps.size()) {
        (*ps).stream(*sobj->_ostream);
      }
    }

    return vm.push(obj);
  }

  int_t zio_stream_to_string_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    //    zs::print("zio_meta_lshift_impl", ps.size());

    //    const object& obj = *ps;
    stream_object* sobj = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<stream_parameter>(sobj), -1);

    return vm.push_string(sobj->_sstream.view());
  }

  zs::object create_stream_delegate(zs::engine* eng) {
    table_object* tbl = table_object::create(eng);
    tbl->reserve(20);

    tbl->emplace(constants::get<meta_method::mt_typeof>(), zs::_ss("stream"));

    tbl->emplace(constants::get<meta_method::mt_call>(), zio_meta_call_impl);
    tbl->emplace(constants::get<meta_method::mt_lshift>(), zio_meta_lshift_impl);
    tbl->emplace(constants::get<meta_method::mt_tostring>(), zio_stream_to_string_impl);
    tbl->emplace(zs::_ss("to_string"), zio_stream_to_string_impl);

    tbl->set_no_default_none();

    return object(tbl, false);
  }
} // namespace.

stream_object::stream_object(zs::engine* eng)
    : _ostream(nullptr)
    , _istream(nullptr)
    //, _sstream(zs::create_string_stream(eng)){
    , _sstream(detail::create_string_stream<string_stream_type>(eng)) {}

const zs::object& get_stream_delegate(zs::engine* eng) {
  object& obj = eng->get_registry_table_object()[k_stream_reg_id];
  return obj.is_table() ? obj : (obj = create_stream_delegate(eng));
}

zs::error_result stream_parameter::parse(zs::parameter_stream& s, bool output_error, stream_object*& value) {

  if (s.is_user_data_with_uid(k_stream_uid)) {
    value = s++->as_udata().data<stream_object>();
    return {};
  }

  s.set_opt_error(output_error, "Invalid stream type.");
  return zs::errc::invalid_parameter_type;
}

zs::error_result stream_parameter::parse(
    zs::parameter_stream& s, bool output_error, const stream_object*& value) {

  if (s.is_user_data_with_uid(k_stream_uid)) {
    value = s++->as_udata().data<stream_object>();
    return {};
  }

  s.set_opt_error(output_error, "Invalid stream type.");
  return zs::errc::invalid_parameter_type;
}

inline constexpr user_data_content k_stream_udata_content
    = { [](zs::engine* eng, zs::raw_pointer_t ptr) { ((stream_object*)ptr)->~stream_object(); },
        [](const zs::object_base& obj, std::ostream& stream) -> error_result {
          //          stream << obj.as_udata().data_ref<stream_object>();
          return {};
        },
        k_stream_uid, k_stream_type_id };

template <class... Args>
inline object create_stream_impl(zs::engine* eng, Args&&... args) {
  user_data_object* uobj = user_data_object::create(eng, sizeof(stream_object), &k_stream_udata_content);
  uobj->construct<stream_object>(eng, std::forward<Args>(args)...);
  uobj->set_delegate(get_stream_delegate(eng), delegate_flags_t::df_none);
  return zs::object(uobj, false);
}

stream_object& stream_object::as_stream(const object& obj) {
  return obj.as_udata().data_ref<stream_object>();
}

bool stream_object::is_stream(const object_base& obj) noexcept {
  return obj.is_user_data(&k_stream_udata_content);
}

object stream_object::create(zs::vm_ref vm) noexcept {

  object obj = create_stream_impl(vm.get_engine());

  stream_object& sobj = as_stream(obj);

  sobj._istream = &sobj._sstream;
  sobj._ostream = &sobj._sstream;

  return obj;
}

object stream_object::create(zs::vm_ref vm, std::istream* istream, std::ostream* ostream) noexcept {

  object obj = create_stream_impl(vm.get_engine());

  stream_object& sobj = as_stream(obj);

  sobj._istream = istream;
  sobj._ostream = ostream;

  return obj;
}

int_t vm_create_stream(zs::vm_ref vm) {
  zs::parameter_stream ps(vm);
  ++ps;

  if (ps.size() > 1) {
    return vm.set_error("Out hjkhjkh bounds\n");
  }

  return vm.push(stream_object::create(vm));
}

zs::object create_io_lib(zs::vm_ref vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object io_module = zs::_t(eng);
  zs::table_object& io_tbl = io_module.as_table();
  io_tbl.reserve(16);

  io_tbl.emplace("print"_ss, zio_print_impl);
  io_tbl.emplace("write"_ss, zio_write_impl);

  io_tbl.emplace("stream"_ss, zs::vm_create_stream);

  return io_module;
}
} // namespace zs.
