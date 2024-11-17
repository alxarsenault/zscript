#include "unit_tests.h"

ZS_CODE_TEST("graphics.color.01", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb(0xCC00FFAA);
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xcc, 0x00, 0xff, 0xaa }));
}

ZS_CODE_TEST("graphics.color.02", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb(0xCC, 0x00, 0xFF, 0xAA);
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xcc, 0x00, 0xff, 0xaa }));
}

ZS_CODE_TEST("graphics.color.03", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb([0xCC, 0x00, 0xFF, 0xAA]);
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xcc, 0x00, 0xff, 0xaa }));
}

ZS_CODE_TEST("graphics.color.04", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb([0xCC, 0x00, 0xFF]);
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xcc, 0x00, 0xff, 0xff }));
}

ZS_CODE_TEST("graphics.color.05", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb([1.0, 0.0, 0.0]);
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0x00, 0x00, 0xff }));
}

ZS_CODE_TEST("graphics.color.06", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb();
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0x0, 0x00, 0x00, 0x0 }));
}

ZS_CODE_TEST("graphics.color.07", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("#ff6f5544");
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0x6f, 0x55, 0x44 }));
}

ZS_CODE_TEST("graphics.color.08", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("#ff6f55");
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0x6f, 0x55, 0xff }));
}

ZS_CODE_TEST("graphics.color.09", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return [c.red(), c.green(), c.blue(), c.alpha()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0xff, 0xff, 0xff }));
}

ZS_CODE_TEST("graphics.color.10", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return [c.redf(), c.greenf(), c.bluef(), c.alphaf()];
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 1.0, 1.0, 1.0, 1.0 }));
}

ZS_CODE_TEST("graphics.color.11", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return c.components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0xff, 0xff, 0xff }));
}

ZS_CODE_TEST("graphics.color.12", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return c.fcomponents();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 1.0, 1.0, 1.0, 1.0 }));
}

ZS_CODE_TEST("graphics.color.13", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return c.with_red(0xA2).components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xA2, 0xff, 0xff, 0xff }));
}

ZS_CODE_TEST("graphics.color.14", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return c.with_green(0xA2).components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0xA2, 0xff, 0xff }));
}

ZS_CODE_TEST("graphics.color.15", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return c.with_blue(0xA2).components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0xff, 0xA2, 0xff }));
}

ZS_CODE_TEST("graphics.color.16", R"""(
var graphics = import("graphics");
var c = graphics.color.rgb("white");
return c.with_alpha(0xA2).components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0xff, 0xff, 0xff, 0xA2 }));
}

ZS_CODE_TEST("graphics.color.17", R"""(
var graphics = import("graphics");
var c = graphics.color.hsv(240, 100, 100);
return c.components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 0, 0, 0xff, 0xff }));
}

ZS_CODE_TEST("graphics.color.18", R"""(
var graphics = import("graphics");
var c = graphics.color.hsv(283, 95, 77);
return c.components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 143, 9, 196, 0xff }));
}

ZS_CODE_TEST("graphics.color.19", R"""(
var graphics = import("graphics");
var c = graphics.color.hsv(360, 86, 64);
return c.components();
)""") {
  REQUIRE(value == zs::_a(vm.get_engine(), { 163, 22, 22, 0xff }));
}
