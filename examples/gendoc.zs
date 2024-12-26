const fs = import("fs");

const examples_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/examples");
const sections_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/sections");
const content_path = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/content.zson");
const output_path = fs.path(ZSCRIPT_DOC_DIRECTORY + "/data/sections.js");

global function main(args) {
//function global.main(args) {
  print("DLKSJDSKJDLSKD");
  var content = { examples = stable() };

  // The examples directory should look like this:
  // - src/examples/basic/var_decl_01.zs
  // - src/examples/basic/var_decl_02.zs
  // - src/examples/functions/fct_decl_01.zs
  //
  // A subdirectory in the examples directory will have it's content added to a
  // sub-table with the subdirectory name,
  // e.g. `src/examples/basic/var_decl_01.zs` will be found in `examples.basic.var_decl_01`.

  // Add all files from the examples directory to the content table.
  var examples = examples_dir.list_recursive(".zs");
  for(int i = 0; i < examples.size(); i++) {
    var p = examples[i];
    content.examples[p.dirname][p.stem] = fs.string_file(p);
  }

  // Add all files from the sections directory to the content table.
  var sections = sections_dir.list(".zson");
  for(int i = 0; i < sections.size(); i++) {
    var p = sections[i];
    content[p.stem] = fs.json_file(p, content);
  }

  fs.file(output_path, fs.mode.write)
    .write("const documentation_content =")
    .write_json(fs.json_file(content_path, content));

  return 0;
}
