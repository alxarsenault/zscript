#!/usr/local/bin/zscript

zs::add_import_directory("/Users/alexarse/Develop/zscript/examples/subfolder");

const fs = import("fs");
const m1 = import("sub_01");
var a = 123;

//zs::print(vargs);

export function main(args) {
  zs::print("Main", fs.exe_path(),"PWD", fs.pwd());

  return 0;
}
