#include "object/zfunction_prototype.h"
#include "zbase/bitsery/bitsery.h"
#include "zbase/bitsery/adapter/buffer.h"
#include "zbase/bitsery/traits/vector.h"
#include "zbase/bitsery/traits/string.h"
#include "utility/json/zjson_parser.h"
#include "zvirtual_machine.h"

namespace bitsery::traits {
template <typename T>
struct ContainerTraits<zs::vector<T>> : public StdContainer<zs::vector<T>, true, true> {};

template <typename T, size_t N>
struct ContainerTraits<zs::small_vector<T, N>> : public StdContainer<zs::small_vector<T, N>, true, true> {};

template <>
struct ContainerTraits<zb::byte_vector> : public StdContainer<zb::byte_vector, true, true> {};

template <>
struct ContainerTraits<zb::byte_vector_t<zs::allocator<uint8_t>>>
    : public StdContainer<zb::byte_vector_t<zs::allocator<uint8_t>>, true, true> {};

template <>
struct ContainerTraits<zb::byte_view> : public StdContainer<zb::byte_view, false, true> {};

template <>
struct BufferAdapterTraits<zb::byte_view> : public StdContainerForBufferAdapter<zb::byte_view, false> {};

template <>
struct BufferAdapterTraits<zb::byte_vector> : public StdContainerForBufferAdapter<zb::byte_vector, true> {};

template <>
struct BufferAdapterTraits<zb::byte_vector_t<zs::allocator<uint8_t>>>
    : public StdContainerForBufferAdapter<zb::byte_vector_t<zs::allocator<uint8_t>>, true> {};

template <class T, size_t N>
struct ContainerTraits<std::array<T, N>> : public StdContainer<std::array<T, N>, false, true> {};

template <class T, size_t N>
struct BufferAdapterTraits<std::array<T, N>> : public StdContainerForBufferAdapter<std::array<T, N>, false> {
};

} // namespace bitsery::traits.

template <class Config>
class write_function_buffer_adapter {
public:
  using BitPackingEnabled
      = bitsery::details::OutputAdapterBitPackingWrapper<write_function_buffer_adapter<Config>>;
  using TConfig = Config;
  using Buffer = std::array<char, 256>;

  using BufferIt = typename bitsery::traits::BufferAdapterTraits<Buffer>::TIterator;

  static_assert(bitsery::details::IsDefined<BufferIt>::value,
      "Please define BufferAdapterTraits or include from <bitsery/traits/...> to "
      "use as buffer for BasicBufferedOutputStreamAdapter");

  static_assert(bitsery::traits::ContainerTraits<Buffer>::isContiguous,
      "BasicBufferedOutputStreamAdapter only works with contiguous containers");

  using TValue = char;

  write_function_buffer_adapter() = delete;
  write_function_buffer_adapter(const write_function_buffer_adapter&) = delete;
  write_function_buffer_adapter& operator=(const write_function_buffer_adapter&) = delete;

  write_function_buffer_adapter(zs::write_function_t wfunc, void* udata)
      : _write_func(wfunc)
      , _udata(udata)
      , _buf{}
      , _beginIt(std::begin(_buf))
      , _currOffset(0) {}

  write_function_buffer_adapter(write_function_buffer_adapter&& rhs)
      : _write_func(rhs._write_func)
      , _udata(rhs._udata)
      , _buf{ std::move(rhs._buf) }
      , _beginIt{ std::begin(_buf) }
      , _currOffset{ rhs._currOffset } {}

  write_function_buffer_adapter& operator=(write_function_buffer_adapter&& rhs) {
    _write_func = rhs._write_func;
    _udata = rhs._udata;
    _buf = std::move(rhs._buf);
    _beginIt = std::begin(_buf);
    _currOffset = rhs._currOffset;
    return *this;
  }

  void align() {}

  void currentWritePos(size_t) {
    //    static_assert(std::is_void<TChar>::value,
    //                  "setting write position is not supported with StreamAdapter");
  }

  size_t currentWritePos() const {
    //    static_assert(std::is_void<TChar>::value,
    //                  "setting write position is not supported with StreamAdapter");
    return {};
  }

  void flush() {
    writeBufferToStream();
    //    _ostream->flush();
  }

  size_t writtenBytesCount() const {
    //    static_assert(std::is_void<TChar>::value,
    //                  "`writtenBytesCount` cannot be used with stream adapter");
    // streaming doesn't return written bytes
    return 0u;
  }

  template <size_t SIZE, typename T>
  void writeBytes(const T& v) {
    static_assert(std::is_integral<T>(), "");
    static_assert(sizeof(T) == SIZE, "");
    writeSwappedValue(&v, bitsery::details::ShouldSwap<Config, T>{});
  }

  template <size_t SIZE, typename T>
  void writeBuffer(const T* buf, size_t count) {
    static_assert(std::is_integral<T>(), "");
    static_assert(sizeof(T) == SIZE, "");
    writeSwappedBuffer(buf, count, bitsery::details::ShouldSwap<Config, T>{});
  }

  template <typename T>
  void writeBits(const T&, size_t) {
    static_assert(std::is_void<T>::value,
        "Bit-packing is not enabled.\nEnable by call to `enableBitPacking`) or "
        "create Serializer with bit packing enabled.");
  }

private:
  using TResizable = std::integral_constant<bool, bitsery::traits::ContainerTraits<Buffer>::isResizable>;
  using diff_t = typename std::iterator_traits<BufferIt>::difference_type;

  template <size_t SIZE>
  void writeInternalValue(const TValue* data) {
    writeInternalImpl(data, SIZE);
  }

  void writeInternalBuffer(const TValue* data, size_t size) { writeInternalImpl(data, size); }

  void writeInternalImpl(const TValue* data, size_t size) {
    const auto newOffset = _currOffset + size;

    if (newOffset <= 256) {
      std::copy_n(data, size, _beginIt + static_cast<diff_t>(_currOffset));
      _currOffset = newOffset;
    }
    else {
      writeBufferToStream();
      // write buffer directly to stream
      _write_func((uint8_t*)data, size, _udata);
    }
  }

  void writeBufferToStream() {
    _write_func((uint8_t*)std::addressof(*_beginIt), _currOffset, _udata);
    _currOffset = 0;
  }

  template <typename T>
  void writeSwappedValue(const T* v, std::true_type) {
    const auto res = bitsery::details::swap(*v);
    writeInternalValue<sizeof(T)>(reinterpret_cast<const TValue*>(&res));
  }

  template <typename T>
  void writeSwappedValue(const T* v, std::false_type) {
    writeInternalValue<sizeof(T)>(reinterpret_cast<const TValue*>(v));
  }

  template <typename T>
  void writeSwappedBuffer(const T* v, size_t count, std::true_type) {
    std::for_each(v, std::next(v, count), [this](const T& inner_v) {
      const auto res = bitsery::details::swap(inner_v);
      writeInternalValue<sizeof(T)>(reinterpret_cast<const TValue*>(&res));
    });
  }

  template <typename T>
  void writeSwappedBuffer(const T* v, size_t count, std::false_type) {
    writeInternalBuffer(reinterpret_cast<const TValue*>(v), count * sizeof(T));
  }

  zs::write_function_t _write_func;
  void* _udata;
  Buffer _buf;
  BufferIt _beginIt;
  size_t _currOffset;
};

namespace zs {

template <class Stream>
  requires Stream::is_serializer
void serialize_string_object(Stream& stream, const object& obj, size_t max_size) {
  zs::engine* eng = stream.template context<zs::engine*>();

  zs::string str(obj.is_string() ? obj.get_string_unchecked() : "", eng);
  stream.text1b(str, max_size);
}

template <class Stream>
  requires(!Stream::is_serializer)
void serialize_string_object(Stream& stream, object& obj, size_t max_size) {
  zs::engine* eng = stream.template context<zs::engine*>();
  zs::string str(eng);
  stream.text1b(str, max_size);
  obj = zs::_s(eng, str);
}

template <class Stream>
  requires Stream::is_serializer
void serialize_table_object(Stream& stream, const object& obj, size_t max_size) {
  zs::engine* eng = stream.template context<zs::engine*>();

  zs::string output("{}", eng);

  if (obj.is_table()) {
    if (auto err = obj.to_json(output)) {
    }
  }

  stream.text1b(output, max_size);
}

template <class Stream>
  requires(!Stream::is_serializer)
void serialize_table_object(Stream& stream, object& obj, size_t max_size) {
  zs::engine* eng = stream.template context<zs::engine*>();
  zs::string str(eng);

  stream.text1b(str, max_size);

  zs::json_parser jparser(eng);
  [[maybe_unused]] zs::error_result err = jparser.parse(nullptr, str, nullptr, obj);

  //  obj = zs::_s(eng, str);
}

template <class Stream>
  requires Stream::is_serializer
void serialize_function_prototype_object(Stream& stream, const object& obj) {
  stream.object(function_prototype_object::as_proto(obj));
}

template <class Stream>
  requires(!Stream::is_serializer)
void serialize_function_prototype_object(Stream& stream, object& obj) {
  zs::engine* eng = stream.template context<zs::engine*>();
  obj = zs::function_prototype_object::create(eng);
  stream.object(function_prototype_object::as_proto(obj));
}

template <typename Stream>
void serialize(Stream& stream, zs::line_info_op_t& o) {
  stream.value8b(o.line);
  stream.value8b(o.column);
  stream.value8b(o.op_index);
  stream.value1b(o.op);
}

template <typename Stream>
void serialize(Stream& stream, zs::local_var_info_t& o) {
  serialize_string_object(stream, o.name, 100);
  stream.value8b(o.start_op);
  stream.value8b(o.end_op);
  stream.value8b(o.pos);
  stream.value4b(o.mask);
  stream.value8b(o.custom_mask);
  stream.value1b(o.flags);
}

template <typename Stream>
void serialize(Stream& stream, zs::captured_variable& o) {
  serialize_string_object(stream, o.name, 100);
  stream.value8b(o.src);
  stream.value1b(o.type);
  stream.boolValue(o.is_weak);
}

template <typename Stream>
void serialize(Stream& stream, zs::function_prototype_object& fpo) {

  serialize_string_object(stream, fpo._source_name, 100);
  serialize_string_object(stream, fpo._name, 100);

  serialize_table_object(stream, fpo._module_info, 1024);

  stream.value8b(fpo._stack_size);
  stream.container(fpo._vlocals, 100);

  stream.container(
      fpo._literals, 100, [](Stream& stream, zs::object& obj) { serialize_string_object(stream, obj, 100); });

  stream.container(fpo._default_params, 100, [](Stream& stream, zs::int_t& val) { stream.value8b(val); });

  stream.container(fpo._parameter_names, 100,
      [](Stream& stream, zs::object& obj) { serialize_string_object(stream, obj, 100); });

  stream.container(fpo._restricted_types, 100,
      [](Stream& stream, zs::object& obj) { serialize_string_object(stream, obj, 100); });

  stream.container(fpo._captures, 100);

  stream.value8b(fpo._n_capture);
  //  stream.value8b(fpo._export_table_target);
  stream.container(fpo._line_info, 100);

  stream.container(fpo._functions, 20,
      [](Stream& stream, zs::object& obj) { serialize_function_prototype_object(stream, obj); });

  stream.container1b(fpo._instructions._data, 1000);
}

} // namespace zs.

//-------------------------------------------------------------------
namespace zs {

inline constexpr object k_fpo_uid = _sv("__fpo_object__");
inline constexpr object k_fpo_type_id = _sv("proto");

inline constexpr user_data_content k_fpo_udata_content
    = { [](zs::engine* eng, zs::raw_pointer_t ptr) {
         ((function_prototype_object*)ptr)->~function_prototype_object();
       },
        [](const zs::object_base& obj, std::ostream& stream) -> error_result {
          stream << "function prototype object";
          return {};
        },
        k_fpo_uid, k_fpo_type_id };

function_prototype_object& function_prototype_object::as_proto(const object_base& obj) {
  return obj.as_udata().data_ref<function_prototype_object>();
}

bool function_prototype_object::is_proto(const object_base& obj) noexcept {
  return obj.is_user_data(&k_fpo_udata_content);
}

function_prototype_object::function_prototype_object(zs::engine* eng)
    : engine_holder(eng)
    , _vlocals(zs::allocator<zs::local_var_info_t>(eng))
    , _literals(zs::allocator<zs::object>(eng))
    , _default_params(zs::allocator<zs::int_t>(eng))
    , _parameter_names(zs::allocator<zs::object>(eng))
    , _restricted_types(zs::allocator<zs::object>(eng))
    , _functions(zs::allocator<zs::object>(eng))
    , _captures(zs::allocator<zs::captured_variable>(eng))
    , _line_info(zs::allocator<zs::line_info_op_t>(eng))

#if ZS_DEBUG
    , _debug_line_info(zs::allocator<zs::line_info_op_t>(eng))
#endif

    , _instructions(eng) {
}

object function_prototype_object::create(zs::engine* eng) {
  if (user_data_object* uobj
      = user_data_object::create(eng, sizeof(function_prototype_object), &k_fpo_udata_content)) {

    zb_placement_new((void*)uobj->data()) function_prototype_object(eng);

    uobj->set_no_default_none();
    return object(uobj, false);
  }

  return nullptr;
}

int_t function_prototype_object::get_parameters_count() const noexcept { return _parameter_names.size(); }

int_t function_prototype_object::get_default_parameters_count() const noexcept {
  return _default_params.size();
}

int_t function_prototype_object::get_minimum_required_parameters_count() const noexcept {
  return _parameter_names.size() - _default_params.size();
}

bool function_prototype_object::is_possible_parameter_count(size_t sz) const noexcept {
  const size_t p_count = _parameter_names.size();
  const size_t def_count = _default_params.size();

  if (!def_count) {
    return sz == p_count;
  }

  return sz >= (p_count - def_count) and sz <= p_count;
}

bool function_prototype_object::is_valid_parameters(
    zs::vm_ref vm, zb::span<const object> params, int_t& n_type_match) const noexcept {
  const size_t p_count = _parameter_names.size();
  const size_t def_count = _default_params.size();
  const size_t n_params = params.size();

  if (!def_count and n_params != p_count) {
    return false;
  }

  if (!(n_params >= (p_count - def_count) and n_params <= p_count)) {
    return false;
  }

  n_type_match = 0;
  for (size_t k = 0; k < n_params; k++) {
    const zs::local_var_info_t* vinfo = find_local(_parameter_names[k]);
    if (!vinfo or !vinfo->mask) {
      continue;
    }

    const object& param_obj = params[k];

    if (!param_obj.has_type_mask(vinfo->mask)) {
      return false;
    }

    if (!vinfo->custom_mask) {
      n_type_match++;
      continue;
    }

    zs::object typeobj;
    if (auto err = vm->type_of(param_obj, typeobj)) {
      return false;
    }

    int_t r_sz = _restricted_types.size();
    bool found = false;

    for (int i = 0; i < r_sz; i++) {
      if ((vinfo->custom_mask & (1 << i)) and typeobj == _restricted_types[i]) {
        found = true;
        break;
      }
    }

    if (!found) {
      return false;
    }

    n_type_match++;
  }

  return true;
}

zs::object function_prototype_object::find_function(std::string_view name) const {

  for (const auto& f : _functions) {
    if (function_prototype_object::as_proto(f)._name == name) {
      return f;
    }
  }

  return nullptr;
}

const zs::local_var_info_t* function_prototype_object::find_local(std::string_view name) const {
  for (const auto& vinfo : _vlocals) {
    if (vinfo.name == name) {
      return &vinfo;
    }
  }

  return nullptr;
}
const zs::local_var_info_t* function_prototype_object::find_local(const zs::object& name) const {
  for (const auto& vinfo : _vlocals) {
    if (vinfo.name == name) {
      return &vinfo;
    }
  }

  return nullptr;
}

void function_prototype_object::debug_print(std::ostream& stream) const {
  zb::stream_print(stream, "Name: ", _name, "\n");
  zb::stream_print(stream, "SourceName: ", _source_name, "\n");
  zb::stream_print(stream, "Stack Size : ", _stack_size, "\n");
  zb::stream_print(stream, "Locals : ", _vlocals, "\n");
  zb::stream_print(stream, "Literals : ", _literals, "\n");
  zb::stream_print(stream, "_n_capture : ", _n_capture, "\n");
  zb::stream_print(stream, "_default_params : ", _default_params, "\n");
  zb::stream_print(stream, "_functions : ", _functions, "\n");
  zb::stream_print(stream, "_restricted_types : ", _restricted_types, "\n");

  zb::stream_print(stream, "Instructions:\n");
  _instructions.debug_print(stream);
  //  _instructions.serialize(stream);
}

zs::error_result function_prototype_object::save(zb::byte_vector& buffer) {
  using output_adapter = bitsery::OutputBufferAdapter<zb::byte_vector>;
  using serializer_type = bitsery::Serializer<output_adapter, zs::engine*>;

  serializer_type s(_engine, buffer);
  s.object(*this);
  buffer.resize(s.adapter().writtenBytesCount());

  buffer.insert(buffer.begin(), k_compiled_header.begin(), k_compiled_header.end());
  return {};
}

zs::error_result function_prototype_object::save(zs::write_function_t write_func, void* udata) {
  using serializer_type
      = bitsery::Serializer<write_function_buffer_adapter<bitsery::DefaultConfig>, zs::engine*>;

  if (auto err = write_func(k_compiled_header.data(), k_compiled_header.size(), udata)) {
    return err;
  }

  serializer_type s(_engine, write_function_buffer_adapter<bitsery::DefaultConfig>(write_func, udata));
  s.object(*this);
  s.adapter().flush();
  return {};
}

zs::error_result function_prototype_object::load(zb::byte_view buffer) {
  using input_adapter = bitsery::InputBufferAdapter<zb::byte_view>;
  using deserializer_type = bitsery::Deserializer<input_adapter, zs::engine*>;

  deserializer_type ds(_engine,
      input_adapter(buffer.begin() + k_compiled_header.size(), buffer.size() - k_compiled_header.size()));

  ds.object(*this);

  if (bitsery::ReaderError derror = ds.adapter().error(); derror != bitsery::ReaderError::NoError) {
    switch (derror) {
    case bitsery::ReaderError::DataOverflow:
      return zs::error_code::memory_error;

    case bitsery::ReaderError::InvalidData:
      return zs::error_code::memory_error;

    case bitsery::ReaderError::InvalidPointer:
      return zs::error_code::memory_error;

    default:
      return zs::error_code::memory_error;
    }
  }

  if (!ds.adapter().isCompletedSuccessfully()) {
    return zs::error_code::memory_error;
  }

  return {};
}
} // namespace zs.
