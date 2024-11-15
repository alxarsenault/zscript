#include "ztests.h"
#include "lang/zpreprocessor.h"

TEST_CASE("macro.01") {
  static constexpr std::string_view code = R"""(

// C1.
@macro bingo(a) { a }
// C2.
@macro bango(a) {
  a
}
// C3.
@macro banana(a, b, c) { a, b, c }
@macro banana2(a, b = 1, c = 2) { $banana(a, b, c) }

var v1 = $bingo(32);
var v2 = $bango(33);
var v3 = [$banana(1, 2, 3)];
var v4 = [$banana2("A")];
var v5 = [$banana2(v4, "S")];
var v6 = @include("raw_array.zs");
var v7 = @include("raw_array");
return [v1, v2, v3, v4, v5, v6, v7];
)""";

  zs::vm vm;
  vm.get_engine()->add_import_directory(ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/preprocessor");
  zs::preprocessor pp(vm.get_engine());

  zs::object obj;
  if (auto err = pp.preprocess(code, ztest::s_current_test_name, obj)) {
    zb::print(err, pp.get_error());
  }

  // zb::print(obj);
  REQUIRE(obj.is_string());

//    zb::print(pp._macros);

  zs::object closure;

  if (auto err = vm->compile_buffer(obj.get_string_unchecked(), ztest::s_current_test_name, closure)) {
    FAIL(vm.get_error());
  }

  REQUIRE(closure.is_closure());

  zs::object value;
  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
    FAIL(vm.get_error());
  }

  // zb:print(value);
  REQUIRE(value.is_array());
  REQUIRE(value.as_array()[0] == 32);
  REQUIRE(value.as_array()[1] == 33);
  REQUIRE(value.as_array()[2] == zs::_a(vm.get_engine(), { 1, 2, 3 }));
  REQUIRE(value.as_array()[3] == zs::_a(vm.get_engine(), { zs::_ss("A"), 1, 2 }));
  REQUIRE(value.as_array()[4].is_array());
  REQUIRE(value.as_array()[4].as_array()[0] == zs::_a(vm.get_engine(), { zs::_ss("A"), 1, 2 }));
  REQUIRE(value.as_array()[4].as_array()[1] == "S");
  REQUIRE(value.as_array()[4].as_array()[2] == 2);
  REQUIRE(value.as_array()[5] == zs::_a(vm.get_engine(), { 4, 5, zs::_ss("Alex") }));
  REQUIRE(value.as_array()[6] == zs::_a(vm.get_engine(), { 4, 5, zs::_ss("Alex") }));
}

// TEST_CASE("preprocessor.01") {
//   static constexpr std::string_view code = R"""(
//@macro EXPECT(cond, msg = "") { ztest.check(cond, msg, __THIS_LINE__, __LINE__) }
//
// var a = 32;
//$EXPECT(a == 32);
//$EXPECT(a == 32, "Wrong");
// return a;
//)""";
//
//   zs::vm vm;
//   zs::preprocessor pp(vm.get_engine());
//
//   zs::object obj;
//   if(auto err = pp.preprocess(code, "preprocessor.01", obj)) {
//     zb::print(pp.get_error(), err);
//   }
//
////  zb::print(obj);
//}
//
//
// TEST_CASE("preprocessor.02") {
//  static constexpr std::string_view code = R"""(
//
//$Alex();
//
// return 32;
//)""";
//
//  zs::vm vm;
//  zs::preprocessor pp(vm.get_engine());
//
//  zs::macro m = { zs::_ss("Alex"), zs::_s(vm.get_engine(), "print(\"Alex\")"), zs::_a(vm.get_engine(), 0),
//  zs::_a(vm.get_engine(), 0) }; pp._macros.push_back(m);
//
//  zs::object obj;
//  if(auto err = pp.preprocess(code, "preprocessor.01", obj)) {
//    zb::print(pp.get_error(), err);
//  }
//
////  zb::print(obj);
//}
//
//
// TEST_CASE("preprocessor.03") {
//  static constexpr std::string_view code = R"""(
//
//$Alex();
//
// return 32;
//)""";
//
//  zs::vm vm;
//  zs::preprocessor pp(vm.get_engine());
//
//  zs::macro m = { zs::_ss("Alex"), zs::_s(vm.get_engine(), "print(\"GeorgeAlex\")"), zs::_a(vm.get_engine(),
//  0),  zs::_a(vm.get_engine(), 0) }; pp._macros.push_back(m);
//
//  zs::object obj;
//  if(auto err = pp.preprocess(code, "preprocessor.03", obj, "$")) {
//    zb::print(pp.get_error(), err);
//  }
//
//}

// TEST_CASE("preprocessor.04") {
////  static constexpr std::string_view code = R"""(
////@macro JOHNSON(arg) { print("JOHNSON", arg) }
////@macro PETER(arg, ab = "Steeve") { arg + ab }
////
////@macro JOHN(a) JOHNSON(a)
////
////JOHNSON("Bacon");
////
////JOHNSON(PETER("Bingo"));
////
////var a = 32;
////
////JOHNSON("Banana");
////
////
////JOHN("PETER");
////)""";
//
//  static constexpr std::string_view code = R"""(
//
// var cool = "123";
//@macro Mom(a) {a+2}
//@macro JOHNSON(arg) { print("JOHNSON", "JOHN", arg, $Mom(cool)) }
//
//@macro JOHN(a, b = "Empty") {
//  $JOHNSON(a);
//  $JOHNSON(b)
//}
//
// var a = 56;
//$JOHN("PETER");
//
//$JOHN(a);
//
//$JOHN(a, "QUI");
//
//@macro PETERSON(A)
//{
//  $JOHN(A)
//}
//
//
//$PETERSON("BIMBO"){};
//)""";
//
//  zs::vm vm;
//  zs::preprocessor pp(vm.get_engine());
//
//  zs::object obj;
//  if(auto err = pp.preprocess(code, "preprocessor.04", obj )) {
//    zb::print(err, pp.get_error());
//  }
//
//  zb::print(obj);
//}
//
//
// TEST_CASE("preprocessor.05") {
//
//  static constexpr std::string_view code = R"""(
//@include("common.zs");
//
//@macro Bingo(filename) {
//  print(filename);
//  @include(filename);
//}
//
//@macro my_include(filename) {
//  @include(filename);
//}
//
//$Bingo("var_array.zs")
//
// var arr = $my_include("raw_array.zs");
//
// return arr;
//)""";
//
//  zs::vm vm;
//  vm.get_engine()->add_import_directory(ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/preprocessor");
//  zs::preprocessor pp(vm.get_engine());
//
//  zs::object obj;
//  if(auto err = pp.preprocess(code, "preprocessor.54", obj )) {
//    zb::print(err, pp.get_error());
//  }
//
//  zb::print(  obj);
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(obj.get_string_unchecked(), "module.02", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object value;
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//
//  REQUIRE(value.is_array());
//
//}
//
//
//
// TEST_CASE("preprocessor.06") {
//
//  static constexpr std::string_view code = R"""(
//@import("common.zs");
//
//@import("common.zs");
//
//@import("common.zs");
//
//@include("common.zs");
//)""";
//
//  zs::vm vm;
//  vm.get_engine()->add_import_directory( ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/preprocessor");
//  zs::preprocessor pp(vm.get_engine());
//
//  zs::object obj;
//  if(auto err = pp.preprocess(code, "preprocessor.54", obj )) {
//    zb::print(err, pp.get_error());
//  }
//
////  zb::print(  obj);
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(obj.get_string_unchecked(), "module.02", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object value;
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//
////  REQUIRE(value.is_array());
//
//}
//
//
// TEST_CASE("preprocessor.07") {
//  static constexpr std::string_view code = R"""(
//@include("common");
//
// var a = @counter;
// var b = @counter;
// var c = @counter;
//
// var t = {
//  k = @counter,
//
//}
//
// print(@counter + 3278);
//
//@macro bingo(a) {a + @counter}
//
// print($bingo(3234));
//
// var @uuid(asj) = 3232;
// var @uuid  = 111;
//
// var johnson = 32;
//
//$asj = 908790;
//
////print($bingo($asj));
//
//
//
//@macro my_uuid() {
//  @uuid(peter)
//}
//
// var $my_uuid() = "DDD";
//
//
// var ghgj = @str(vbn);
//
//
// var ghgjkl = @str(var $my_uuid());
// var ghgjkljk = @str(@uuid);
// var ghgjkljk = @str($my_uuid());
// var ghgjkljk = @str(@counter);
//
//)""";
//
//  zs::vm vm;
//  vm.get_engine()->add_import_directory( ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/preprocessor");
//  zs::preprocessor pp(vm.get_engine());
//
//  zs::object obj;
//  if(auto err = pp.preprocess(code, ztest::s_current_test_name, obj )) {
//    zb::print(err, pp.get_error());
//  }
//
//  zb::print(  obj);
//  REQUIRE(obj.is_string());
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(obj.get_string_unchecked(), ztest::s_current_test_name, closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object value;
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//
////  REQUIRE(value.is_array());
//
//}

// #include "subprocess.h"
// #include <sys/mman.h>
// #include <unistd.h>
//
// #include <sys/mman.h>
// #include <pthread.h>
// #include <libkern/OSCacheControl.h>
// enum class shell_name {
//   bash,
//   sh,
//   zsh
// };
//
// const char* get_shell_path(shell_name sh) {
//   switch(sh) {
//     case shell_name::bash:  return "/bin/bash";
//     case shell_name::zsh:  return "/bin/zsh";
//     case shell_name::sh:  return "/bin/sh";
//     default:  return "/bin/zsh";
//   }
// }
//
// const char* get_shell_name(shell_name sh) {
//   switch(sh) {
//     case shell_name::bash:  return "bash";
//     case shell_name::zsh:  return "zsh";
//     case shell_name::sh:  return "sh";
//     default:  return "zsh";
//   }
// }
//
// zb::optional_result<int>  bash(shell_name shname,
//     const char* cmd, std::span<const uint8_t> input, zb::vector<uint8_t>* output, zb::vector<uint8_t>*
//     error, std::span<const char*> env) noexcept {
//
//   const int READ_END = 0;
//   const int WRITE_END = 1;
//
//   int infd[2] = { 0, 0 };
//   int outfd[2] = { 0, 0 };
//   int errfd[2] = { 0, 0 };
//
//   auto cleanup = [&]() {
//     ::close(infd[READ_END]);
//     ::close(infd[WRITE_END]);
//
//     ::close(outfd[READ_END]);
//     ::close(outfd[WRITE_END]);
//
//     ::close(errfd[READ_END]);
//     ::close(errfd[WRITE_END]);
//   };
//
//   auto rc = ::pipe(infd);
//   if (rc < 0) {
//     return zb::errc::broken_pipe;
//   }
//
//   rc = ::pipe(outfd);
//   if (rc < 0) {
//     ::close(infd[READ_END]);
//     ::close(infd[WRITE_END]);
//     return zb::errc::broken_pipe;
//   }
//
//   rc = ::pipe(errfd);
//   if (rc < 0) {
//     ::close(infd[READ_END]);
//     ::close(infd[WRITE_END]);
//
//     ::close(outfd[READ_END]);
//     ::close(outfd[WRITE_END]);
//
//     return zb::errc::broken_pipe;
//   }
//
//   pid_t pid = fork();
//
//   // Parent.
//   if (pid > 0)
//   {
//     // Parent does not read from stdin.
//     ::close(infd[READ_END]);
//
//     // Parent does not write to stdout.
//     ::close(outfd[WRITE_END]);
//
//     // Parent does not write to stderr.
//     ::close(errfd[WRITE_END]);
//
//     if (!input.empty()) {
//       if (::write(infd[WRITE_END], input.data(), input.size()) < 0) {
//         return zb::errc::broken_pipe;
//       }
//
//       // Done writing.
//       ::close(infd[WRITE_END]);
//     }
//   }
//
//   // Child.
//   else if (pid == 0)
//   {
//     ::dup2(infd[READ_END], STDIN_FILENO);
//     ::dup2(outfd[WRITE_END], STDOUT_FILENO);
//     ::dup2(errfd[WRITE_END], STDERR_FILENO);
//
//     // Child does not write to stdin.
//     ::close(infd[WRITE_END]);
//
//     // Child does not read from stdout.
//     ::close(outfd[READ_END]);
//
//     // Child does not read from stderr.
//     ::close(errfd[READ_END]);
//
//     std::vector<const char*> env_vec;
//     env_vec.resize(env.size() + 1);
//
//     for(size_t i = 0; i < env.size(); i++) {
//       env_vec[i] = env[i];
//     }
//
//     env_vec.back() = nullptr;
//     const char* shell_path =get_shell_path(shname);
//     const char* sname =get_shell_name(shname);
//
//     ::execle(shell_path, sname, "-c", cmd, nullptr, env_vec.data());
//     ::exit(EXIT_SUCCESS);
//   }
//
//   // PARENT
//   if (pid < 0) {
//     cleanup();
//     return zb::errc::broken_pipe;
//   }
//
//   int p_status = 0;
//   ::waitpid(pid, &p_status, 0);
//
//   std::array<uint8_t, 256> buffer;
//
//   ssize_t bytes = 0;
//
//   if (output) {
//     do {
//       bytes = ::read(outfd[READ_END], buffer.data(), buffer.size());
//       output->insert(output->end(), buffer.data(), buffer.data() + bytes);
//     } while (bytes > 0);
//   }
//
//   if (error) {
//     do {
//       bytes = ::read(errfd[READ_END], buffer.data(), buffer.size());
//       error->insert(error->end(), buffer.data(), buffer.data() + bytes);
//     } while (bytes > 0);
//   }
//
//   int status = 0;
//   if (WIFEXITED(p_status)) {
//     status = WEXITSTATUS(p_status);
//   }
//
//   cleanup();
//
//   return { status };
// }
//
// extern char **environ;
//
//
// std::vector<std::string> get_all_env() {
//   char **s = environ;
//   std::vector<std::string> vals;
//   for (; *s; s++) {
//     vals.push_back(*s);
//   }
//
//   return vals;
// }
//
// TEST_CASE("subprocess") {
//
//   std::string msg = "banana";
//   zb::vector<uint8_t> output;
//
//   zb::print<",">(89, 888);
//   zb::print(90);
//   const char* env[] = {"BANAN=STEETE", "JOHN=789"};
//   zb::print(std::span<const char*>(env), 8978);
//   zb::print<zb::separator{"--", zb::print_options::endl_each_vector_item}>(std::span<const char*>(env),
//   8978); bash(shell_name::zsh, "echo $BANAN $JOHN; printenv", std::span<const uint8_t>((const
//   uint8_t*)msg.data(), msg.size()), &output, nullptr, env);
//
//   zb::print(std::string_view((const char*)output.data(), output.size()));
//   std::cout << zb::stringifier( std::span<const char*>(env)) << "\n";
//   std::cout << zb::stringifier( "op") << "\n";
//
//
//   zb::print(789, zb::stringifier(std::span<const char*>(env)) );;
//   std::cout << zb::stringifier( "op") << "\n";
//   std::cout << zb::stringifier ( std::array<int, 3>{1, 2, 3}) << "\n";
//
//   struct ABC {
//     int a;
//   };
//
//   zb::print(ABC{32});
// }
//
//
