#include "ztests.h"

ZS_CODE_TEST("base64.encode.01", R"""(
var base64 = import("base64");
var a = base64.encode("john");
return a;
)""") {
  REQUIRE(value == "am9obg==");
}

ZS_CODE_TEST("base64.decode.01", R"""(
var base64 = import("base64");
var a = base64.decode("am9obg==");
return a;
)""") {
  REQUIRE(value == "john");
}
