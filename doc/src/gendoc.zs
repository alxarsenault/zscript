const fs = import("fs");

const examples_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/examples");
const sections_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/sections");
const pages_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/pages");
const contents_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/contents");
const output_path = fs.path(ZSCRIPT_DOC_DIRECTORY + "/data/sections.js");
const output_list_path = fs.path(ZSCRIPT_DOC_DIRECTORY + "/data/pages-list.js");

var content = {
  examples = stable(),
  contents = stable()
};

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
for(var ex : examples) {
  content.examples[ex.dirname][ex.stem] = fs.string_file(ex);
}

// Add all files from the contents directory to the content table.
var contents = contents_dir.list_recursive(".html");
for(var f : contents) {
  content.contents[f.dirname][f.stem] = fs.string_file(f);
}

// Add all files from the sections directory to the content table.
var sections = sections_dir.list(".zson");
for(var s : sections) {
  content[s.stem] = fs.json_file(s, content);
}

var pages_content = {};
var pages_list = [];

const pages = pages_dir.list(".zson");
for(var p : pages) {
  pages_content[p.stem] = fs.json_file(p, content);
  pages_list.push(p.stem);

}

fs.file(output_path, fs.mode.write)
  .write("const documentation_content = ")
  .write_json(pages_content);


fs.file(output_list_path, fs.mode.write)
  .write("const pages_list = ")
  .write_json(pages_list);