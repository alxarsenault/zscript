const Handlebars = require("handlebars");
const hljs = require("highlight.js");

module.exports = function () {
  Handlebars.registerHelper("link", function (str) {
    return new Handlebars.SafeString(
      `<link href="` + str + `" rel="stylesheet" type="text/css">`
    );
  });

  Handlebars.registerHelper("pad", function (height) {
    return new Handlebars.SafeString(
      `<p style="padding-block: ` + (height ? height : 0) + `rem;"></p>`
    );
  });

  Handlebars.registerHelper("filepath", function (filepath) {
    return new Handlebars.SafeString(
      `<span class="zscript-filepath">` + filepath + `</span>`
    );
  });

  Handlebars.registerHelper("bold", function (text) {
    return new Handlebars.SafeString(
      `<strong>` + text + `</strong>`
    );
  });

  Handlebars.registerHelper("zname", function (filepath) {
    return new Handlebars.SafeString(
      `<span class="zscript-name-in-text">ZScript</span>`
    );
  });

  Handlebars.registerHelper("nav-item", function (options) {
    const active = (options.data.root.global_page_name == options.hash.label) ? "active" : "";
    return new Handlebars.SafeString(
      `<a class="nav-link ` + active + `" href="` + options.hash.href + `">` + options.hash.label + `</a>`
    );
  });

  Handlebars.registerHelper("inline-code", function (str) {
    return new Handlebars.SafeString(
      `<code class="inline-code">` + str + `</code>`
    );
  });

  Handlebars.registerHelper("zscript-intext-code", function (code) {
    return new Handlebars.SafeString(
      `<code class="zscript-intext-code">` + code + `</code>`
    );
  });

  // Handlebars.registerHelper("zscript-intext-code", function (code) {
  //   return new Handlebars.SafeString(
  //     `<code class="zscript-intext-code hljs language-zscript">` +
  //       hljs.highlight(code, { language: 'zscript' }).value + `</code>`
  //   );
  // });

  Handlebars.registerHelper("code-block", function (options) {
    const code = hljs.highlight(options.fn(this), { language: 'zscript' }).value;
    const figure_index = (1 + options.data.root.figure_count++);
    const figure_label = (options.hash.label ? options.hash.label : "");
    const figure_caption = `<strong>Figure ` + figure_index + `&nbsp;</strong>` + figure_label;

    return new Handlebars.SafeString(
      `<figure>
        <pre><code class="zscript-code hljs language-cpp">` + code + `</code></pre>
        <figcaption class="blockquote-footer text-center">` + figure_caption + `</figcaption>
      </figure>`
    );
  });


  Handlebars.registerHelper("cpp-code-block", function (options) {
    const code = hljs.highlight(options.fn(this), { language: 'cpp' }).value;
    const figure_index = (1 + options.data.root.figure_count++);
    const figure_label = (options.hash.label ? options.hash.label : "");
    const figure_caption = `<strong>Figure ` + figure_index + `&nbsp;</strong>` + figure_label;

    return new Handlebars.SafeString(
      `<figure>
        <pre><code class="hljs language-cpp">` + code + `</code></pre>
        <figcaption class="blockquote-footer text-center">` + figure_caption + `</figcaption>
      </figure>`
    );
  });

  Handlebars.registerHelper("error-code-block", function (options) {
    const code = hljs.highlight(options.fn(this), { language: 'zscript' }).value;
    const figure_index = (1 + options.data.root.figure_count++);
    const figure_label = (options.hash.label ? options.hash.label : "");
    const figure_caption = `<strong>Figure ` + figure_index + `&nbsp;</strong>` + figure_label;

    return new Handlebars.SafeString(
      `<figure>
        <pre class="zscript-error-code"><code class="zscript-code hljs language-zscript">` + code + `</code></pre>
        <figcaption class="blockquote-footer text-center">` + figure_caption + `</figcaption>
      </figure>`
    );
  });

  Handlebars.registerHelper("terminal-block", function (options) {
    const code = hljs.highlight(options.fn(this), { language: 'shell' }).value;

    return new Handlebars.SafeString(
      `<div class="terminal " role="alert">
      <div class=fakeMenu>
        <div class="fakeButtons fakeClose"></div>
        <div class="fakeButtons fakeMinimize"></div>
        <div class="fakeButtons fakeZoom"></div>
        <div class="fakeTitle">Output` + (options.hash.add_figure_count ? ` <span class="terminal-output-figure">&nbsp;[Figure ` + options.data.root.figure_count + `]</span>` : "") + `</div>
      </div>
      <div class="fakeScreen">
        <pre><code class="terminal-code hljs language-shell">` + code + `</code></pre>
      </div>
    </div>`
    );
  });

  Handlebars.registerHelper("code-file", function (options) {
    const code = hljs.highlight(options.fn(this), { language: 'zscript' }).value;

    return new Handlebars.SafeString(
      `<div class="card">
        <div class="card-header example-filepath-header">` + options.hash.filename + `</div>
        <div class="card-body">
          <pre class="file-example"><code class="zscript language-zscript zscript-code">` + code + `</code></pre>
        </div>
      </div>`
    );
  });

  Handlebars.registerHelper("content-container", function (options) {
    return new Handlebars.SafeString(
      `<div id="content-container" class="container">
      <div class="row">
        <div id="content" class="mx-auto col-10">
          ` + options.fn(this) + `
        </div>
      </div>
    </div>`
    );
  });

  Handlebars.registerHelper("page-section", function (options) {
    return new Handlebars.SafeString(
      `<section class="zscript-top-section">
        <h1 class="zscript-page-title"><span class="zscript-page-title-num">` + options.data.root.page_info.num + `&nbsp;</span>` + options.data.root.page_info.label + `</h1>
          ` + options.fn(this) + `
      </section>`
    );
  });


  Handlebars.registerHelper("sidebar-item", function (options) {
    const selected_item_class = options.data.root.page_name == options.hash.name ? "zscript-sidebar-selected-item" : "";

    var content = "";

    if (options.fn) {
      content = options.fn(this);
    }

    return new Handlebars.SafeString(
      `<li>
      <a id="side-menu-` + options.hash.name + `" href="` + options.hash.name + `.html"
        class="zscript-sidebar-item ` + selected_item_class + ` d-inline-flex">
          <span class="zscript-sidebar-item-num">` + options.data.root.site_info[options.hash.name].num + `&nbsp;</span>
          <span>` + options.data.root.site_info[options.hash.name].label + `</span>
      </a>
      ` + content + `
    </li>`
    );
  });

  Handlebars.registerHelper("sidebar-section", function (options) {
    const selected_item_class = options.data.root.page_section == options.hash.name ? "zscript-sidebar-section-selected" : "";

    return new Handlebars.SafeString(
      `<button id="` + options.hash.name + `-collapse-button"
        class="btn zscript-sidebar-section-button ` + selected_item_class + ` d-inline-flex align-items-center rounded border-0 collapsed"
        data-bs-toggle="collapse" data-bs-target="#` + options.hash.name + `-collapse" aria-expanded="true">
        ` + options.hash.label + `
      </button>`
    );
  });
}

