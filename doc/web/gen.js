const fs = require('fs');
const path = require('path');
const Handlebars = require("handlebars");
// const template = Handlebars.compile("Name: {{name}}");
// console.log(template({ name: "Nils" }));

Handlebars.registerHelper("code-block", function (options) {
  console.log(options.fn(this))
  return new Handlebars.SafeString(`<figure>
  <pre><code class="zscript-code">`+ options.fn(this) + `</code></pre>
  <figcaption class="blockquote-footer text-center">` + options.hash.label + `</figcaption>
</figure>`
  );
});


Handlebars.registerHelper("terminal-block", function (options) {
  return new Handlebars.SafeString(
    `   <div class="terminal " role="alert">
              <div class=fakeMenu>
                <div class="fakeButtons fakeClose"></div>
                <div class="fakeButtons fakeMinimize"></div>
                <div class="fakeButtons fakeZoom"></div>
                <div class="fakeTitle">Output</div>
              </div>
              <div class="fakeScreen">
                <pre><code class="terminal-code language-shell">` + options.fn(this) + `</code></pre>
              </div>
            </div>`
  );
});



var content = {};
const components = fs.readdirSync("template/components");

components.forEach(file => {
  const name = path.parse(file).name;
  const fcontent = fs.readFileSync("template/components/" + file, "utf-8");
  content[name] = fcontent;
});



const pages = fs.readdirSync("template/pages");

pages.forEach(file => {
  var template = fs.readFileSync("template/pages/" + file, "utf-8");
  var renderTemplate = Handlebars.compile(template);
  const result = renderTemplate(content);

  fs.writeFileSync("dist/" + file, result);
});

// var head = fs.readFileSync("template/components/head.html", "utf-8");
// var head = fs.readFileSync("template/components/head.html", "utf-8");
// var head = fs.readFileSync("template/components/head.html", "utf-8");
// var head = fs.readFileSync("template/components/head.html", "utf-8");



// questionsJson is a buffer
// var html = buildHtml(JSON.parse(questionsJson));

// function buildHtml(fileData) {
//   var template = fs.readFileSync(path.resolve(path.join(__dirname, 'template.hbs')), "utf-8");
//   var renderTemplate = Handlebars.compile(template);
//   // console.log(fileData);
//   var html = renderTemplate(fileData);
//   // Write to build folder. Copy the built file and deploy
//   fs.writeFile("./build/index.html", html, err => {
//     if (err) console.log(err);
//     console.log("File written succesfully");
//   });
// }