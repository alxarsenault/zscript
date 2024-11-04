const s_site_map = {
  sections: [
    {
      name: "Introduction",
      sections: [
        "intro-overview",
        "intro-getting-started",
        "intro-hello-world"
      ]
    },
    {
      name: "Language",
      sections: [
        "language-types",
        "language-statements",
        "language-expressions",
        "language-scopes",
        "language-modules",
        "language-compile-time"
      ]
    }
  ]
};




function InitCollapses(section) {


  $(".collapse").each(function () {
    const collapse = $(this);
    const button = collapse.prev();
    const id = collapse.attr("id");
    const sectionId = section + "-collapse";

    button.attr("data-no-transition", "true");

    collapse.on("show.bs.collapse", function () {
      sessionStorage.setItem(id, "show");
    });

    collapse.on("hide.bs.collapse", function () {
      sessionStorage.removeItem(id);
    });



    if (id == sectionId) {
      button.addClass("zscript-sidebar-section-selected");
      collapse.addClass("show");
      button.attr("aria-expanded", "true");
    }
    else {
      button.removeClass("zscript-sidebar-section-selected");

      if (sessionStorage.getItem(id) == "show") {
        collapse.addClass("show");
        button.attr("aria-expanded", "true");
      }
      else {
        collapse.removeClass("show");
        button.attr("aria-expanded", "false");
      }
    }

    setTimeout(function () {
      button.attr("data-no-transition", "false");
    }, 500);
    // 

  });

}



$(document).ready(function () {
  var pageName = window.location.pathname.split('\\').pop().split('/').pop().replace(/\.[^/.]+$/, "");
  const section = pageName.split("-")[0];
  // console.log(section)
  // pageName = pageName[0].toUpperCase() + pageName.slice(1);
  // console.log(window.location.pathname.split('\\').pop().split('/').pop().replace(/\.[^/.]+$/, ""))
  $("#side-menu-" + pageName).addClass("zscript-sidebar-selected-item");
  InitCollapses(section);


  $(".zscript-code").each(function () { hljs.highlightElement($(this)[0]); });

  $(".terminal-code").each(function () {
    hljs.highlightElement($(this)[0]);
  });


  // console.log(hljs.listLanguages());
  $(".blockquote-footer").each(function (idx) {
    $(this).prepend($("<strong></strong>").html("Figure " + (idx + 1) + " &nbsp;"));
  });


  // InitCollapse("intro-collapse");
  // InitCollapse("language-collapse");
});