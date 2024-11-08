#pragma once

#include <zscript.h>

struct profiler {
  static void init();
  static zs::allocate_t get_engine_allocator();
  static void set_verbose(bool verbose);
  static void terminate();
};

void* operator new(size_t n);
void operator delete(void* p) noexcept;
