const fs = import("fs");
print(__FILE__)
print(PAGE_NAME);
print(vargs);

const template_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/template");
const pages_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/pages");
const page_path = ZSCRIPT_DOC_DIRECTORY + "/pages/" + PAGE_NAME + ".html";

var doc = {
  content = fs.string_file(page_path)
};

doc.add = function (keyname, prefix = "") {
  var filename = keyname + ".html";
  var data = fs.string_file(ZSCRIPT_DOC_DIRECTORY + "/template/" + filename);
  this.content = this.content.replace_first("%" + keyname + "\n", data);
}

doc.add("head");
doc.add("nav", "  ");
doc.add("sidebar", "    ");
doc.add("load-scripts", "  ");

//print(doc.content);
fs.file(ZSCRIPT_DOC_DIRECTORY + "/" + PAGE_NAME + ".html", fs.mode.write).write(doc.content);