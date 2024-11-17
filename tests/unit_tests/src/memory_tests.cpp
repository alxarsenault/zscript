#include "unit_tests.h"
#include <zscript/object_stack.h>

#include "zvirtual_machine.h"
#include <numeric>

#define ZS_MEMORY_TEST_PRINT_OUTPUT 0

#if ZS_MEMORY_TEST_PRINT_OUTPUT
#define ZS_MEMORY_TEST_PRINT zb::print
#else
#define ZS_MEMORY_TEST_PRINT(...)
#endif // ZS_MEMORY_TEST_PRINT_OUTPUT.

TEST_CASE("zs::memory_test") {

  static std::unordered_map<void*, size_t> addr_map;

  const zs::allocate_t test_allocate = [](zs::engine* eng, zs::raw_pointer_t user_ptr, void* ptr, size_t size,
                                           size_t old_size, zs::alloc_info_t info) -> void* {
    if (!size) {
      zbase_assert(ptr, "invalid pointer");
      addr_map.erase(ptr);
      ::free(ptr);
      ZS_MEMORY_TEST_PRINT("free", ptr);
      return nullptr;
    }

    if (ptr) {
      addr_map.erase(ptr);
      ptr = ::realloc(ptr, size);
      addr_map[ptr] = size;
      ZS_MEMORY_TEST_PRINT("realloc", ptr, size);
      return ptr;
    }

    zbase_assert(!ptr, "pointer should be nullptr");
    ptr = ::malloc(size);
    addr_map[ptr] = size;
    ZS_MEMORY_TEST_PRINT("malloc", ptr, size);
    return ptr;
  };

  zs::virtual_machine* vmachine = zs::create_virtual_machine(32, test_allocate);

  // Get a random working buffer.
  //  vmachine->get_engine()->get_working_buffer(55);

  {
    zs::object closure;

    if (auto err = vmachine->compile_file(ZSTD_PATH("var_decl_int_01.zs"), "test", closure)) {
      REQUIRE(!err);
      return;
    }

    REQUIRE(closure.is_closure());

    zs::int_t n_params = 1;
    vmachine->push_root();

    zs::object ret_value;
    REQUIRE(!vmachine->call(closure, n_params, vmachine->stack_size() - n_params, ret_value));

    ZS_MEMORY_TEST_PRINT("addr_map", addr_map);

    REQUIRE(!addr_map.empty());
  }

  REQUIRE(!addr_map.empty());
  zs::close_virtual_machine(vmachine);

  ZS_MEMORY_TEST_PRINT("addr_map", addr_map);

  REQUIRE(addr_map.empty());
}
