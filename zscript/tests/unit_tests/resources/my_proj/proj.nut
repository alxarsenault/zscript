
// fs_add_import_directory("/Users/aarsenault/develop/wp/modules/script/tests/resources");

fs.import_directory(fs.get_source_directory() + "/..");

// if( ::rawin("std")) {
//     printf("%s\n", "dsadsadsadsa")
//     }



local utils = import("utils");
local uuid = import("uuid");
// local utils = import("/Users/aarsenault/develop/wp/modules/script/tests/resources/utils.nut");
// local json = import("/Users/aarsenault/develop/wp/modules/script/tests/resources/json.nut");

local s0 = "john";
utils.print(uuid.gen());
// local k0 = std.Serializer.serialize(s0);
// std.print(type(k0));

// local k1 = std.Serializer.deserialize(k0);
// std.print(type(k1));

// local j = std.path("/john");
// std.print(j);
// local diresss = fs_get_import_directories();

// utils.print_array(fs.get_import_directories());
// utils.print_array(exe._include_directories);
local p = project("Peter");
local proj_dir = p.directory();

local sources = list_directory(proj_dir, ["*.h", "*.cpp"]);
local exe = p.create_executable("test", sources);
exe.include_directory(proj_dir / "include");
exe.include_directory(proj_dir / "src", true);
exe.link_directory(proj_dir / "build");
exe.link_libraries(["wp-core", "wp-json"]);
// utils.print_array(exe._include_directories);

p.generate();