
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

#include <filesystem>
#include <span>
#include <string>
#include <zbase/sys/file_view.h>
#include <zbase/container/byte.h>

TEST_CASE("proto-serialize") {
  const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_03.zs";
  zs::vm vm;
  zs::engine* eng = vm.get_engine();
  zb::byte_vector data_buffer;

  {
    zb::file_view file;
    REQUIRE(!file.open(filepath));

    zs::object fpo;
    zs::jit_compiler compiler(eng);
    if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
      FAIL(compiler.get_error());
    }

    REQUIRE(fpo.is_function_prototype());
    //    fpo.as_proto().debug_print();

    if (auto err = fpo.as_proto().save(data_buffer)) {
      FAIL(err.message());
    }

    std::ofstream output_file;
    output_file.open(
        ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/compiler_03.zsc", std::ios_base::out | std::ios_base::binary);
    REQUIRE(output_file.is_open());

    output_file.write((const char*)data_buffer.data(), data_buffer.size());
    output_file.close();
  }

  {
    zb::file_view file;
    REQUIRE(!file.open(ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/compiler_03.zsc"));

    zs::function_prototype_object* fpo_ptr = zs::function_prototype_object::create(eng);
    if (auto err = fpo_ptr->load(file.content())) {
      FAIL(err.message());
    }

    zs::object fpo(fpo_ptr, false);

    //    zb::print("--------------------------------");
    //    fpo.as_proto().debug_print();

    zs::object closure = zs::object::create_closure(eng, fpo, vm->get_root());

    zs::object result;
    if (auto err = vm->call(closure, { vm->get_root() }, result)) {
      FAIL(vm.get_error());
    }

    //    zb::print(result);
  }
}

TEST_CASE("proto-serialize2") {
  const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_03.zs";
  zs::vm vm;
  zs::engine* eng = vm.get_engine();
  //  zb::byte_vector data_buffer;

  {
    zb::file_view file;
    REQUIRE(!file.open(filepath));

    zs::object fpo;
    zs::jit_compiler compiler(eng);
    if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
      FAIL(compiler.get_error());
    }

    REQUIRE(fpo.is_function_prototype());
    //    fpo.as_proto().debug_print();

    std::vector<uint8_t> data_buffer;
    zs::write_function_t write_func
        = [](const uint8_t* content, size_t size, void* udata) -> zs::error_result {
      std::vector<uint8_t>* buffer = (std::vector<uint8_t>*)udata;
      buffer->insert(buffer->end(), content, content + size);
      return {};
    };

    if (auto err = fpo.as_proto().save(write_func, &data_buffer)) {
      FAIL(err.message());
    }

    std::ofstream output_file;
    output_file.open(
        ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/compiler_04.zsc", std::ios_base::out | std::ios_base::binary);
    REQUIRE(output_file.is_open());

    output_file.write((const char*)data_buffer.data(), data_buffer.size());
    output_file.close();
  }
  //
  {
    zb::file_view file;
    REQUIRE(!file.open(ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/compiler_04.zsc"));

    zs::function_prototype_object* fpo_ptr = zs::function_prototype_object::create(eng);
    if (auto err = fpo_ptr->load(file.content())) {
      FAIL(err.message());
    }

    zs::object fpo(fpo_ptr, false);

    //    zb::print("--------------------------------");
    //    fpo.as_proto().debug_print();

    zs::object closure = zs::object::create_closure(eng, fpo, vm->get_root());

    zs::object result;
    if (auto err = vm->call(closure, { vm->get_root() }, result)) {
      FAIL(vm.get_error());
    }

    //    zb::print(result);
  }
}

inline std::string left_aligned_string(std::string s, size_t n, char fill_char) {
  if (s.size() < n) {
    s.insert(s.end(), n - s.size(), fill_char);
  }

  return s;
}

inline std::string right_aligned_string(std::string s, size_t n, char fill_char) {
  if (s.size() < n) {
    s.insert(s.begin(), n - s.size(), fill_char);
  }

  return s;
}

// #ifdef ZS_COMPILER_DEV
// inline void print_dev_token(const zs::dev_token_info& dtoken) {
//   //  zb::sprint("001:006 identifier   small_string ---");
//   //  zb::sprint("LINE    TOKEN        VALUE TYPE   VALUE");
//   //  zb::sprint(" LINE     TOKEN       VALUE TYPE  VALUE");
//
//   std::string tok_value = dtoken.value.convert_to_string();
//   if (tok_value.size() > 15) {
//     tok_value.resize(12);
//     tok_value.push_back('.');
//     tok_value.push_back('.');
//     tok_value.push_back('.');
//   }
//
//   zb::print<"">("| ", right_aligned_string(std::to_string(dtoken.linfo.line),
//   3, '0'), ":",
//       right_aligned_string(std::to_string(dtoken.linfo.column), 3, '0'), " |
//       ", left_aligned_string(zs::token_to_string(dtoken.token), 13, ' '), " |
//       ",
//       left_aligned_string(zs::get_object_type_name(dtoken.value.get_type()),
//       12, ' '), " | ", left_aligned_string(tok_value, 15, ' '), " |");
// }
//
// inline void print_compiler_dev_info(const zs::compiler& compiler) {
//   zb::print("┌─────────┬───────────────┬──────────────┬─────────────────┐");
//   zb::print("│  LINE   │     TOKEN     │  VALUE TYPE  │ VALUE           │");
//   zb::print("├─────────┼───────────────┼──────────────┼─────────────────┤");
//   for (const auto& k : compiler._dev._info) {
//     print_dev_token(k);
//   }
//   zb::print("└─────────┴───────────────┴──────────────┴─────────────────┘\n");
// }
// #endif // ZS_COMPILER_DEV

// TEST_CASE("dsdsdsdsdsdsdsdsd") {
//   const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_03.zs";
//   zs::vm vm;
//   zs::engine* eng = vm.get_engine();
//
//   zb::file_view file;
//   REQUIRE(!file.open(filepath));
//
//   zs::object fpo;
//   zs::jit_compiler compiler(eng);
//   if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
//     zb::print(compiler.get_error());
//     REQUIRE(false);
//   }
//
//   REQUIRE(fpo.is_function_prototype());
//   fpo.as_proto().debug_print();
//
//
//
//   zs::object closure = zs::object::create_closure(eng, fpo, vm->get_root());
//
//   zs::object result;
//   if (auto err = vm->call(closure, { vm->get_root() }, result)) {
//     zb::print(vm.get_error());
//     REQUIRE(false);
//   }
//
//   zb::print(result);
//   //  compiler._lexer.
//   //  compiler.print_handler();
//
//   //    print_compiler_dev_info(compiler);
//   //    zb::print("| 001:006 | local        | none         |");
//
//   //    return comp.compile(shared_state, content, filename);
//   //    auto res = zs::compile(shared_state, file.str(), filepath);
//   //    REQUIRE(!res);
// }

//
// #include <cereal/archives/binary.hpp>
// #include <bitsery/bitsery.h>
// #include <bitsery/adapter/buffer.h>
// #include <bitsery/traits/vector.h>
// #include <bitsery/traits/string.h>
//
//// some helper types
//
//// namespace bitsery::ext {
////
//// class Object
////{
//// public:
////   template<typename Ser , class Fnc>
////   void serialize(Ser& ser,
////                  const zs::object& obj,
////                  Fnc&& fnc) const
////   {
////     std::string s(obj.get_string_unchecked());
////     fnc(ser, s);
////   }
////
////   template<typename Des, typename Fnc>
////   void deserialize(Des& des,
////                    zs::object& obj,
////                    Fnc&& fnc) const
////   {
////     std::string res;
////     fnc(des, res);
////
////     obj = zs::_ss(res);
//////    obj = std::chrono::duration<T, Period>{ res };
////  }
////};
////
////}
//
// namespace bitsery::traits {
//
//// template<>
//// struct ExtensionTraits<ext::Object, zs::object>
////{
////   using TValue = std::string;
////   static constexpr bool SupportValueOverload = true;
////   static constexpr bool SupportObjectOverload = false;
////   static constexpr bool SupportLambdaOverload = false;
//// };
//
// template <typename T>
// struct ContainerTraits<zs::vector<T>> : public StdContainer<zs::vector<T>, true, true> {};
//
// template <typename T, size_t N>
// struct ContainerTraits<zs::small_vector<T, N>> : public StdContainer<zs::small_vector<T, N>, true, true>
// {};
//
//} // namespace bitsery::traits.
//
// namespace zs {
//
// template <class Stream>
//  requires Stream::is_serializer
// void serialize_string_object(Stream& stream, const object& obj, size_t max_size) {
//  zs::engine* eng = stream.template context<zs::engine*>();
//
//  zs::string str(obj.is_string() ? obj.get_string_unchecked() : "", eng);
//  stream.text1b(str, max_size);
//}
//
// template <class Stream>
//  requires(!Stream::is_serializer)
// void serialize_string_object(Stream& stream, object& obj, size_t max_size) {
//  zs::engine* eng = stream.template context<zs::engine*>();
//  zs::string str(eng);
//  stream.text1b(str, max_size);
//  obj = zs::_s(eng, str);
//}
//
// template <class Stream>
//  requires Stream::is_serializer
// void serialize_function_prototype_object(Stream& stream, const object& obj) {
//  stream.object(obj.as_proto());
//}
//
// template <class Stream>
//  requires(!Stream::is_serializer)
// void serialize_function_prototype_object(Stream& stream, object& obj) {
//  zs::engine* eng = stream.template context<zs::engine*>();
//  zs::function_prototype_object* fpo_result = zs::function_prototype_object::create(eng);
//  stream.object(*fpo_result);
//  obj = zs::object(fpo_result, false);
//}
//
// template <typename Stream>
// void serialize(Stream& stream, zs::line_info_op_t& o) {
//  stream.value8b(o.line);
//  stream.value8b(o.column);
//  stream.value8b(o.op_index);
//  stream.value1b(o.op);
//}
//
// template <typename Stream>
// void serialize(Stream& stream, zs::local_var_info_t& o) {
//  zs::engine* eng = stream.template context<zs::engine*>();
//  serialize_string_object(stream, o._name, 100);
//  stream.value8b(o._start_op);
//  stream.value8b(o._end_op);
//  stream.value8b(o._pos);
//  stream.value4b(o._mask);
//  stream.value8b(o._custom_mask);
//  stream.boolValue(o._is_const);
//}
//
// template <typename Stream>
// void serialize(Stream& stream, zs::captured_variable& o) {
//  zs::engine* eng = stream.template context<zs::engine*>();
//  serialize_string_object(stream, o.name, 100);
//  stream.value8b(o.src);
//  stream.value1b(o.type);
//  stream.boolValue(o.is_weak);
//}
//
// template <typename Stream>
// void serialize(Stream& stream, zs::function_prototype_object& fpo) {
//  zs::engine* eng = stream.template context<zs::engine*>();
//
//  serialize_string_object(stream, fpo._source_name, 100);
//  serialize_string_object(stream, fpo._name, 100);
//  serialize_string_object(stream, fpo._module_name, 100);
//
//  stream.value8b(fpo._stack_size);
//  stream.container(fpo._vlocals, 100);
//
//  stream.container(
//      fpo._literals, 100, [](Stream& stream, zs::object& obj) { serialize_string_object(stream, obj, 100);
//      });
//
//  stream.container(fpo._default_params, 100, [](Stream& stream, zs::int_t& val) { stream.value8b(val); });
//
//  stream.container(fpo._parameter_names, 100,
//      [](Stream& stream, zs::object& obj) { serialize_string_object(stream, obj, 100); });
//
//  stream.container(fpo._restricted_types, 100,
//      [](Stream& stream, zs::object& obj) { serialize_string_object(stream, obj, 100); });
//
//  stream.container(fpo._captures, 100);
//
//  stream.value8b(fpo._n_capture);
//  stream.value8b(fpo._export_table_target);
//  stream.container(fpo._line_info, 100);
//
//  stream.container(fpo._functions, 20,
//      [](Stream& stream, zs::object& obj) { serialize_function_prototype_object(stream, obj); });
//
//  stream.container1b(fpo._instructions._data, 1000);
//
//}
//
//} // namespace zs.
//
// using Buffer = std::vector<uint8_t>;
// using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
// using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
//
// TEST_CASE("KL") {
//  const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_03.zs";
//  zs::vm vm;
//  zs::engine* eng = vm.get_engine();
//
//  zb::file_view file;
//  REQUIRE(!file.open(filepath));
//
//  zs::object fpo;
//  zs::jit_compiler compiler(eng);
//  if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
//    zb::print(compiler.get_error());
//    REQUIRE(false);
//  }
//
//  REQUIRE(fpo.is_function_prototype());
//  fpo.as_proto().debug_print();
//
//  // create buffer to store data
//  Buffer buffer;
//  bitsery::Serializer<OutputAdapter, zs::engine*> ser{ eng, buffer };
//  ser.object(fpo.as_proto());
//  ser.adapter().flush();
//  size_t writtenSize = ser.adapter().writtenBytesCount();
//
//  bitsery::Deserializer<InputAdapter, zs::engine*> des{ eng, InputAdapter{ buffer.begin(), writtenSize } };
//
//  zs::function_prototype_object* fpo_result = zs::function_prototype_object::create(eng);
//
//  des.object(*fpo_result);
//
//  std::pair<bitsery::ReaderError, bool> state
//      = { des.adapter().error(), des.adapter().isCompletedSuccessfully() };
//  zb::print(state);
//
//  fpo_result->debug_print();
//}
//
//// TEST_CASE("KL") {
////   const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/compiler/compiler_03.zs";
////   zs::vm vm;
////   zs::engine* eng = vm.get_engine();
////
////   zb::file_view file;
////   REQUIRE(!file.open(filepath));
////
////   zs::object fpo;
////   zs::jit_compiler compiler(eng);
////   if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
////     zb::print(compiler.get_error());
////     REQUIRE(false);
////   }
////
////   REQUIRE(fpo.is_function_prototype());
////   fpo.as_proto().debug_print();
////
////   // create buffer to store data
////   Buffer buffer;
////   // use quick serialization function,
////   // it will use default configuration to setup all the nesessary steps
////   // and serialize data to container
//////  zs::local_var_info_t vinfo;
//////  vinfo._start_op = 1289;
//////  vinfo._end_op = 121289;
//////  vinfo._pos = 123;
//////  vinfo._custom_mask = 1237832;
//////  vinfo._is_const = true;
//////
//////  zs::captured_variable cvar;
//////  zs::captured_variable cvar_result;
//////  cvar.name = zs::_ss("ASKLJD");
////
////  zs::local_var_info_t vinfores;
////  vinfo._name = zs::_s(eng, "dsdfdsfdfdfdsfdsfdsfds");
////  bitsery::Serializer<OutputAdapter, zs::engine*> ser{ eng, buffer };
////  ser.object(vinfo);
////  ser.object(cvar);
////  ser.adapter().flush();
////  auto writtenSize = ser.adapter().writtenBytesCount();
////
////  bitsery::Deserializer<InputAdapter, zs::engine*> des{ eng, InputAdapter{ buffer.begin(), writtenSize }
///}; /  des.object(vinfores); /  des.object(cvar_result); /  std::pair<bitsery::ReaderError, bool> state / =
///{ des.adapter().error(), des.adapter().isCompletedSuccessfully() };
////
//////  zb::print(vinfores._name, vinfores._start_op, vinfores._end_op, vinfores._pos, vinfores._mask,
//////      vinfores._custom_mask, vinfores._is_const);
//////
//////  zb::print(cvar_result.name);
////}
