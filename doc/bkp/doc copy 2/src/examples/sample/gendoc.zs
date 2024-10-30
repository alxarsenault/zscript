const fs = import("fs");

const examples_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/examples");
const sections_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/sections");
const content_path = fs.path(ZSCRIPT_DOC_DIRECTORY + "/src/content.zson");
const output_path = fs.path(ZSCRIPT_DOC_DIRECTORY + "/data/sections.js");

var content = { examples = stable() };

// Add all files from the examples directory to the content table.
for(var p : examples_dir.list_recursive(".zs")) {
  content.examples[p.dirname][p.stem] = fs.string_file(p);
}

// Add all files from the sections directory to the content table.
for(var p : sections_dir.list(".zson")) {
  content[p.stem] = fs.json_file(p, content);
}

fs.file(output_path, fs.mode.write)
  .write("const documentation_content = ")
  .write_json(fs.json_file(content_path, content));