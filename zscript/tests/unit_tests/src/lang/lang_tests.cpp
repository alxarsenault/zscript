#include "ztests.h"

ZS_CODE_TEST("tostring_01", "return tostring(5);") { REQUIRE(vm.top() == "5"); }
ZS_CODE_TEST("tostring_02", "return tostring(false);") { REQUIRE(vm.top() == "false"); }
ZS_CODE_TEST("tostring_03", "return tostring(true);") { REQUIRE(vm.top() == "true"); }
ZS_CODE_TEST("tostring_04", "return tostring(2.0);") { REQUIRE(vm.top() == "2.00"); }
ZS_CODE_TEST("tostring_05", "return tostring(2.2);") { REQUIRE(vm.top() == "2.20"); }
ZS_CODE_TEST("tostring_06", "return tostring('''john''');") { REQUIRE(vm.top() == "john"); }

ZS_CODE_TEST("tostring_07", "return tostring({a = 32});") {
  REQUIRE(vm.top().get_string_unchecked().starts_with("0x"));
}

ZS_CODE_TEST("tostring_08", "return tostring([1, 2, 3]);") {
  REQUIRE(vm.top().get_string_unchecked().starts_with("0x"));
}

ZS_CODE_TEST("toint_01", "return toint(1);") { REQUIRE(vm.top() == 1); }
ZS_CODE_TEST("toint_02", "return toint(5.0);") { REQUIRE(vm.top() == 5); }
ZS_CODE_TEST("toint_03", "return toint(5.1);") { REQUIRE(vm.top() == 5); }
ZS_CODE_TEST("toint_04", "return toint(true);") { REQUIRE(vm.top() == 1); }
ZS_CODE_TEST("toint_05", "return toint('''32''');") { REQUIRE(vm.top() == 32); }
ZS_CODE_TEST("toint_06", "return toint('''32.2''');") { REQUIRE(vm.top() == 32); }

ZS_CODE_TEST("tofloat_01", "return tofloat('''32.2''');") { REQUIRE(vm.top() == 32.2); }

ZS_CODE_TEST("compare_01", R"""(
var i = 1;
return i < 10;
)""") {
  REQUIRE(value == true);
}

ZS_CODE_TEST("compare_02", R"""(
var i = 1;
i = i + 1;
if(i < 10) {
  return true;
}

return false;
)""") {
  REQUIRE(value == true);
}

// inline constexpr bool (*always_true_cond)(int) = [](int) { return true; };
// inline constexpr bool (*always_false_cond)(int) = [](int) { return false; };
//
// void restricted(std::string_view code, char name, int count, int skip, bool (*cond)(int) =
// always_true_cond,
//     bool (*reset)(int) = always_false_cond) {
//
//   std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//   std::string_view comma = "";
//   int vcount = 0;
//   int vskip = 0;
//   for (int i = 0; i < 10; i++) {
//     if (cond(i) and vskip++ >= skip and vcount++ < count) {
//       std::cout << std::exchange(comma, ", ") << i;
//     }
//
//     if (reset(i)) {
//       vcount = 0;
//       vskip = 0;
//     }
//   }
//   std::cout << "]\n\n";
// }
//
// TEST_CASE("restricted.01") {
//
//   char letter = 'A';
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1) {
//     print(i);
//   }
// })",
//       letter++, 1, 0);
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1, 1) {
//     print(i);
//   }
// })",
//       letter++, 1, 1);
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1, 5) {
//     print(i);
//   }
// })",
//       letter++, 1, 5);
//
//   //
//   restricted(R"(var sz = 10;
// for(var i = 0; i < sz; i++) {
//   @restricted(1, sz - 1) {
//     print(i);
//   }
// })",
//       letter++, 1, 9);
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(3, 4) {
//     print(i);
//   }
// })",
//       letter++, 3, 4);
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(inf, 3) {
//     print(i);
//   }
// })",
//       letter++, 1000, 3);
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   if(i > 5) @restricted(1) {
//     print(i);
//   }
// })",
//       letter++, 1, 0, [](int i) { return i > 5; });
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   if(i > 5) @restricted(2, 2) {
//     print(i);
//   }
// })",
//       letter++, 2, 2, [](int i) { return i > 5; });
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(2, 2, i == 5) {
//     print(i);
//   }
// })",
//       letter++, 2, 2, always_true_cond, [](int i) { return i == 5; });
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1, 0, i % 2 == 0) {
//     print(i);
//   }
// })",
//       letter++, 1, 0, always_true_cond, [](int i) { return i % 2 == 0; });
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1, 1, i % 2 == 0) {
//     print(i);
//   }
// })",
//       letter++, 1, 1, always_true_cond, [](int i) { return i % 2 == 0; });
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1, 0, true) {
//     print(i);
//   }
// })",
//       letter++, 1, 0, always_true_cond, always_true_cond);
//
//   //
//   restricted(R"(for(var i = 0; i < 10; i++) {
//   @restricted(1, 1, true) {
//     print(i);
//   }
// })",
//       letter++, 1, 1, always_true_cond, always_true_cond);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//
//     for (int k = 0; k < 3; k++) {
//       int vcount = 0;
//       int vskip = 0;
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (did_output) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//           did_output = true;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted(1) {
//       print(i);
//     }
//   }
// })",
//       letter++, 1, 0);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int vcount = 0;
//     int vskip = 0;
//     for (int k = 0; k < 3; k++) {
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (did_output) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//           did_output = true;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana;
// for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted<banana>(2) {
//       print(i);
//     }
//   }
// })",
//       letter++, 2, 0);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//
//     for (int k = 0; k < 3; k++) {
//
//       for (int i = 0; i < 10; i++) {
//         int vcount = 0;
//         int vskip = 0;
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (did_output) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//           did_output = true;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     @scope banana;
//     @restricted<banana>(1) {
//       print(i);
//     }
//   }
// })",
//       letter++, 1, 0);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//
//     for (int k = 0; k < 3; k++) {
//
//       for (int i = 0; i < 10; i++) {
//         int vcount = 0;
//         int vskip = 0;
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (did_output) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//           did_output = true;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     @scope banana;
//     @restricted<banana>(1, 1) {
//       print(i);
//     }
//   }
// })",
//       letter++, 1, 1);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int vcount = 0;
//     int vskip = 0;
//     for (int k = 0; k < 3; k++) {
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//       vcount = 0;
//       vskip = 0;
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana;
// for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted<banana>(1, 1) {
//       print(i);
//     }
//   }
//
//   banana.reset();
// })",
//       letter++, 1, 1);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int vcount = 0;
//     int vskip = 0;
//     for (int k = 0; k < 3; k++) {
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(1);
// for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted::banana {
//       print(i);
//     }
//   }
// })",
//       letter++, 1, 0);
//
//   [](std::string_view code, char name, int count, int skip, bool (*cond)(int) = always_true_cond,
//       bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int vcount = 0;
//     int vskip = 0;
//     for (int k = 0; k < 3; k++) {
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and vskip++ >= skip and vcount++ < count) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(1);
// for(var k = 0; k < 3; k++) {
//   for(var i = 0; i < 10; i++) {
//     if(i > 2) @restricted::banana {
//       print(i);
//     }
//   }
// })",
//       letter++, 1, 0, [](int i) { return i > 2; });
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(1, 1);
// for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted<banana>(1, 2) {
//       print(k, "-", i);
//     }
//   }
// })",
//       letter++, 1, 2, 1, 1);
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(inf, 2);
// for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted<banana>(1) {
//       print(k, "-", i);
//     }
//   }
// })",
//       letter++, 1, 0, 10000, 2);
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(2);
// for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted<banana>(1) {
//       print(k, "-", i);
//     }
//   }
// })",
//       letter++, 1, 0, 2, 0);
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(2, 2);
// for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted::banana {
//       print(k, "-", i);
//     }
//   }
// })",
//       letter++, 10000, 0, 2, 2);
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(4, 2);
// for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted<banana>(2, 3) {
//       print(k, "-", i);
//     }
//   }
// })",
//       letter++, 2, 3, 4, 2);
//
//
//
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(@scope banana(1, 1);
// for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     if(i > 2) @restricted<banana>(2, 3) {
//       print(k, "-", i);
//     }
//   }
// })",
//     letter++, 2, 3, 1, 1, [](int i) { return i > 2; });
//
//
//   letter = 'A';
//
//   [](std::string_view code, char name, int count, int skip, int parent_count, int parent_skip,
//       bool (*cond)(int) = always_true_cond, bool (*reset)(int) = always_false_cond) {
//     std::cout << "'''\n" << code << "\n'''\nCASE " << name << ": [";
//
//     bool did_output = false;
//     int pcount = 0;
//     int pskip = 0;
//     for (int k = 0; k < 5; k++) {
//       int vcount = 0;
//       int vskip = 0;
//
//       for (int i = 0; i < 10; i++) {
//         if (cond(i) and (vskip++ >= skip and vcount++ < count)
//             and (pskip++ >= parent_skip and pcount++ < parent_count)) {
//
//           if (std::exchange(did_output, true)) {
//             std::cout << ", ";
//           }
//
//           std::cout << k << "-" << i;
//         }
//
//         if (reset(i)) {
//           vcount = 0;
//           vskip = 0;
//         }
//       }
//     }
//     std::cout << "]\n\n";
//   }(R"(for(var k = 0; k < 5; k++) {
//   for(var i = 0; i < 10; i++) {
//     @restricted(inf, 1) {
//       print(k, "-", i);
//     }
//   }
// })",
//     letter++, 2, 3, 1, 1, [](int i) { return i > 2; });
// }
