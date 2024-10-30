#include <zbase/sys/apple/cf/run_loop.h>

#if __ZBASE_APPLE__

#include <CoreFoundation/CoreFoundation.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)
run_loop::run_loop() { _native_run_loop = CFRunLoopGetMain(); }

run_loop::~run_loop() {}

void run_loop::run() { CFRunLoopRun(); }

void run_loop::stop() { CFRunLoopStop(_native_run_loop); }

void run_loop::perform(std::unique_ptr<__zb::task> task) {

  __zb::task* task_ptr = task.release();
  CFRunLoopPerformBlock(_native_run_loop, kCFRunLoopDefaultMode, ^() {
      if (task_ptr) {
        task_ptr->call();
        delete task_ptr;
      }
  });
}

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__
