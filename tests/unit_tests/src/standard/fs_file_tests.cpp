#include "ztests.h"
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

inline constexpr std::string_view k_fs_test_filepath
    = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/file_test.txt";

static inline void fs_path_test_function(zs::virtual_machine& vm) {
  zs::engine* eng = vm.get_engine();
  zs::var& root = vm.get_root();
  zs::table_object* root_tbl = root._table;

  root_tbl->set(zs::_ss("dirpath"), zs::_s(eng, ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data"));
  root_tbl->set(zs::_s(eng, "TEST_FILE_PATH"), zs::_s(eng, k_fs_test_filepath));
  std::filesystem::remove(k_fs_test_filepath);
}

#define ZS_FS_FILE_TEST(...) ZS_CODE_TEST(__VA_ARGS__, fs_path_test_function)

// TEST_FILE_PATH doesn't exists, it should not be opened if we try to open it
// with `fs.mode.read | fs.mode.write`.
ZS_FS_FILE_TEST("fs.file.create_01", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH);
return file.is_open();
)""") {
  REQUIRE(vm.top() == false);
}

// TEST_FILE_PATH doesn't exists, it should not be opened if we try to open it
// with `fs.mode.read`.
ZS_FS_FILE_TEST("fs.file.create_02", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.read);
return file.is_open();
)""") {
  REQUIRE(vm.top() == false);
}

// TEST_FILE_PATH doesn't exists, it should not be opened if we try to open it
// with `fs.mode.create`.
ZS_FS_FILE_TEST("fs.file.create_03", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.create);
return file.is_open();
)""") {
  REQUIRE(vm.top() == false);
}

// TEST_FILE_PATH doesn't exists, it should not be opened if we try to open it
// with `fs.mode.create | fs.mode.read`.
ZS_FS_FILE_TEST("fs.file.create_04", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.create | fs.mode.read);
return file.is_open();
)""") {
  REQUIRE(vm.top() == false);
}

// TEST_FILE_PATH doesn't exists, it should be opened if we open it
// with `fs.mode.write`.
ZS_FS_FILE_TEST("fs.file.create_05", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.write);
return file.is_open();
)""") {
  REQUIRE(vm.top() == true);
}

// We write "file-test" to the file.
ZS_FS_FILE_TEST("fs.file.create_06", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.write);
file.write("file-test");
file.close();
)""") {
  // Make sure "file-test" was written to the file.
  zb::file_view fv;
  REQUIRE(!fv.open(std::string(k_fs_test_filepath).c_str()));
  REQUIRE(fv.str() == "file-test");
}

ZS_FS_FILE_TEST("fs.file.create_07", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.write);
file.write("file-test");
file.close();

file = fs.file(TEST_FILE_PATH);
var value = file.read();
file.close();
return value;
)""") {
  REQUIRE(vm.top() == "file-test");
}

ZS_FS_FILE_TEST("fs.file.create_08", R"""(
var fs = import("fs")
var file = fs.file(TEST_FILE_PATH, fs.mode.w);
file.write("file-test");
file.close();

file = fs.file(TEST_FILE_PATH, fs.mode.w | fs.mode.a);
file.write(" bingo");
file.close();
)""") {
  // Make sure "file-test bingo" was written to the file.
  zb::file_view fv;
  REQUIRE(!fv.open(std::string(k_fs_test_filepath).c_str()));
  REQUIRE(fv.str() == "file-test bingo");
}

ZS_FS_FILE_TEST("fs.file.get_path_01", R"""(
var fs = import("fs")

// Create an fs.path object.
var path = fs.path(TEST_FILE_PATH);

// Create an empty file with the fs.path object.
var file = fs.file(path, fs.mode.w);

// Get the path of the file.
var fpath = file.path;

// Close the file.
file.close();

// Delete the file.
var removed = path.remove();

return [fpath, removed];
)""") {
  const zs::var& res = vm.top();
  REQUIRE(res.is_array());
  REQUIRE(res.as_array()[0] == k_fs_test_filepath);
  REQUIRE(res.as_array()[1] == true);
  REQUIRE(!std::filesystem::exists(k_fs_test_filepath));
}

ZS_FS_FILE_TEST("fs.file.get_path_02", R"""(
var fs = import("fs")

// Create an fs.path object.
var path = fs.path(TEST_FILE_PATH);

// Create an empty file with the fs.path object.
var file = fs.file(path, fs.mode.w);

// Get the path of the file.
var fpath = file.get_path();

// Close the file.
file.close();

// Delete the file.
var removed = path.remove();

return [fpath, removed];
)""") {
  const zs::var& res = vm.top();
  REQUIRE(res.is_array());
  REQUIRE(res.as_array()[0] == k_fs_test_filepath);
  REQUIRE(res.as_array()[1] == true);
  REQUIRE(!std::filesystem::exists(k_fs_test_filepath));
}

//
// ZS_FS_FILE_TEST("fs.file.get_path_01", R"""(
// var fs = import("fs")
// var p = fs.file(filepath);
// return p.get_path();
//)""") {
//   REQUIRE(vm.top() == k_fs_filepath);
// }

// ZS_FS_FILE_TEST("fs.file.path_01", R"""(
// var fs = import("fs")
// var p = fs.file(filepath);
// return p.path;
//)""") {
//   REQUIRE(vm.top() == k_fs_filepath);
// }

// ZS_FS_FILE_TEST("fs.file.open_01", R"""(
// var fs = import("fs")
// var f = fs.file(filepath);
// return f.open();
//)""") {
//   REQUIRE(vm.top() == true);
// }

// ZS_FS_FILE_TEST("fs.file.open_02", R"""(
// var fs = import("fs")
// var f = fs.file(filepath);
// var is_open = f.open();
// f.close();
// return is_open;
//)""") {
//   REQUIRE(vm.top() == true);
// }

// ZS_FS_FILE_TEST("fs.file.write_01", R"""(
// var fs = import("fs")
// var f = fs.file(newpath);
// if(f.open() == false) {
//   return false;
// }
//
// f.write("Banana", " ", 32);
//
// return fs.openmode.append | fs.openmode.read;
//)""") {
//   REQUIRE(vm.top() == (std::ios::app | std::ios::in));
// }
