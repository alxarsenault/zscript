
#include "unit_tests.h"
#include <zscript.h>
#include "bytecode/zinstruction.h"
#include "bytecode/zinstruction_vector.h"
#include <zscript/object_stack.h>

#include "zvirtual_machine.h"
#include <zbase/utility/print.h>

TEST_CASE("zs::instruction_vector") {
  zs::vm vm;
  zs::engine* eng = vm->get_engine();

  zs::instruction_vector instructions(eng);
  instructions.push<zs::opcode::op_add>((uint8_t)3, (uint8_t)1, (uint8_t)2);
  instructions.push<zs::opcode::op_load>((uint8_t)3, (uint32_t)200);
  instructions.push<zs::opcode::op_sub>((uint8_t)3, (uint8_t)1, (uint8_t)2);

  size_t index = 0;
  std::vector<zs::opcode> ops = { zs::opcode::op_add, zs::opcode::op_load, zs::opcode::op_sub };
  for (auto it = instructions.begin(); it != instructions.end(); ++it) {
    REQUIRE(it.get_opcode() == ops[index]);

    index++;
  }

  zs::instruction_stream ss(instructions);

  REQUIRE(ss.get_opcode() == zs::opcode::op_add);
  REQUIRE(ss.get<zs::opcode::op_add>()->target_idx == 3);
  REQUIRE(ss.get<zs::opcode::op_add>()->lhs_idx == 1);
  REQUIRE(ss.get<zs::opcode::op_add>()->rhs_idx == 2);

  ++ss;
  REQUIRE(ss.get_opcode() == zs::opcode::op_load);
  REQUIRE(ss.get<zs::opcode::op_load>()->target_idx == 3);
  REQUIRE(ss.get<zs::opcode::op_load>()->idx == 200);

  REQUIRE(ss.next() == zs::opcode::op_sub);
  REQUIRE(ss.get<zs::opcode::op_sub>()->target_idx == 3);
  REQUIRE(ss.get<zs::opcode::op_sub>()->lhs_idx == 1);
  REQUIRE(ss.get<zs::opcode::op_sub>()->rhs_idx == 2);
}

TEST_CASE("zs::instruction") {
  zs::virtual_machine* vmachine = zs::create_virtual_machine(255);
  {
    zs::virtual_machine& vm = *vmachine;
    vm.push(10);
    vm.push(20);
    vm.push(nullptr);

    REQUIRE(vm.stack_size() == 3);

    REQUIRE(vm.stack_get(0).is_integer());
    REQUIRE(vm[0] == 10);
    //
    REQUIRE(vm[1].is_integer());
    REQUIRE(vm[1] == 20);

    // Add 1 with 2 put it in 3.
    zs::instruction_t<zs::opcode::op_add> add_op{ zs::opcode::op_add, 2, 0, 1 };

    REQUIRE(!vm.add(vm[add_op.target_idx], vm[add_op.lhs_idx], vm[add_op.rhs_idx]));
    REQUIRE(vm[2].is_integer());
    REQUIRE(vm[2] == 30);

    vm[1] = 5.5;
    //    zb::print("DSDJSKJKLDS", vm[add_op.lhs_idx], vm[add_op.rhs_idx]);
    REQUIRE(!vm.add(vm[add_op.target_idx], vm[add_op.lhs_idx], vm[add_op.rhs_idx]));
    REQUIRE(vm[2].is_float());
    REQUIRE(vm[2] == 15.5);

    // Sub.
    zs::instruction_t<zs::opcode::op_sub> sub_op{ zs::opcode::op_sub, 2, 0, 1 };

    REQUIRE(!vm.sub(vm[sub_op.target_idx], vm[sub_op.lhs_idx], vm[sub_op.rhs_idx]));
    REQUIRE(vm[2].is_float());
    REQUIRE(vm[2] == 4.5);

    //    vm[0] = zs::object_ptr::create_small_string("A");
    //    vm[1] = zs::object_ptr::create_small_string("B");
    //    REQUIRE(!vm.add(vm[add_op.target_idx], vm[add_op.lhs_idx],
    //    vm[add_op.rhs_idx])); REQUIRE(vm[2].is_string()); REQUIRE(vm[2] ==
    //    "AB");
  }

  zs::close_virtual_machine(vmachine);
}
