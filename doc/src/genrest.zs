const fs = import("fs");

const template_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/template");
const pages_dir = fs.path(ZSCRIPT_DOC_DIRECTORY + "/pages");

var doc = {
  content = fs.string_file(ZSCRIPT_DOC_DIRECTORY + "/pages/restriction.html")
};

doc.add = function (keyname, prefix = "") {
  var filename = keyname + ".html";
  var data = fs.string_file(ZSCRIPT_DOC_DIRECTORY + "/template/" + filename);

  //if(prefix) {
  //  data = data.add_line_prefix(prefix);
  //}

  this.content = this.content.replace_first("%" + keyname + "\n", data);
}

doc.add("head");
doc.add("nav", "  ");
doc.add("sidebar", "    ");
doc.add("load-scripts", "  ");

print(doc.content);

//var pages = pages_dir.list_recursive(".html");
//for(var page : pages) {
//  print(page);
//  //content.examples[ex.dirname][ex.stem] = fs.string_file(ex);
//}


fs.file(ZSCRIPT_DOC_DIRECTORY + "/restriction.html", fs.mode.write).write(doc.content);