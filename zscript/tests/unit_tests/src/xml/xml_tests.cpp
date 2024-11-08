
#include <ztests/ztests.h>
#include <zscript/zscript.h>
#include <zbase/utility/print.h>
#include <zbase/strings/parse_utils.h>

#include "xml/zxml_parser.h"
#include <fstream>

TEST_CASE("zs::xml.122321312") {
  std::string_view s = "Alex";

  constexpr uint64_t k_type_shift_mask = 0x00FF000000000000;
  constexpr uint64_t k_type_shift_offset = 48;

  zs::object ss = zs::object::create_small_string("Bingo");
  zs::object iobj = 32;
  zs::object_base obj;
  obj._int = 32;
  obj._ext = (k_type_shift_mask) & (((uint64_t)zs::object_type::k_integer) << k_type_shift_offset);

  constexpr zs::object ddsd = zs::_sv("john");
  REQUIRE(iobj._ext == obj._ext);
}

TEST_CASE("zs::xml.01") {

  zs::engine eng;

  {
    zs::xml_parser parser(&eng);

    zs::object output;
    REQUIRE(!parser.parse(nullptr, R"""(
<doc class="bingo" env="123" john=67>
  <bono>
    Banana
    <binfo>
      Pico
      <div class="hidden">Yes!</div>
    </binfo>
  </bono>
</doc>
)""",
        output));

    REQUIRE(output.is_node());

    //    zb::print(output.as_node());
    //    zb::print(parser.get_error());
  }
}

#include <zbase/sys/path.h>

TEST_CASE("zs::xml.02") {

  zs::engine eng;

  zs::xml_parser parser(&eng);

  zs::object node;
  REQUIRE(!parser.parse(nullptr, R"""(
<A class="bingo" env="123" john=67>
  <B>
    abc
    <C>
       def
      <E class="hidden">hij</E>
    </C>
    <D>John</D>
  </B>
</A>
)""",
      node));

  zs::var iii = 21;
  zs::int_t aaaaa = iii.get();
  zs::float_t aaaaa2 = iii.get();
  float aaaaa3 = iii.get();

  REQUIRE(node.is_node());
  const zs::node_object& A = node.get();
  REQUIRE(A.size() == 1);
  REQUIRE(A[0].is_node());

  const zs::node_object& B = A[0].get();

  REQUIRE(B.value().is_string());
  std::string_view ss = B.value().get();
  //  std::string_view sss = B.value();

  zs::var arr = zs::_a(&eng, { 1, 2, 3 });
  zs::array_object& aobj = arr.get();

  REQUIRE(zb::strip_all(B.value().get_string_unchecked()) == "abc");

  REQUIRE(B.size() == 2);
  REQUIRE(B[0].is_node());

  const zs::node_object& C = B[0].get();

  REQUIRE(C.value().is_string());
  REQUIRE(zb::strip_all(C.value().get_string_unchecked()) == "def");

  REQUIRE(C.size() == 1);
  REQUIRE(C[0].is_node());

  const zs::node_object& E = C[0].get();
  REQUIRE(E.name() == "E");
  REQUIRE(E.value() == "hij");

  //  REQUIRE(C.as_node()[0].as_node().value()  () == "E");
  //  REQUIRE(zb::strip_all(C.as_node()[0].as_node().value().get_string_unchecked()) == "hij");
}

// TEST_CASE("zs::xml.02") {
//
//   zs::engine eng;
//
//   {
//     zs::xml_parser parser(&eng);
//     zs::object tbl = zs::_t(&eng,
//                             { { zs::_ss("alex"), zs::_ss("binfo") },{ zs::_ss("Johnny"), zs::_ss("binfo")
//                             },{ zs::_ss("C"), zs::_ss("env") },
//             { zs::_ss("A"), zs::_t(&eng, { { zs::_ss("B"), zs::_ss("div") } }) } });
//     zs::object output;
//     CHECK(!parser.parse(nullptr, R"""(
//<doc class="bingo" {{C}}="123" john=67>
//   <bono>
//     Banana
//     <a></a>jkljkjl<{{alex}}>
//       Pico
//       <{{A.B}} class="hidden">Yes!</div>
//     </binfo>
//
//     <div class="yes">jjj</div>
//   </bono>
//</doc>
//)""",
//         tbl, output));
//
//     //    REQUIRE(output.is_node());
//     //
//     zb::print(output.as_node());
//         zb::print(parser.get_error());
//
//     REQUIRE(output.as_node().children()[0].as_node().name() == "bono");
//     REQUIRE(output.as_node().children()[0].as_node().children()[1].as_node().name() == "a");
//     zb::print(output.as_node().children()[0].as_node().children()[2].get_type() );
//   }
//// }
// TEST_CASE("zs::xml.02") {
//
//   zs::engine eng;
//
//   {
//     zs::xml_parser parser(&eng);
//     zs::object tbl = zs::_t(&eng,
//         { { zs::_ss("alex"), zs::_ss("binfo") }, { zs::_ss("Johnny"), zs::_ss("binfo") },
//             { zs::_ss("C"), zs::_ss("env") },
//             { zs::_ss("A"), zs::_t(&eng, { { zs::_ss("B"), zs::_ss("div") } }) } });
//     zs::object output;
//     CHECK(!parser.parse(nullptr, R"""(
//<doc>
//   <A>
//     Banana
//
//<B>snv</B>
//
// Binfo
//     </A>
//</doc>
//)""",
//         tbl, output));
//
//     //    REQUIRE(output.is_node());
//     //
//     zb::print(output.as_node());
//     zb::print(output.as_node().children()[0].as_node().size());
//     zb::print(parser.get_error());
//     //
//     //    REQUIRE(output.as_node().children()[0].as_node().name() == "bono");
//     //    REQUIRE(output.as_node().children()[0].as_node().children()[1].as_node().name() == "a");
//     //    zb::print(output.as_node().children()[0].as_node().children()[2].get_type() );
//   }
// }

TEST_CASE("zs::xml.04") {

  zs::engine eng;

  {
    zs::xml_parser parser(&eng);
    zs::object tbl = zs::_t(&eng,
        { { zs::_ss("alex"), zs::_ss("binfo") }, { zs::_ss("Johnny"), zs::_ss("binfo") },
            { zs::_ss("C"), zs::_ss("env") },
            { zs::_ss("A"), zs::_t(&eng, { { zs::_ss("B"), zs::_ss("div") } }) } });
    zs::object output;
    REQUIRE(!parser.parse(nullptr, R"""(
<doc>
  <A>asdd jklj 89<B>bn hhj<C>54.f g6.r5.856.kjas</C></B>dsasdaass hjuyu<K>kkk</K>uio ui</A>
</doc>
)""",
        tbl, output));

    REQUIRE(output.is_node());
    //
    //    zb::print(output.as_node());
    //    REQUIRE(output.as_node().children()[0].as_node().size() == 5);
    //    zb::print(parser.get_error());
    //
    //    REQUIRE(output.as_node().children()[0].as_node().name() == "bono");
    //    REQUIRE(output.as_node().children()[0].as_node().children()[1].as_node().name() == "a");
    //    zb::print(output.as_node().children()[0].as_node().children()[2].get_type() );
  }
}

TEST_CASE("zs::xml.05") {

  zs::engine eng;

  {
    zs::xml_parser parser(&eng);

    zs::object peter = zs::object::create_node(&eng, "peter");
    peter.as_node().attributes().emplace_back(zs::_ss("class"), zs::_ss("ninja"));
    peter.as_node().children().push_back(zs::_ss("Full Ninja"));

    zs::object john = zs::object::create_node(&eng, "John");
    john.as_node().children().push_back(peter);
    john.as_node().attributes().emplace_back(zs::_ss("class"), zs::_ss("none"));

    zs::object tbl = zs::_t(&eng,
        {
            { zs::_ss("MyNodeName"), zs::_ss("A") }, //
            { zs::_s(&eng, "MyAttributeName"), zs::_ss("type") }, //
            { zs::_ss("MyNode"), john }, //
            {
                zs::_ss("A"),
                zs::_t(&eng,
                    {
                        { zs::_ss("B"), zs::_ss("div") } //
                    }) //
            } //
        });

    zs::object output;
    REQUIRE(!parser.parse(nullptr, R"""(
<doc>
  <{{MyNodeName}} class="bingo" {{MyAttributeName}}="&lt;full">
    &lt;asdd jklj 89
    <B>bn hhj
      <C>54.f g6.r5.856.kjas</C>
    </B>
    dsasdaass hjuyu
    <K>{{MyNode}}</K>
    uio ui
  </{{MyNodeName}}>
</doc>
)""",
        tbl, output));

    REQUIRE(output.is_node());
    //
    //        zb::print(output.as_node());
    //    REQUIRE(output.as_node().children()[0].as_node().size() == 5);
    //    zb::print(parser.get_error());
    //
    //    REQUIRE(output.as_node().children()[0].as_node().name() == "bono");
    //    REQUIRE(output.as_node().children()[0].as_node().children()[1].as_node().name() == "a");
    //    zb::print(output.as_node().children()[0].as_node().children()[2].get_type() );
  }
}

TEST_CASE("zs::xml.06") {
  zs::engine eng;

  {
    zs::xml_parser parser(&eng);

    zs::object output;
    auto err = parser.parse(nullptr, R"""(
<root id="0">
  <variable_declaration id="2" name="s" type="var" cpp-type="zs::var" line="2" col="4">
    var s = """sakjsakjsl
    <string_value id="3">sakjsakjsl
saskljskal
sakjsla</string_value>
  </variable_declaration>
  <variable_declaration id="5" name="a" type="var" cpp-type="zs::var" line="4" col="4">
    var a = {
    <table_declaration id="6">
      <new_table_field id="13">
        <string_value id="7" role="key">a</string_value>
        <op_add id="12" role="value">
          <op_add id="10" role="lhs">
            <string_value id="8" role="lhs">Alex</string_value>
            <string_value id="9" role="rhs"> </string_value>
          </op_add>
          <string_value id="11" role="rhs">John</string_value>
        </op_add>
      </new_table_field>
      <new_table_field id="25">
        <string_value id="14" role="key">b</string_value>
        <table_declaration id="15" role="value">
          <new_table_field id="18">
            <string_value id="16" role="key">j</string_value>
            <integer_value id="17" role="value">90</integer_value>
          </new_table_field>
          <new_table_field id="24">
            <string_value id="19" role="key">k</string_value>
            <table_declaration id="20" role="value">
              <new_table_field id="23">
                <string_value id="21" role="key">l</string_value>
                <integer_value id="22" role="value">9</integer_value>
              </new_table_field>
            </table_declaration>
          </new_table_field>
        </table_declaration>
      </new_table_field>
    </table_declaration>
  </variable_declaration>
  <variable_declaration id="27" name="b" type="var" cpp-type="zs::var" line="9" col="4">
    var b = a["b"];
    <dot id="30">
      <identifier id="28" role="object">a</identifier>
      <string_value id="29" role="key">b</string_value>
    </dot>
  </variable_declaration>
  <set id="35" line="10" col="2">
    a.c = 89;
    <identifier id="31" role="object">a</identifier>
    <string_value id="33" role="key">c</string_value>
    <integer_value id="34" role="value">89</integer_value>
  </set>
  <set id="42" line="11" col="2">
    a.b.j = 21;
    <dot id="37" role="object">
      <identifier id="36" role="object">a</identifier>
      <string_value id="38" role="key">b</string_value>
    </dot>
    <string_value id="40" role="key">j</string_value>
    <integer_value id="41" role="value">21</integer_value>
  </set>
  <set id="47" line="12" col="2">
    a["c"] = 78;
    <identifier id="43" role="object">a</identifier>
    <string_value id="44" role="key">c</string_value>
    <integer_value id="46" role="value">78</integer_value>
  </set>
  <variable_declaration id="49" name="val" type="var" cpp-type="zs::var" line="14" col="4">
    var val = a.b.j;
    <dot id="53">
      <dot id="51" role="object">
        <identifier id="50" role="object">a</identifier>
        <string_value id="52" role="key">b</string_value>
      </dot>
      <string_value id="54" role="key">j</string_value>
    </dot>
  </variable_declaration>
  <set id="63" line="16" col="2">
    a.b.k.l = "bob";
    <dot id="58" role="object">
      <dot id="56" role="object">
        <identifier id="55" role="object">a</identifier>
        <string_value id="57" role="key">b</string_value>
      </dot>
      <string_value id="59" role="key">k</string_value>
    </dot>
    <string_value id="61" role="key">l</string_value>
    <string_value id="62" role="value">bob</string_value>
  </set>
  <variable_declaration id="65" name="g" type="var" cpp-type="zs::var" line="18" col="4">
    var&lt;int&gt; g = 89;
    <integer_value id="66">89</integer_value>
  </variable_declaration>
  <if_statement id="70" line="19" col="3">
    if(g - 89) {
    <op_sub id="69" role="condition">
      <identifier id="67" role="lhs">g</identifier>
      <integer_value id="68" role="rhs">89</integer_value>
    </op_sub>
    <if_block id="71">
      <assignment id="74" line="20" col="4">
        g = 90;
        <identifier id="72" role="lhs">g</identifier>
        <integer_value id="73" role="rhs">90</integer_value>
      </assignment>
      <return_statement id="76">
        <integer_value id="75">897</integer_value>
      </return_statement>
    </if_block>
  </if_statement>
  <return_statement id="78">
    <string_value id="77">John</string_value>
  </return_statement>
</root>
)""",
        output);

    //    REQUIRE(output.is_node());

    //    zb::print(parser.get_error());
    //        zb::print(output.as_node());
    //    zb::print(zs::serializer(zs::serializer_type::plain, output));

    //    std::ofstream file;
    //    file.open("/Users/alexarse/Develop/zscript/build-xcode/apps/compiler_test/generated/abc.xml");
    //    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    //    file << zs::serializer(zs::serializer_type::plain, output);
    //    file.close();

    //    const zs::object& obj = output.as_node().children()[0];
    //    zb::print(obj.as_node().value());
  }
}
