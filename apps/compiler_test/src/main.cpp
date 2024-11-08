#include <zscript.h>
#include "zvirtual_machine.h"

#include "objects/zfunction_prototype.h"
#include "lang/zparser.h"

#include <zbase/strings/string_view.h>
#include <zbase/container/span.h>
#include "cpp_compiler.h"
#include <fstream>
#include <format>

void run(std::string_view code);

inline constexpr std::string_view s_code1 = R"""(
 var a = 32;
 var b = "dsahdjsahdjksahfjahsfjkhfjkdhfdjkshfjkdshfjkdshfjkdshf";
 var c = 32 + a + 55 + 89.56;
 var h = false;
 var L = true;
 var bingo = { a = "banana" };
 bingo.a = 4523;
 var p = [1, 2.89, 3 + 5];
 var g = {
  "abc": 50,
  john = "peterson",
  [b] = 90,
  sub = {
    a = 1,
    sub_sub = {
      b = 90
    }
  }
};

 if(a == 32) {
  c = a;
  var z = 42;
  b = z;
}
 else if(c == 199) {
  c = 122;
}
 else if(c == 200) {
  c = 596783;
}
 else {
  var q = 55;
  c = q;
}

 var d = a + c;

 if(d == 12) {
  d = 2;
}
 else {
  if(d == 31) {
    d = 323;
  }

  a = 21;
}

 return d + 90;
)""";

inline constexpr std::string_view s_code = R"""(
var<int> g = 89;
var π = 888;
var hhjh = [g, false, (89 * 7) /(g+9)];
float k = 23.923;
var ds = g + k / 232.2;
var eee=null;
var t = {
  a = 32,
  b = "johnny"
};

var arr = [1, 2, 3, 4, "a33"];

var v1 = t.a;
v1 = 32;
v1 += 89;
v1 -= 1;
v1 *= 2;
v1 /= 3;
v1 %= 4;
v1 ^= 5;
 
function bingo() {
}

if(g) {
  g = v1 + 90;
  return 897;
}

  return "John";
)""";

inline constexpr std::string_view s_code2 = R"""(
var s = """sakjsakjsl
saskljskal
sakjsla""";

var a = {
  a = "Alex" + " " + "John",
  b = {j = 90, k= {l = 9}}
};

var b = a["b"];
a.c = 89;
a.b.j = 21;
a["c"] = 78;

var val = a.b.j;

a.b.k.l = "bob";

var<int> g = 89;
if(g - 89) {
  g = 90;
  return 897;
}

  return "John";
)""";

int main(int argc, char** argv) {
  run(s_code2);

  return 0;
}

zs::error_result do_parse(zs::parser& parser, std::string_view code) {

  zs::object output;
  if (zs::error_result err = parser.parse(code, "compiler_test", output)) {
    zb::print("parser error", err.message(), parser.get_error());

    return err;
  }

  return {};
}

void print_parser(zs::parser& parser) {
  zs::engine* eng = parser.get_engine();

  std::ofstream file;
  file.open(ZSCRIPT_COMPILER_TEST_OUTPUT_DIRECTORY "/generated/kkk.xml");
  file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  file << zs::serializer(zs::serializer_type::plain, parser.root());
  file.close();

  //     std::cout << buffer;
  //  zb::print(zs::serializer(zs::serializer_type::plain, parser.root()));
  //  return;;
  //  zb::print(parser.root().as_node());

  zb::print("\nAST Stack:", parser.stack().size());
  zb::print("-------------------------------------------");

  for (size_t i = 0; i < parser.stack().size(); i++) {
    zs::ast_node_walker walker(parser.stack()[i]);
    zs::string f = walker.serialize(eng);
    zb::print(f);
  }

  zb::print("\nAST:");
  zb::print("-------------------------------------------");
  zs::ast_node_walker walker(parser.root());
  zs::string f = walker.serialize(eng);
  zb::print(f);
}

void run(std::string_view code) {
  zs::engine eng;

  zs::object obj;

  {
    zs::parser parser(&eng);

    if (auto err = do_parse(parser, code)) {
      return;
    }

    zs::cpp_compiler cpp_comp(&eng, &parser);

    cpp_comp.generate("my_function", "my_function_name", "my_namespace",
        {
            { zs::_ss("p1"), false, zs::create_type_mask(zs::object_type::k_integer) }, //
            { zs::_ss("p2"), false }, //
            { zs::_ss("p3"), true,
                zs::create_type_mask(zs::object::k_string_mask, zs::object_type::k_bool) }, //
            { zs::_ss("p4"), true } //
        });

    cpp_comp.export_code(ZSCRIPT_COMPILER_TEST_OUTPUT_DIRECTORY "/generated");

    print_parser(parser);
  }

  {
    zs::vm vm(&eng);

    zs::object closure;

    if (auto err = vm->compile_buffer(code, "compiler_test", closure)) {
      zb::print("error", vm->get_error());
      return;
    }

    if (!closure.is_closure()) {
      zb::print("error not a closure");
      return;
    }

    zs::int_t n_params = 1;
    vm.push_root();

    zs::object ret_value;
    if (auto err = vm->call(closure, n_params, vm->stack_size() - n_params, ret_value, true)) {
      zb::print("error", vm->get_error());
      return;
    }

    vm->push(ret_value);

    // Print instructions.
    zb::print("\nInstructions:");
    zb::print("-------------------------------------------");

    {
      zs::function_prototype_object* fct = closure._closure->get_function_prototype();
      zs::instruction_vector& ivec = fct->_instructions;

      for (auto it = ivec.begin(); it != ivec.end(); ++it) {
        switch (it.get_opcode()) {
#define ZS_DECL_OPCODE(name)                        \
  case zs::opcode::op_##name:                       \
    zb::print(it.get_ref<zs::opcode::op_##name>()); \
    break;

#include "lang/zopcode_def.h"
#undef ZS_DECL_OPCODE

        default:
          zb::print(it.get_opcode());
        }
      }

      // Stack size.
      zb::print("\nStack size:", fct->_stack_size);
      zb::print("-------------------------------------------");

      // Local variables.
      zb::print("\nLocal variables:");
      zb::print("-------------------------------------------");
      {
        const size_t sz = fct->_vlocals.size();
        for (size_t i = 0; i < sz; i++) {
          zb::print(i, "pos:", fct->_vlocals[i]._pos, fct->_vlocals[i]._name.to_debug_string());
        }
      }

      // Literals.
      zb::print("\nLiterals:");
      zb::print("-------------------------------------------");

      {
        const size_t sz = fct->_literals.size();
        for (size_t i = 0; i < sz; i++) {
          zb::print(i, fct->_literals[i].to_debug_string());
        }
      }
    }

    zb::print("\nStack:");
    zb::print("-------------------------------------------");
    for (zs::int_t i = 0; i < vm->stack_size(); i++) {
      zb::print("stack", i, ":", vm->stack_get(i).convert_to_string());
    }
  }
}
