
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
      collapse.addClass("show");
      button.attr("aria-expanded", "true");
    }
    else {
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
  });
}

function InitSidebar() {

  var x = window.matchMedia("(min-width: 992px)");
  x.addEventListener("change", function() {
    if(x.matches) {
      console.log("SHOW");
    }
    else {
      console.log("HIDE");
    }
  });

}

$(document).ready(function () {
  var pageName = window.location.pathname.split('\\').pop().split('/').pop().replace(/\.[^/.]+$/, "");
  const section = pageName.split("-")[0];

  InitCollapses(section);
  InitSidebar();

  // Breakpoint	Class infix	Dimensions
  // X-Small	None	<576px
  // Small	sm	≥576px
  // Medium	md	≥768px
  // Large	lg	≥992px
  // Extra large	xl	≥1200px
  // Extra extra large	xxl	≥1400px

  // $(".zscript-code").each(function () {
  //   hljs.highlightElement($(this)[0]);
  // });

  // $(".terminal-code").each(function () {
  //   hljs.highlightElement($(this)[0]);
  // });

  // $(".blockquote-footer").each(function (idx) {
  //   $(this).prepend($("<strong></strong>").html("Figure " + (idx + 1) + " &nbsp;"));
  // });
 


  // const footer = $("#footer");
  // const doc_height = $(window).height();
  // const footer_height = footer.height();
  // const footer_top = footer.position().top + footer_height;
 

  // if (footer_top < doc_height) {
  //   // console.log(footer_top, doc_height)
  //   footer.css('margin-top', 0 + (doc_height - footer_top) + 'px');
  // }

});