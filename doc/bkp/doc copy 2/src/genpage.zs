const fs = import("fs");

const template_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/template");

var doc = {
  content = fs.string_file(ZSCRIPT_DOC_DIRECTORY + "/template/overview.html")
};

doc.add = function (keyname, prefix = "") {
  var filename = keyname + ".html";
  var data = fs.string_file(ZSCRIPT_DOC_DIRECTORY + "/template/" + filename);

  if(prefix) {
    data = data.add_line_prefix(prefix);
  }

  this.content = this.content.replace_first("%" + keyname + "\n", data);
}

doc.add("head");
doc.add("nav", "  ");
doc.add("sidebar", "    ");
doc.add("load-scripts", "  ");

//print(doc.content);

fs.file(ZSCRIPT_DOC_DIRECTORY + "/overview.html", fs.mode.write).write(doc.content);