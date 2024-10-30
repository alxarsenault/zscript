const fs = import("fs");

var pages = fs.path("doc/template/pages").list(".html");

for(var page : pages) {
  var dst_path = fs.path("doc/site/" + page.stem + ".html");

  print(page, "->", dst_path);

  var doc = {
    content = fs.string_file(page)
  };

  doc.add = function (keyname, prefix = "") {
    var filename = keyname + ".html";
    var data = fs.string_file("doc/template/components/" + filename);
    this.content = this.content.replace_first("%" + keyname + "\n", data);
  }

  doc.add("head");
  doc.add("nav", "  ");
  doc.add("sidebar", "    ");
  doc.add("load-scripts", "  ");

  //print(doc.content);
  fs.file(dst_path, fs.mode.write).write(doc.content);
}