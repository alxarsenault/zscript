#include "unit_tests.h"
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

inline constexpr std::string_view k_fs_filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/text_01.txt";

static inline void fs_test_function(zs::virtual_machine& vm) {
  zs::engine* eng = vm.get_engine();
  zs::var& root = vm.get_root();
  zs::table_object* root_tbl = root._table;

  root_tbl->set(zs::_ss("filepath"), zs::_s(eng, k_fs_filepath));
  root_tbl->set(zs::_ss("dirpath"), zs::_s(eng, ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data"));
  root_tbl->set(zs::_ss("dirpath2"), zs::_s(eng, ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/"));
  root_tbl->set(zs::_ss("newpath"), zs::_s(eng, ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/john.txt"));
  std::filesystem::remove(ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/john.txt");
}

#define ZS_PATH_TEST(...) ZS_CODE_TEST(__VA_ARGS__, fs_test_function)

ZS_PATH_TEST("fs.path.create_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.tostring();
)""") {
  REQUIRE(vm.top() == k_fs_filepath);
}

ZS_PATH_TEST("fs.path.create_02", R"""(
var fs = import("fs")
var p1 = fs.path(filepath);
var p2 = fs.path(p1);
return p2.tostring();
)""") {
  REQUIRE(vm.top() == k_fs_filepath);
}

TEST_CASE("fs.path.create_03") {
  constexpr std::string_view code = R"""(
var fs = import("fs")
var p = fs.path(12345);
return p.tostring();
)""";

  zs::vm vm;
  zs::object closure;
  REQUIRE(!vm->compile_buffer(code, "test", closure));
  REQUIRE(closure.is_closure());
  REQUIRE(!vm->call_from_top_opt(closure));
}

TEST_CASE("fs.path.create_04") {
  constexpr std::string_view code = R"""(
var fs = import("fs")
var p = fs.path({a=12});
return p.tostring();
)""";

  zs::vm vm;
  zs::object closure;
  REQUIRE(!vm->compile_buffer(code, "test", closure));
  REQUIRE(closure.is_closure());
  REQUIRE(!vm->call_from_top_opt(closure));
}

ZS_PATH_TEST("fs.path.filename_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.filename;
)""") {
  REQUIRE(vm.top() == "text_01.txt");
}

ZS_PATH_TEST("fs.path.get_filename_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.get_filename();
)""") {
  REQUIRE(vm.top() == "text_01.txt");
}

ZS_PATH_TEST("fs.path.stem_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.stem;
)""") {
  REQUIRE(vm.top() == "text_01");
}

ZS_PATH_TEST("fs.path.stem_02", R"""(
var fs = import("fs")
var p = fs.path("banana");
return p.stem;
)""") {
  REQUIRE(vm.top() == "banana");
}

ZS_PATH_TEST("fs.path.stem_03", R"""(
var fs = import("fs")
var p = fs.path("banana.g.txt");
return p.stem;
)""") {
  REQUIRE(vm.top() == "banana");
}

ZS_PATH_TEST("fs.path.get_stem_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.get_stem();
)""") {
  REQUIRE(vm.top() == "text_01");
}

ZS_PATH_TEST("fs.path.has_stem", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.has_stem();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.is_directory_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.is_directory();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.is_directory_02", R"""(
var fs = import("fs")
var p = fs.path("/bingo.txt");
return p.is_directory();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.is_directory_03", R"""(
var fs = import("fs")
var p = fs.path("/usr");
return p.is_directory();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.is_directory_04", R"""(
var fs = import("fs")
var p = fs.path(dirpath2);
return p.is_directory();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.dirname_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.dirname;
)""") {
  REQUIRE(vm.top() == "data");
}

ZS_PATH_TEST("fs.path.parent_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.parent.tostring();
)""") {
  REQUIRE(vm.top() == ZSCRIPT_TESTS_RESOURCES_DIRECTORY);
}

ZS_PATH_TEST("fs.path.get_dirname_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.get_dirname();
)""") {
  REQUIRE(vm.top() == ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data");
}

ZS_PATH_TEST("fs.path.has_filename_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.has_filename();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.has_filename_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.has_filename();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.has_filename_03", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/");
return p.has_filename();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.has_filename_04", R"""(
var fs = import("fs")
var p = fs.path("john.txt");
return p.has_filename();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.has_filename_05", R"""(
var fs = import("fs")
var p = fs.path("bob");
return p.has_filename();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.has_filename_06", R"""(
var fs = import("fs")
var p = fs.path("/bob");
return p.has_filename();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.has_filename_07", R"""(
var fs = import("fs")
var p = fs.path("/bob/");
return p.has_filename();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.exists_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.exists();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.exists_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.exists();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.exists_03", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "dsjkdjskj");
return p.exists();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.is_file_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.is_file();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.is_file_02", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.is_file();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.is_file_03", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/dsjkdjskj.txt");
return p.is_file();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.touch_01", R"""(
var fs = import("fs")
var p = fs.path(newpath);
return p.touch();
)""") {
  REQUIRE(vm.top() == true);
  REQUIRE(std::filesystem::exists(ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/john.txt"));
  std::filesystem::remove(ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/john.txt");
}

ZS_PATH_TEST("fs.path.prune_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.prune().tostring();
)""") {
  REQUIRE(vm.top() == k_fs_filepath);
}

ZS_PATH_TEST("fs.path.prune_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.prune().tostring();
)""") {
  REQUIRE(vm.top() == ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data");
}

ZS_PATH_TEST("fs.path.prune_03", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/..");
return p.prune().tostring();
)""") {
  REQUIRE(vm.top() == ZSCRIPT_TESTS_RESOURCES_DIRECTORY);
}

ZS_PATH_TEST("fs.path.split_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.split();
)""") {
  std::filesystem::path p(k_fs_filepath);

  std::vector<std::string> ps;
  for (const auto& it : p) {
    ps.push_back(it.string());
  }

  const zs::var& pobj = vm.top();
  for (size_t i = 0; i < ps.size(); i++) {
    REQUIRE(*pobj[i] == ps[i]);
  }
}

ZS_PATH_TEST("fs.path.file_size_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.file_size();
)""") {
  REQUIRE(vm.top() == 11);
}

ZS_PATH_TEST("fs.path.mkdir_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/johndoe/ddd/abc/john.txt");
return p.mkdir();
)""") {}

ZS_PATH_TEST("fs.path.list_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/dont_touch");
return p.list();
)""") {
  //    zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.list_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/dont_touch");
return p.list([".txt"]);
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.list_03", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/dont_touch");
return p.list(".json");
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.list_04", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/dont_touch");
return p.list("json");
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.list_recursive_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.list_recursive();
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.list_recursive_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.list_recursive(["json"]);
)""") {
  // zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.list_recursive_03", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.list_recursive("json");
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.rmdir_recursive_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/johndoe");
return p.rmdir_recursive();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.rename_01", R"""(
var fs = import("fs")
var p = fs.path(dirpath + "/renamed.txt");

p.touch();

return p.rename(dirpath + "/renamed_02.txt");
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.path.access_time_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return [p.access_time(), now()];
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.path.creation_time_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return [p.creation_time(), now()];
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.pwd_01", R"""(
var fs = import("fs")
var p = fs.pwd();
return p;
)""") {
  REQUIRE(vm.top() == std::filesystem::current_path().string());
}

ZS_PATH_TEST("fs.exe_path_01", R"""(
var fs = import("fs")
return fs.exe_path();
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.exe_home_01", R"""(
var fs = import("fs")
return fs.home();
)""") {
  //  zb::print(vm.top());
}

ZS_PATH_TEST("fs.is_absolute_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.is_absolute();
)""") {
  REQUIRE(vm.top() == true);
}

ZS_PATH_TEST("fs.is_absolute_02", R"""(
var fs = import("fs")
var p = fs.path("john/doe");
return p.is_absolute();
)""") {
  REQUIRE(vm.top() == false);
}

ZS_PATH_TEST("fs.path.extension_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.extension;
)""") {
  REQUIRE(vm.top() == ".txt");
}

ZS_PATH_TEST("fs.path.extension_02", R"""(
var fs = import("fs")
var p = fs.path(filepath);
p.extension = ".json";
return p.extension;
)""") {
  REQUIRE(vm.top() == ".json");
}

ZS_PATH_TEST("fs.path.get_extension_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.get_extension();
)""") {
  REQUIRE(vm.top() == ".txt");
}

ZS_PATH_TEST("fs.path.get_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p[0];
)""") {
  REQUIRE(vm.top() == "/");
}

ZS_PATH_TEST("fs.path.get_02", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p[1];
)""") {
  REQUIRE(vm.top() == "Users");
}

ZS_PATH_TEST("fs.path.get_03", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p[-1];
)""") {
  REQUIRE(vm.top() == "text_01.txt");
}

ZS_PATH_TEST("fs.path.get_04", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p[-2];
)""") {
  REQUIRE(vm.top() == "data");
}

ZS_PATH_TEST("fs.path.read_all_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.read_all();
)""") {
  REQUIRE(vm.top() == "bacon-dance");
}

ZS_PATH_TEST("fs.path.content_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.content;
)""") {
  REQUIRE(vm.top() == "bacon-dance");
}

ZS_PATH_TEST("fs.path.component_count_01", R"""(
var fs = import("fs")
var p = fs.path("/A/B/C/john.txt");
return p.component_count();
)""") {
  REQUIRE(vm.top() == 5);
}

ZS_PATH_TEST("fs.path.component_count_02", R"""(
var fs = import("fs")
var p = fs.path("john.txt");
return p.component_count();
)""") {
  REQUIRE(vm.top() == 1);
}

ZS_PATH_TEST("fs.path.component_count_03", R"""(
var fs = import("fs")
var p = fs.path("/");
return p.component_count();
)""") {
  REQUIRE(vm.top() == 1);
}

ZS_PATH_TEST("fs.path.component_count_04", R"""(
var fs = import("fs")
var p = fs.path("./");
return p.component_count();
)""") {
  REQUIRE(vm.top() == 2);
}

ZS_PATH_TEST("fs.path.length_01", R"""(
var fs = import("fs")
var p = fs.path("/A/B/C/john.txt");
return p.length();
)""") {
  REQUIRE(vm.top() == 15);
}

ZS_PATH_TEST("fs.path.file_type_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.file_type();
)""") {
  REQUIRE(vm.top() == "regular");
}

ZS_PATH_TEST("fs.path.file_type_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.file_type();
)""") {
  REQUIRE(vm.top() == "directory");
}

ZS_PATH_TEST("fs.path.type_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.type;
)""") {
  REQUIRE(vm.top() == "regular");
}

ZS_PATH_TEST("fs.path.type_02", R"""(
var fs = import("fs")
var p = fs.path(dirpath);
return p.type;
)""") {
  REQUIRE(vm.top() == "directory");
}

ZS_PATH_TEST("fs.path.perm_string_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.perm_string();
)""") {
  REQUIRE(vm.top() == "-rw-r--r--");
}

ZS_PATH_TEST("fs.path.perm_01", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.perm();
)""") {
  REQUIRE(vm.top() == 420);
}

ZS_PATH_TEST("fs.path.perm_02", R"""(
var fs = import("fs")
var p = fs.path(filepath);
return p.owner_read && p.owner_write && p.owner_exec == false;
)""") {
  REQUIRE(vm.top() == true);
}
