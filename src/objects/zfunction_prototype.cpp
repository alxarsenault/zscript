#include "objects/zfunction_prototype.h"
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>

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
void serialize_function_prototype_object(Stream& stream, const object& obj) {
  stream.object(obj.as_proto());
}

template <class Stream>
  requires(!Stream::is_serializer)
void serialize_function_prototype_object(Stream& stream, object& obj) {
  zs::engine* eng = stream.template context<zs::engine*>();
  zs::function_prototype_object* fpo_result = zs::function_prototype_object::create(eng);
  stream.object(*fpo_result);
  obj = zs::object(fpo_result, false);
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
  serialize_string_object(stream, o._name, 100);
  stream.value8b(o._start_op);
  stream.value8b(o._end_op);
  stream.value8b(o._pos);
  stream.value4b(o._mask);
  stream.value8b(o._custom_mask);
  stream.boolValue(o._is_const);
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
  serialize_string_object(stream, fpo._module_name, 100);

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
  stream.value8b(fpo._export_table_target);
  stream.container(fpo._line_info, 100);

  stream.container(fpo._functions, 20,
      [](Stream& stream, zs::object& obj) { serialize_function_prototype_object(stream, obj); });

  stream.container1b(fpo._instructions._data, 1000);
}

} // namespace zs.

namespace zs {

function_prototype_object::function_prototype_object(zs::engine* eng)
    : reference_counted_object(eng, zs::object_type::k_function_prototype)
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

function_prototype_object* function_prototype_object::create(zs::engine* eng) {
  function_prototype_object* fpo = internal::zs_new<function_prototype_object>(eng, eng);
  return fpo;
}

zs::object function_prototype_object::find_function(std::string_view name) const {

  for (const auto& f : _functions) {
    if (f._fproto->_name == name) {
      return f;
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

zs::error_result function_prototype_object::serialize(zb::byte_vector& buffer) {
  using output_adapter = bitsery::OutputBufferAdapter<zb::byte_vector>;
  using serializer_type = bitsery::Serializer<output_adapter, zs::engine*>;

  serializer_type s(_engine, buffer);
  s.object(*this);
  buffer.resize(s.adapter().writtenBytesCount());
  return {};
}

zs::error_result function_prototype_object::serialize(zs::write_function_t write_func, void* udata) {
  using serializer_type
      = bitsery::Serializer<write_function_buffer_adapter<bitsery::DefaultConfig>, zs::engine*>;

  serializer_type s(_engine, write_function_buffer_adapter<bitsery::DefaultConfig>(write_func, udata));
  s.object(*this);
  s.adapter().flush();
  return {};
}

zs::error_result function_prototype_object::deserialize(zb::byte_view buffer) {
  using input_adapter = bitsery::InputBufferAdapter<zb::byte_view>;
  using deserializer_type = bitsery::Deserializer<input_adapter, zs::engine*>;

  deserializer_type ds(_engine, input_adapter(buffer.begin(), buffer.size()));

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

//
// zs::error_result function_prototype_object::serialize(zb::byte_vector& buffer) {
////  zs::engine* eng = _engine;
////  zs::object stable = zs::object::create_table(eng);
////  zs::object_unordered_map<zs::object>& sbl = stable._table->get_map();
////
////  sbl[zs::_ss("source_name")] = _source_name;
////  sbl[zs::_ss("name")] = _name;
////  sbl[zs::_ss("stack_size")] = _stack_size;
////  sbl[zs::_ss("literals")] = zs::object::create_array(eng, _literals);
////  sbl[zs::_s(eng, "default_params")] = zs::object::create_array(eng, _default_params);
////  sbl[zs::_s(eng, "parameter_names")] = zs::object::create_array(eng, _parameter_names);
////  sbl[zs::_s(eng, "restricted_types")] = zs::object::create_array(eng, _restricted_types);
////
////  sbl[zs::_ss("n_capture")] = _n_capture;
////
////  size_t sz = 0;
////  if (auto err = stable.to_binary(buffer, sz, 0)) {
////    return err;
////  }
//
//  return {};
//}
} // namespace zs.
