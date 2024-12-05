#include <catch2.h>
#include <zapp.h>
#include <zbase/utility/print.h>

TEST_CASE("Equation") {
  zapp_options options;
  options.filepath = "/Users/alexarse/Develop/zscript/examples/main_01.zs";
  options.main_function = "main";
  options.include_directories = { nullptr, 0 };
  options.defines = { nullptr, 0 };
  options.args = { nullptr, 0 };

  int ret = zapp_call_main(&options);
  REQUIRE(ret == 0);
}
