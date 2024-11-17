#include "specification/specification.h"

ZS_SPEC_SECTION(compare, "Compare", "hjhkj");

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a == 32) {
  a = 33;
}
)"""))
{
  REQUIRE(value == 33);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a == 35) {
  a = 33;
}
)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a < 35) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a < 3) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a > 3) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a > 35) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a <= 35) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a == 32.0) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a === 32.0) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a === 32) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 0;

if(a) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 0);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 0;

if(!a) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a <==> 32) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a <==> 32.0) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a <--> 32) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 32;

if(a <--> 32.0) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = "bacon";

if(a == "bacon") {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}
ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = "bacon";

if(a == "ham") {
  a = 21;
}
)"""))
{
  REQUIRE(value == "bacon");
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = "bacon";

if(a != "ham") {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = "bacon";

if(a) {
  a = 21;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 2;

if(a == 2) {
  a = 21;
}
else {
 a = 23;
}
)"""))
{
  REQUIRE(value == 21);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
var a = 2;

if(a == 32) {
  a = 21;
}
else {
 a = 23;
}
)"""))
{
  REQUIRE(value == 23);
}
//
// ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
// var a = 2;
//
// if(a == 1) {
//  a = 88;
//}
// else if(a == 2) {
//  a = 99;
//}
// else {
// a = 23;
//}
//)"""))
//{
//  REQUIRE(value == 99);
//}
//
// ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
// var a = 2;
//
// if(a == 1) {
//  a = 88;
//}
// else if(a == 3) {
//  a = 99;
//}
// else {
// a = 23;
//}
//)"""))
//{
//  REQUIRE(value == 23);
//}
//
// ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
// var a = 2;
//
// if(a == 1) {
//  a = 88;
//}
// else if(a == 3) {
//  a = 99;
//}
//)"""))
//{
//  REQUIRE(value == 2);
//}

ZS_SPEC_TEST(compare, "If Compare", "jgghhjghjghjgjh", ZCODE_A(R"""(
var a = 2;

if(a == 1) {
  a = 88;
}
else if(a == 3) {
  a = 99;
}
else {
}
)"""))
{
  REQUIRE(value == 2);
}

ZS_SPEC_TEST(compare, "Compare", "", ZCODE_A(R"""(
 var a = "bacon";

 if(!a) {
  a = 21;
}
)"""))
{
  REQUIRE(value == "bacon");
}

// TEST_CASE("DSLKDJSKJDLKSD")
//{
//   //  uint64_t seed_in = 23;
//   //  uint64_t seed = zb::hash_detail::wyrand(&seed_in);
//   std::array<uint64_t, 4> arr = {};
//   zb::hash_detail::make_secret(0, arr.data());
//
////  zb::print(arr);
//  //  zb::print(sizeof(zs::type_info_object));
//}
