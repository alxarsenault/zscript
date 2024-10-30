const fs = import("fs");

print(__FILE__)
print(PAGE_NAME);
print(vargs);

var doc = {
  content = fs.string_file("doc/pages/" + PAGE_NAME + ".html")
};

doc.add = function (keyname, prefix = "") {
  var filename = keyname + ".html";
  var data = fs.string_file("doc/template/" + filename);
  this.content = this.content.replace_first("%" + keyname + "\n", data);
}

doc.add("head");
doc.add("nav", "  ");
doc.add("sidebar", "    ");
doc.add("load-scripts", "  ");

//print(doc.content);
fs.file("doc/" + PAGE_NAME + ".html", fs.mode.write).write(doc.content);