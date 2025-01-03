#include <zscript/zscript.h>
#include <zscript/std/zmutable_string.h>
#include "zvirtual_machine.h"
#include "utility/zparameter_stream.h"
#include <zscript/base/strings/charconv.h>
#include <zscript/base/strings/unicode.h>
#include <zscript/base/strings/stack_string.h>

namespace zs {
namespace {
  using namespace literals;
  zs::object create_mutable_string_delegate(zs::engine* eng);

  namespace mustr {
    inline constexpr object uid = _sv("__mutable_string_object__");
    inline constexpr object type_id = _sv("mutable_string");
    inline constexpr object reg_id = _sv("__mutable_string_delegate__");

    inline zs::error_result validate_index(int_t& index, size_t u32_length) {
      if (index < 0) {
        index += u32_length;
      }

      if (index < 0 or index >= (int_t)u32_length) {
        return errc::out_of_bounds;
      }

      return {};
    }

    inline int_t u8_to_u32(const char* it) { return zb::unicode::next_u8_to_u32_s(it); }
    inline object u8_to_char_obj(const char* it) { return object::create_char(u8_to_u32(it)); }

    inline zb::stack_string<4> u32_to_u8_char_str(int_t input_char) {
      zb::stack_string<4> buffer(zb::unicode::code_point_size_u8((char32_t)input_char), 0);
      zb::unicode::append_u32_to_u8((char32_t)input_char, buffer.data());
      return buffer;
    }

    inline zs::error_result get_u8_index(
        size_t& u8_index, std::string_view str, size_t u32_index, size_t u32_length) {
      u8_index = zb::unicode::u32_index_to_u8_index(str, u32_index, u32_length);
      if (u8_index == zs::mutable_string::npos) {
        return errc::out_of_bounds;
      }

      return {};
    }
  } // namespace mustr

  struct mutable_string_iterator_ref {

    inline mutable_string_iterator_ref(object& obj) noexcept
        : index(obj._ex1_u32)
        , pointer(obj._pointer) {}

    uint32_t& index;
    void*& pointer;

    ZS_CK_INLINE int_t idx() const noexcept { return (int_t)index; }
    ZS_CK_INLINE char*& ptr() const noexcept { return reinterpret_cast<char*&>(pointer); }
    ZS_CK_INLINE bool operator==(const mutable_string_iterator_ref& it) const noexcept {
      return ptr() == it.ptr();
    }
    ZS_CK_INLINE bool operator!=(const mutable_string_iterator_ref& it) const noexcept {
      return ptr() != it.ptr();
    }
  };

  object create_mutable_string_iterator(zs::vm_ref vm, int_t index, const char* ptr);

  inline object create_mutable_string_iterator(zs::vm_ref vm, mutable_string_iterator_ref it_ref) {
    return create_mutable_string_iterator(vm, it_ref.idx(), it_ref.ptr());
  }

  int_t mutable_string_iterator_add_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 3) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in mutable_string._add.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in mutable_string._add.\n");
      return -1;
    }

    int_t rhs;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(rhs), -1);

    if (rhs == 1) {
      object it = create_mutable_string_iterator(vm, mutable_string_iterator_ref(it_atom));
      mutable_string_iterator_ref it_ref(it);

      it_ref.index++;
      it_ref.ptr() += zb::unicode::sequence_length(*it_ref.ptr());

      return vm.push(it);
    }
    else if (rhs == -1) {
      object it = create_mutable_string_iterator(vm, it_atom);
      mutable_string_iterator_ref it_ref(it);

      it_ref.index--;

      char*& ptr = it_ref.ptr();
      while (zb::unicode::is_trail(*(--ptr))) {
      }

      return vm.push(it);
    }

    vm->ZS_VM_ERROR(errc::invalid_argument, "Invalid iterator offset in mutable_string iterator.\n");
    return -1;
  }

  int_t mutable_string_iterator_pre_incr_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.++.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in mutable_string.++.\n");
      return -1;
    }

    object it = create_mutable_string_iterator(vm, mutable_string_iterator_ref(it_atom));
    mutable_string_iterator_ref it_ref(it);

    ++it_ref.index;
    it_ref.ptr() += zb::unicode::sequence_length(*it_ref.ptr());
    return vm.push(it);
  }

  int_t mutable_string_iterator_pre_decr_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in mutable_string._add.\n");
      return -1;
    }

    object it_atom = *ps++;
    if (!it_atom.is_atom()) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid iterator type in mutable_string._add.\n");
      return -1;
    }

    mutable_string_iterator_ref it_ref(it_atom);
    --it_ref.index;

    char*& ptr = it_ref.ptr();
    while (zb::unicode::is_trail(*(--ptr))) {
    }

    return vm.push(it_atom);
  }

  zs::object create_mutable_string_iterator_delegate(zs::engine* eng) {
    object obj = object::create_table(eng);
    table_object& tbl = obj.as_table();
    tbl.reserve(10);

    tbl.emplace(constants::get<meta_method::mt_typeof>(), _s(eng, "mutable_string_iterator"));
    tbl.emplace(constants::get<meta_method::mt_add>(), mutable_string_iterator_add_impl);
    tbl.emplace(constants::get<meta_method::mt_pre_incr>(), mutable_string_iterator_pre_incr_impl);
    tbl.emplace(constants::get<meta_method::mt_pre_decr>(), mutable_string_iterator_pre_decr_impl);

    tbl.emplace("next"_ss, [](vm_ref vm) -> int_t {
      mutable_string_iterator_ref it_ref(vm[0]);
      return vm.push(create_mutable_string_iterator(
          vm, it_ref.index + 1, it_ref.ptr() + zb::unicode::sequence_length(*it_ref.ptr())));
    });

    tbl.emplace("is_same", [](vm_ref vm) -> int_t {
      return vm.push(mutable_string_iterator_ref(vm[0]) == mutable_string_iterator_ref(vm[1]));
    });

    tbl.emplace("get", [](vm_ref vm) -> int_t {
      return vm.push(mustr::u8_to_char_obj(mutable_string_iterator_ref(vm[0]).ptr()));
    });

    tbl.emplace("get_if_not", [](vm_ref vm) -> int_t {
      mutable_string_iterator_ref it_ref(vm[0]);
      return it_ref != mutable_string_iterator_ref(vm[1]) ? vm.push(mustr::u8_to_char_obj(it_ref.ptr()))
                                                          : vm.push_null();
    });

    tbl.emplace("safe_get", [](vm_ref vm) -> int_t {
      mutable_string_iterator_ref it_ref(vm[0]);
      return mutable_string::as_mutable_string(vm[1]).is_ptr_in_range(it_ref.ptr())
          ? vm.push(mustr::u8_to_char_obj(it_ref.ptr()))
          : vm.push_null();
    });

    tbl.emplace(
        "get_key", [](vm_ref vm) -> int_t { return vm.push(mutable_string_iterator_ref(vm[0]).idx()); });

    tbl.emplace("safe_key", [](vm_ref vm) -> int_t {
      mutable_string_iterator_ref it_ref(vm[0]);
      return mutable_string::as_mutable_string(vm[1]).is_ptr_in_range(it_ref.ptr()) ? vm.push(it_ref.idx())
                                                                                    : vm.push_null();
    });

    tbl.emplace("get_key_if_not", [](vm_ref vm) -> int_t {
      mutable_string_iterator_ref it_ref(vm[0]);
      return it_ref != mutable_string_iterator_ref(vm[1]) ? vm.push(it_ref.idx()) : vm.push_null();
    });

    return obj;
  }

  object create_mutable_string_iterator(zs::vm_ref vm, int_t index, const char* ptr) {
    if (object& obj = vm->get_delegated_atom_delegates_table()
                          .as_table()[(int_t)constants::k_atom_mutable_string_iterator_delegate_id];
        !obj.is_table()) {
      obj = create_mutable_string_iterator_delegate(vm.get_engine());
    }

    object it;
    it._type = object_type::k_atom;
    it._atom_type = atom_type::atom_mutable_string_iterator;
    it._pointer = (void*)ptr;
    it._ex1_u32 = (uint32_t)index;
    it._ex2_delegate_id = constants::k_atom_mutable_string_iterator_delegate_id;
    return it;
  }

  int_t mutable_string_begin_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);
    return vm.push(create_mutable_string_iterator(vm, 0, mstr->data()));
  }

  int_t mutable_string_end_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);
    return vm.push(create_mutable_string_iterator(vm, mstr->size(), mstr->end_ptr()));
  }

  int_t mutable_string_size_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);
    return vm.push(zb::unicode::length(*mstr));
  }

  int_t mutable_string_is_empty_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);
    return vm.push_bool(mstr->empty());
  }

  int_t mutable_string_ascii_size_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);
    return vm.push((int_t)mstr->size());
  }

  int_t mutable_string_is_ascii_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    const size_t sz = mstr->size();
    for (size_t i = 0; i < sz; i++) {
      if (zb::unicode::sequence_length((*mstr)[i]) != 1) {
        return vm.push_bool(false);
      }
    }

    return vm.push_bool(true);
  }

  int_t mutable_string_to_string_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);
    return vm.push(zs::_s(vm, *mstr));
  }

  // vm[0] should be the mutable string.
  // vm[1] should be the key.
  // vm[2] should be the value.
  // vm[3] should be the delegate.
  int_t mutable_string_meta_set_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    if (ps.size() != 4) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.__set.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    int_t u32_input_index = 0;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(u32_input_index), -1);

    // Length of the mutable string in u32.
    const size_t u32_length = zb::unicode::length(*mstr);

    ZS_RETURN_IF_ERROR(mustr::validate_index(u32_input_index, u32_length), vm.set_error("Out of bounds\n"));

    // Find the u8 index from the u32 input index.
    size_t u8_index = 0;
    ZS_RETURN_IF_ERROR(mustr::get_u8_index(u8_index, *mstr, u32_input_index, u32_length),
        vm.set_error("Out of bounds index in `mutable_string[]`."));

    int_t input_char = 0;
    ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(input_char), -1);

    // This is just a buffer to convert the char32_t to a utf8 string.
    zb::stack_string<4> input_char_str = mustr::u32_to_u8_char_str(input_char);

    // Replace the dest char at index with the input char string.
    mstr->replace(mstr->begin() + u8_index,
        mstr->begin() + u8_index + zb::unicode::sequence_length((*mstr)[u8_index]), input_char_str);
    return vm.push(vm[0]);
  }

  // vm[0] should be the mutable string (lhs).
  // vm[1] should be the rhs.
  // vm[2] should be the delegate.
  int_t mutable_string_meta_add_impl(zs::vm_ref vm) {

    zs::parameter_stream ps(vm);

    if (ps.size() != 3) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count, "Invalid number of parameter in mutable_string._add.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);

    zs::string new_str(*mstr, zs::string_allocator(vm.get_engine()));
    new_str += rhs_str;
    return vm.push(mutable_string::create(vm, std::move(new_str)));
  }

  int_t mutable_string_meta_rhs_add_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 3) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.__rhs_add.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);

    return vm.push(object::create_concat_string(vm.get_engine(), rhs_str, *mstr));
  }

  // vm[0] should be the mutable string (lhs).
  // vm[1] should be the rhs.
  // vm[2] should be the delegate.
  int_t mutable_string_meta_add_eq_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);

    if (ps.size() != 3) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.__add_eq.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);

    *mstr += rhs_str;
    return vm.push(vm[0]);
  }

  int_t mutable_string_starts_with_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.starts_with.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);
    return vm.push(mstr->starts_with(rhs_str));
  }

  int_t mutable_string_ends_with_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.ends_with.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);
    return vm.push(mstr->ends_with(rhs_str));
  }

  int_t mutable_string_contains_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    if (ps.size() != 2) {
      vm->ZS_VM_ERROR(
          errc::invalid_parameter_count, "Invalid number of parameter in mutable_string.contains.\n");
      return -1;
    }

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);
    return vm.push(mstr->contains(rhs_str));
  }

  int_t mutable_string_append_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    while (ps.size()) {
      if (std::string_view rhs_str; !ps.optional<string_parameter>(rhs_str)) {
        mstr->append(rhs_str);
      }

      else if (int_t input_char; !ps.optional<integer_parameter>(input_char)) {
        zb::stack_string<4> input_char_str = mustr::u32_to_u8_char_str(input_char);
        mstr->append(input_char_str);
      }
      else {

        vm->ZS_VM_ERROR(errc::invalid_parameter_type,
            "Invalid parameter type in mutable_string.append. Got '",
            zs::get_object_type_name(ps->get_type()), "', expected 'string' or 'char'.\n");
        return -1;
      }
    }
    return vm.push(vm[0]);
  }

  int_t mutable_string_compare_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    std::string_view rhs_str;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(rhs_str), -1);
    return vm.push(mstr->compare(rhs_str));
  }

  int_t mutable_string_copy_impl(zs::vm_ref vm) noexcept {
    zs::parameter_stream ps(vm);

    const object& mobj = *ps;
    mutable_string* mstr = nullptr;
    ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

    object obj = mstr->clone();

    obj.as_udata().set_delegate(mobj.as_udata().get_delegate(), mobj.as_udata().get_delegate_flags());
    return vm.push(obj);
  }

  zs::object create_mutable_string_delegate(zs::engine* eng) {
    table_object* tbl = table_object::create(eng);
    tbl->reserve(20);

    tbl->emplace(constants::get<meta_method::mt_typeof>(), "mutable_string"_ss);
    tbl->emplace(constants::get<meta_method::mt_get>(), mutable_string_meta_get_impl);
    tbl->emplace(constants::get<meta_method::mt_set>(), mutable_string_meta_set_impl);
    tbl->emplace(constants::get<meta_method::mt_tostring>(), mutable_string_to_string_impl);
    tbl->emplace(constants::get<meta_method::mt_add>(), mutable_string_meta_add_impl);
    tbl->emplace(constants::get<meta_method::mt_add_eq>(), mutable_string_meta_add_eq_impl);
    tbl->emplace(constants::get<meta_method::mt_rhs_add>(), mutable_string_meta_rhs_add_impl);
    tbl->emplace(constants::get<meta_method::mt_copy>(), mutable_string_copy_impl);
    tbl->emplace(constants::get<meta_method::mt_compare>(), mutable_string_compare_impl);

    tbl->emplace("to_string"_ss, mutable_string_to_string_impl);
    tbl->emplace("size"_ss, mutable_string_size_impl);
    tbl->emplace(_ss("is_empty"), mutable_string_is_empty_impl);
    tbl->emplace("ascii_size"_ss, mutable_string_ascii_size_impl);
    tbl->emplace("is_ascii"_ss, mutable_string_is_ascii_impl);
    tbl->emplace("starts_with"_ss, mutable_string_starts_with_impl);
    tbl->emplace("ends_with"_ss, mutable_string_ends_with_impl);
    tbl->emplace("append"_ss, mutable_string_append_impl);
    tbl->emplace("contains"_ss, mutable_string_contains_impl);
    tbl->emplace("begin"_ss, mutable_string_begin_impl);
    tbl->emplace("end"_ss, mutable_string_end_impl);

    tbl->set_no_default_none();

    return object(tbl, false);
  }

} // namespace

const zs::object& get_mutable_string_delegate(zs::engine* eng) {
  object& obj = eng->get_registry_table_object()[mustr::reg_id];
  return obj.is_table() ? obj : (obj = create_mutable_string_delegate(eng));
}

zs::error_result mutable_string_parameter::parse(
    zs::parameter_stream& s, bool output_error, mutable_string*& value) {

  if (s.is_user_data_with_uid(mustr::uid)) {
    value = s++->as_udata().data<mutable_string>();
    return {};
  }

  s.set_opt_error(output_error, "Invalid mutable string type.");
  return zs::errc::invalid_parameter_type;
}

zs::error_result mutable_string_parameter::parse(
    zs::parameter_stream& s, bool output_error, const mutable_string*& value) {

  if (s.is_user_data_with_uid(mustr::uid)) {
    value = s++->as_udata().data<mutable_string>();
    return {};
  }

  s.set_opt_error(output_error, "Invalid mutable string type.");
  return zs::errc::invalid_parameter_type;
}

inline constexpr user_data_content k_mutable_string_udata_content
    = { [](zs::engine* eng, zs::raw_pointer_t ptr) { ((mutable_string*)ptr)->~mutable_string(); },
        [](const zs::object_base& obj, std::ostream& stream) -> error_result {
          stream << obj.as_udata().data_ref<mutable_string>();
          return {};
        },
        mustr::uid, mustr::type_id };

template <class... Args>
inline object create_mutable_string_impl(zs::engine* eng, Args&&... args) {
  user_data_object* uobj
      = user_data_object::create(eng, sizeof(mutable_string), &k_mutable_string_udata_content);
  uobj->construct<mutable_string>(std::forward<Args>(args)..., eng);
  uobj->set_delegate(get_mutable_string_delegate(eng), delegate_flags_t::df_none);
  return zs::object(uobj, false);
}

object mutable_string::clone() const noexcept {

  object obj = create_mutable_string_impl(this->get_allocator().get_engine(), *this);
  return obj;
}

mutable_string& mutable_string::as_mutable_string(const object& obj) {
  return obj.as_udata().data_ref<mutable_string>();
}

bool mutable_string::is_mutable_string(const object_base& obj) noexcept {
  return obj.is_user_data(&k_mutable_string_udata_content);
}

object mutable_string::create(zs::vm_ref vm) noexcept { return create_mutable_string_impl(vm.get_engine()); }

object mutable_string::create(zs::vm_ref vm, size_t n) noexcept {
  return create_mutable_string_impl(vm.get_engine(), n, 0);
}

object mutable_string::create(zs::vm_ref vm, std::string_view s) noexcept {
  return create_mutable_string_impl(vm.get_engine(), s);
}

object mutable_string::create(zs::vm_ref vm, zs::string&& s) noexcept {
  return create_mutable_string_impl(vm.get_engine(), std::move(s));
}

int_t vm_create_mutable_string(zs::vm_ref vm) {
  zs::parameter_stream ps(vm);
  ++ps;

  if (ps.size() > 1) {
    return vm.set_error("Out hjkhjkh bounds\n");
  }

  if (std::string_view svalue; !ps.optional<string_parameter>(svalue)) {
    return vm.push(mutable_string::create(vm, svalue));
  }

  if (int_t sz = 0; !ps.optional<integer_parameter>(sz)) {
    return vm.push(mutable_string::create(vm, sz));
  }

  if (ps.is_valid()) {
    return vm.set_error("Out hjkhjkhmm,m,m,m, bounds\n");
  }

  return vm.push(mutable_string::create(vm, ""));
}

// vm[0] should be the mutable string.
// vm[1] should be the key.
// vm[2] should be the delegate.
int_t mutable_string_meta_get_impl(zs::vm_ref vm) noexcept {
  zs::parameter_stream ps(vm);

  mutable_string* mstr = nullptr;
  ZS_RETURN_IF_ERROR(ps.require<mutable_string_parameter>(mstr), -1);

  int_t u32_input_index = 0;
  ZS_RETURN_IF_ERROR(ps.require<integer_parameter>(u32_input_index), -1);

  // Length of the mutable string in u32.
  const size_t u32_length = zb::unicode::length(*mstr);

  ZS_RETURN_IF_ERROR(mustr::validate_index(u32_input_index, u32_length), vm.set_error("Out of bounds."));

  // Find the u8 index from the u32 input index.
  size_t u8_index = 0;
  ZS_RETURN_IF_ERROR(mustr::get_u8_index(u8_index, *mstr, u32_input_index, u32_length),
      vm.set_error("Out of bounds index in `mutable_string[]`."));

  return vm.push(mustr::u8_to_char_obj(mstr->data() + u8_index));
}

} // namespace zs.
