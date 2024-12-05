#define ZTESTS_MAIN
#include <catch2.h>

#include <uv.h>
#include <zbase/utility/print.h>
#include <zscript.h>
#include "zvirtual_machine.h"

#include <zasync/zasync.h>

TEST_CASE("ASYNC") {
  static constexpr std::string_view code = R"""(
var async = import("async");
 
 
var ctx = async.context();

var count = 0;
var idle_obj = ctx.create_hook("Hook", async.hook_type.check, $(hook){
  zs::print("hook", hook.uid(), count, hook.type());
  count = count + 1;

  if(count > 100) {
//  hook.stop();
    hook.remove();
  }
});

var timer_obj = ctx.create_timer("John", (timer)=>{
  zs::print("TIMER");
  timer.stop();
  timer.context().stop();
});

var timer_obj2 = ctx.create_timer("Johnson", (timer)=>{
  zs::print("Johnson TIMER");
  timer.set_repeat(500);
});

zs::print(typeof(timer_obj), timer_obj.uid(), typeof(timer_obj.context()));
 
idle_obj.start();
ctx.start_timer("John", 1000, 1000);
timer_obj2.start(200);

//ctx.start_timer("Johnson", 200);
 
 
ctx.run();
zs::print("post run JOHNSON");

)""";
  
  zs::vm vm;
  zs::table_object& cache = vm->get_imported_modules().as_table();
  cache["async"] = zs::async::create_zs_async_lib(vm);

  zs::object closure;

  if (auto err = vm->compile_buffer(code, "module.02", closure)) {
    FAIL(vm.get_error());
  }

  REQUIRE(closure.is_closure());

  zs::object value;
  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
    FAIL(vm.get_error());
  }
}

// class loop_object  {
// public:
//   loop_object(zs::vm_ref vm)
//   : _vm(vm) {
//
//
//     if (uv_loop_init(&_loop)) {
//       zb::print("ERROR");
//     }
//
//     uv_loop_set_data(&_loop, this);
//
//
////    uv_prepare_init(&_loop, &_prep_handle);
////    uv_prepare_start(&_prep_handle, prep_cb);
//
////    uv_idle_init(&_loop, &_idle_handle);
////    uv_idle_start(&_idle_handle, idle_cb);
//
//    //
//        uv_timer_init(&_loop, &_timer_handle);
//
////         uv_unref((uv_handle_t*) &_timer_handle);
//
//        uv_timer_start(&_timer_handle, timer_cb, 0, 200);
//  }
//
//
//  ~loop_object() {
//
////    zb::print("uv_has_ref", uv_has_ref((const uv_handle_t *)&_timer_handle));
//    uv_loop_close(&_loop);
//  }
//
//  int run() {
//    return uv_run(&_loop, uv_run_mode::UV_RUN_DEFAULT);
//  }
//
//  void stop() {
//    uv_stop(&_loop);
//  }
//
//  static void prep_cb(uv_prepare_t *handle) {
//    zb::print("prep");
//  }
//
//  static void idle_cb(uv_idle_t* handle) {
//    zb::print("IDLE");
//
//    uv_idle_stop(handle);
//
//  }
//
//  static void timer_cb(uv_timer_t *handle) {
//    zb::print("timer", uv_now(handle->loop));
//
//
//    loop_object* self = get_loop_object(handle->loop);
//
//    if(self->_timer_callback.is_function()) {
//      zs::object ret;
//      if(auto err = self->_vm->call(self->_timer_callback, {self->_vm->get_root()}, ret)) {
//        zb::print("ERROR:", err.message());
//      }
//    }
//
//    if(get_loop_object(handle->loop)->_counter++ >= 10) {
//      get_loop_object(handle->loop)->stop();
//    }
//  }
//
//  static constexpr std::string_view uid = "__async_loop_object__";
//
//  static loop_object* get_loop_object(uv_loop_t *loop) {
//    return (loop_object*)uv_loop_get_data(loop);
//  }
//
//  zs::vm_ref _vm;
//  uv_loop_t _loop;
//  uv_prepare_t _prep_handle;
//  uv_idle_t _idle_handle;
//  uv_timer_t _timer_handle;
//
//  zs::object _timer_callback;
//  int _counter = 0;
//};
//
// zs::object create_loop_object(zs::vm_ref vm) {
//  zs::engine* eng = vm.get_engine();
//
//  zs::object obj;
//  obj._type = zs::object_type::k_user_data;
//
//  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(loop_object));
//  obj._udata = uobj;
//
//  zb_placement_new(uobj->data()) loop_object(vm);
//
//  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
//    loop_object* t = (loop_object*)ptr;
//    t->~loop_object();
//  });
//
//  uobj->set_uid(zs::_sv(loop_object::uid));
//  uobj->set_typeid(zs::_sv(loop_object::uid));
//  return obj;
//}

//
// TEST_CASE("DSKDJJKSD") {
//
//  zs::vm vm;
//  {
//    zs::object loop_obj = create_loop_object(vm);
//    loop_obj.as_udata().data_ref<loop_object>().run();
//
//  }
//
//}

// TEST_CASE("DSDSDSDSDSDSDSDDS") {
//
//   static constexpr std::string_view code = R"""(
// var async = import("async");
//
// zs::print("JOHNSON");
//
// var ctx = async.context();
//
// ctx.set_timer_callback($() { zs::print("TIMER"); });
//
// zs::print("post JOHNSON");
//
// ctx.run();
// zs::print("post run JOHNSON");
//)""";
//
//   zs::vm vm;
//   {
//
//     zs::table_object& cache = vm->get_imported_modules().as_table();
//
//     zs::object async_module = zs::_t(vm.get_engine());
//
//     async_module.as_table()["context"] = +[](zs::vm_ref vm)->zs::int_t {
//
//       zs::object obj = create_loop_object(vm);
//
//       zs::object delegate = zs::_t(vm.get_engine());
//
//       delegate.as_table()["run"] =+[](zs::vm_ref vm)->zs::int_t {
//         vm[0].as_udata().data_ref<loop_object>().run();
//         return 0;
//       };
//
//       delegate.as_table()["set_timer_callback"] =+[](zs::vm_ref vm)->zs::int_t {
//         vm[0].as_udata().data_ref<loop_object>()._timer_callback = vm[1];
//         return 0;
//       };
//
//       obj.as_udata().set_delegate(delegate);
//       return vm.push( obj);
//     };
//
//
//     cache["async"] = async_module;
//
//
//     zs::object closure;
//
//     if (auto err = vm->compile_buffer(code, "module.02", closure)) {
//       FAIL(vm.get_error());
//     }
//
//     REQUIRE(closure.is_closure());
//
//     zs::object value;
//     if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//       FAIL(vm.get_error());
//     }
//   }
//
// }

//
//
// #define FIB_UNTIL 25
// uv_loop_t *loop;
//
// long fib_(long t) {
//    if (t == 0 || t == 1)
//        return 1;
//    else
//        return fib_(t-1) + fib_(t-2);
//}
//
// void fib(uv_work_t *req) {
//    int n = *(int *) req->data;
//    if (random() % 2)
//      uv_sleep(1);
//    else
//      uv_sleep(3);
//    long fib = fib_(n);
//    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
//}
//
// void after_fib(uv_work_t *req, int status) {
//    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) req->data);
//}
//
// TEST_CASE("DSKDJJKSD") {
//
//
//    loop = uv_default_loop();
//
//    int data[FIB_UNTIL];
//    uv_work_t req[FIB_UNTIL];
//    int i;
//    for (i = 0; i < FIB_UNTIL; i++) {
//        data[i] = i;
//        req[i].data = (void *) &data[i];
//        uv_queue_work(loop, &req[i], fib, after_fib);
//
////      uv_print_active_handles(loop, stdout);
//    }
//
//      uv_run(loop, UV_RUN_DEFAULT);
//}
