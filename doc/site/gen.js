// nodemon -w template/pages/docs -e html gen.js
// nodemon -w template -e html gen.js
const fs = require('fs');
const path = require('path');
const Handlebars = require("handlebars");

function include(filename) {
  require("./" + filename)();
}

include("src/zscript-highlight");
include("src/template-helper");

try {
  if (!fs.existsSync("dist")) {
    fs.mkdirSync("dist");
  }
} catch (err) {
  console.error(err);
}


fs.cp("data", "dist", {recursive: true}, (err) => {/* callback */});


const component_order_pattern = /^\d{2}-.+/;

const components = fs.readdirSync("template/components").filter(file => {
  return path.extname(file) == ".html";
});

function add_common_components(content) {
  components.forEach(component_filepath => {
    let component_name = path.parse(component_filepath).name;

    if(component_order_pattern.test(component_name)) {
      component_name = component_name.substring(3);
    }

    const component_content = fs.readFileSync("template/components/" + component_filepath, "utf-8");
    content[component_name] = Handlebars.compile(component_content)(content);
  });
}

Handlebars.registerPartial({
  meta: fs.readFileSync("template/partials/common/meta.html", "utf-8"),
});

Handlebars.registerPartial({
  head: fs.readFileSync("template/partials/docs/head.html", "utf-8"),
  nav: fs.readFileSync("template/partials/docs/nav.html", "utf-8"),
  menu: fs.readFileSync("template/partials/docs/menu.html", "utf-8"),
  sidebar: fs.readFileSync("template/partials/docs/sidebar.html", "utf-8"),
  "bottom-nav": fs.readFileSync("template/partials/docs/bottom-nav.html", "utf-8"),
  breadcrumb: fs.readFileSync("template/partials/docs/breadcrumb.html", "utf-8"),
  ["load-scripts"]: fs.readFileSync("template/partials/docs/load-scripts.html", "utf-8")
});

var site_map = [
  {
    label: "Introduction",
    name: "intro",
    sections: [
      {
        label: "Overview",
        name: "intro-overview"
      },
      {
        label: "Getting Started",
        name: "intro-getting-started"
      },
      {
        label: "Hello World",
        name: "intro-hello-world"
      },
    ]
  },
  {
    label: "Language",
    name: "language",
    sections: [
      {
        label: "Basics",
        name: "language-basics",
        sections: [
          {
            label: "Comments",
            name: "language-basics-comments"
          },
          {
            label: "Identifiers",
            name: "language-basics-identifiers"
          },
          {
            label: "Types",
            name: "language-basics-types"
          },
          {
            label: "Loop",
            name: "language-basics-loop"
          },
          {
            label: "Arithmetics",
            name: "language-arithmetics"
          }
        ]
      },
      {
        label: "Types",
        name: "language-types",
        sections: [
          {
            label: "Integers",
            name: "language-types-integers"
          },
          {
            label: "Floats",
            name: "language-types-floats"
          },
          {
            label: "Strings",
            name: "language-types-strings"
          },
          {
            label: "Arrays",
            name: "language-types-arrays"
          },
          {
            label: "Tables",
            name: "language-types-tables"
          },
        ]
      },

      {
        label: "Statements",
        name: "language-statements"
      },
      {
        label: "Expressions",
        name: "language-expressions"
      },
      {
        label: "Delegates",
        name: "language-delegates"
      },
      {
        label: "Scopes",
        name: "language-scopes"
      },
      {
        label: "Modules",
        name: "language-modules"
      },
      {
        label: "Compile-Time",
        name: "language-compile-time"
      }
    ]
  },
  {
    label: "API",
    name: "api",
    sections: [
      {
        label: "Objects",
        name: "api-objects"
      },
      {
        label: "Virual Machine",
        name: "api-virtual-machine"
      }
    ]
  },
];

let flat_site_map = {};
let flat_site_array = [];

let snum = 0;
for (let i = 0; i < site_map.length; i++) {
  snum++;
  site_map[i].num = snum.toString() + ".";

  if (site_map[i].name) {
    flat_site_map[site_map[i].name] = site_map[i];
  }
  let subnum = 0;

  for (let k = 0; k < site_map[i].sections.length; k++) {
    subnum++;

    site_map[i].sections[k].parent = site_map[i];
    site_map[i].sections[k].num = site_map[i].num + subnum.toString() + ".";

    flat_site_map[site_map[i].sections[k].name] = site_map[i].sections[k];
    flat_site_array.push(site_map[i].sections[k]);
    if (site_map[i].sections[k].sections) {
      let subsubnum = 0;

      for (let n = 0; n < site_map[i].sections[k].sections.length; n++) {
        subsubnum++;
        site_map[i].sections[k].sections[n].parent = site_map[i].sections[k];
        site_map[i].sections[k].sections[n].num = site_map[i].sections[k].num + subsubnum.toString() + ".";
        flat_site_map[site_map[i].sections[k].sections[n].name] = site_map[i].sections[k].sections[n];
        flat_site_array.push(site_map[i].sections[k].sections[n]);
      }
    }
  }
}

// console.log(JSON.stringify(flat_site_map, null, " "))

// const pages = fs.readdirSync("template/pages");

for (let i = 0; i < flat_site_array.length; i++) {
  let page = flat_site_array[i];
  const page_name = page.name;
  const page_section = page_name.split("-")[0];
  const previous_page_name = i > 0 ? flat_site_array[i - 1].name : "#";
  const previous_page_class = i > 0 ? "" : "disabled";
  const next_page_name = i < flat_site_array.length - 1 ? flat_site_array[i + 1].name : "#";
  const next_page_class = i < flat_site_array.length - 1 ? "" : "disabled";
  const page_info = flat_site_map[page_name];

  var content = {
    global_page_name: "Documentation",
    figure_count: 0,
    page_name: page_name,
    page_section: page_section,
    previous_page_name: previous_page_name,
    next_page_name: next_page_name,
    previous_page_class: previous_page_class,
    next_page_class: next_page_class,
    site_info: flat_site_map,
    page_info: page_info,
    page_title: page_info.label
  };

  add_common_components(content);
  // components.add_doc_components(content);

  var page_content = fs.readFileSync("template/pages/docs/" + page.name + ".html", "utf-8");
  const result = Handlebars.compile(page_content)(content);
  console.log("dist/" + page.name + ".html");
  fs.writeFileSync("dist/" + page.name + ".html", result);
}


{
  console.log("dist/index.html");

  // Handlebars.unregisterPartial("head");
  // Handlebars.unregisterPartial("nav");

  // Handlebars.registerPartial("head", fs.readFileSync("template/partials/home/head.html", "utf-8"));
  // Handlebars.registerPartial('nav', fs.readFileSync("template/partials/home/nav.html", "utf-8"));

  Handlebars.registerPartial({
    head: fs.readFileSync("template/partials/home/head.html", "utf-8"),
    nav: fs.readFileSync("template/partials/home/nav.html", "utf-8"),
    ["load-scripts"]: fs.readFileSync("template/partials/home/load-scripts.html", "utf-8")
  });

  const page_name = "index";
  var content = {
    global_page_name: "Home",
    figure_count: 0,
    page_name: page_name,
    page_title: "ZScript"
  };

  add_common_components(content);

  fs.writeFileSync("dist/index.html",
    Handlebars.compile(fs.readFileSync("template/pages/index.html", "utf-8"))(content));
}

{
  console.log("dist/install.html");

  // Handlebars.unregisterPartial("head");
  // Handlebars.unregisterPartial("nav");

  // Handlebars.registerPartial("head", fs.readFileSync("template/partials/home/head.html", "utf-8"));
  // Handlebars.registerPartial('nav', fs.readFileSync("template/partials/home/nav.html", "utf-8"));

  Handlebars.registerPartial({
    head: fs.readFileSync("template/partials/home/head.html", "utf-8"),
    nav: fs.readFileSync("template/partials/home/nav.html", "utf-8"),
    ["load-scripts"]: fs.readFileSync("template/partials/home/load-scripts.html", "utf-8")
  });

  const page_name = "install";
  var content = {
    global_page_name: "Install",
    figure_count: 0,
    page_name: page_name,
    page_title: "ZScript"
  };

  add_common_components(content);

  fs.writeFileSync("dist/install.html",
    Handlebars.compile(fs.readFileSync("template/pages/install.html", "utf-8"))(content));
}


{
  console.log("dist/examples.html");

  // Handlebars.unregisterPartial("head");
  // Handlebars.unregisterPartial("nav");

  // Handlebars.registerPartial("head", fs.readFileSync("template/partials/home/head.html", "utf-8"));
  // Handlebars.registerPartial('nav', fs.readFileSync("template/partials/home/nav.html", "utf-8"));

  Handlebars.registerPartial({
    head: fs.readFileSync("template/partials/home/head.html", "utf-8"),
    nav: fs.readFileSync("template/partials/home/nav.html", "utf-8"),
    ["load-scripts"]: fs.readFileSync("template/partials/home/load-scripts.html", "utf-8")
  });

  const page_name = "examples";
  var content = {
    global_page_name: "Examples",
    figure_count: 0,
    page_name: page_name,
    page_title: "ZScript"
  };

  add_common_components(content);

  fs.writeFileSync("dist/examples.html",
    Handlebars.compile(fs.readFileSync("template/pages/examples.html", "utf-8"))(content));
}


{
  console.log("dist/development.html");

  // Handlebars.unregisterPartial("head");
  // Handlebars.unregisterPartial("nav");

  // Handlebars.registerPartial("head", fs.readFileSync("template/partials/home/head.html", "utf-8"));
  // Handlebars.registerPartial('nav', fs.readFileSync("template/partials/home/nav.html", "utf-8"));

  Handlebars.registerPartial({
    head: fs.readFileSync("template/partials/home/head.html", "utf-8"),
    nav: fs.readFileSync("template/partials/home/nav.html", "utf-8"),
    ["load-scripts"]: fs.readFileSync("template/partials/home/load-scripts.html", "utf-8")
  });

  const page_name = "development";
  var content = {
    global_page_name: "Development",
    figure_count: 0,
    page_name: page_name,
    page_title: "ZScript"
  };

  add_common_components(content);

  fs.writeFileSync("dist/development.html",
    Handlebars.compile(fs.readFileSync("template/pages/development.html", "utf-8"))(content));
}


{
  console.log("dist/contact.html");

  // Handlebars.unregisterPartial("head");
  // Handlebars.unregisterPartial("nav");

  // Handlebars.registerPartial("head", fs.readFileSync("template/partials/home/head.html", "utf-8"));
  // Handlebars.registerPartial('nav', fs.readFileSync("template/partials/home/nav.html", "utf-8"));

  Handlebars.registerPartial({
    head: fs.readFileSync("template/partials/home/head.html", "utf-8"),
    nav: fs.readFileSync("template/partials/home/nav.html", "utf-8"),
    ["load-scripts"]: fs.readFileSync("template/partials/home/load-scripts.html", "utf-8")
  });

  const page_name = "contact";
  var content = {
    global_page_name: "Contact",
    figure_count: 0,
    page_name: page_name,
    page_title: "ZScript"
  };

  add_common_components(content);

  fs.writeFileSync("dist/contact.html",
    Handlebars.compile(fs.readFileSync("template/pages/contact.html", "utf-8"))(content));
}