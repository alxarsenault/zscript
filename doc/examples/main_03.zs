#!/usr/local/bin/zscript

const argparser = import("argparser");

// Main.
export function main(args) {
  var fpath = fs::path("/Users/alexarse/Develop/zscript/examples/example_01_package");
  var build_directory = fs::path(fpath.tostring() + "/build");

  build_directory.mkdir();

  var res = sys::exec("cd " + build_directory.tostring());
  zs::print(res);

//res =  zs::exec("cmake -S " + fpath.tostring() + " -B " + build_directory.tostring());
//zs::print(res);
 //-S <path-to-source>          = Explicitly specify a source directory.
  //-B <path-to-build>           = Explicitly specify a build directory.

  res = sys::exec("pwd");
  zs::print(res);

  res = sys::exec("which cmake");
  zs::print("ENVVVVVVVVV\n\n\n", res);

  return 0;
}
