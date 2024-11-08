#include <zscript.h>
#include "profiler.h"

int main(int argc, char** argv) {

  profiler::init();
  //    zs::object caaaa;

  {

    zs::vm vm(1024, profiler::get_engine_allocator());
    //
    zs::engine* eng = vm.get_engine();
    zb::print("VM DONE");
    //    zs::engine engine(profiler::get_engine_allocator());
    //        caaaa = zs::_t(eng);
    zs::object obj1 = zs::object::create_table(eng);
    zs::object obj2 = zs::object::create_array(eng, 32);
    //
    //    zs::object cls = zs::object::create_class(eng);
    //    zs::object inst = cls.as_class().create_instance();
  }

  profiler::terminate();
  return 0;
}
