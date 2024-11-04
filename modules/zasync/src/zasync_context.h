

#pragma once

#include <zscript/zscript.h>
#include <uv.h>

namespace zs::async {
class context;

struct timer_object {
  zs::object uid;
  zs::object callback;
  uv_timer_t handle;

  context* get_context() const;
};

struct hook_object {
  enum class hook_type { idle, check, prepare };

  zs::object uid;
  zs::object callback;
  hook_type type;
  union {
    uv_idle_t idle_handle;
    uv_check_t check_handle;
    uv_prepare_t prep_handle;
  };

  context* get_context() const;
};

class context {
public:
  context(zs::vm_ref vm);

  ~context();

  int run();

  void stop();

  zs::error_result create_hook(
      zs::object uid, zs::object callback, hook_object::hook_type type, zs::object& hook_obj_output);
  zs::error_result start_hook(const zs::object& uid);

  zs::error_result stop_hook(const zs::object& uid);

  zs::error_result remove_hook(const zs::object& uid);

  zs::error_result create_timer(zs::object uid, zs::object callback, zs::object& timer_obj_output);

  /// Start the timer. timeout and repeat are in milliseconds.
  /// If timeout is zero, the callback fires on the next event loop iteration.
  /// If repeat is non-zero, the callback fires first after timeout milliseconds
  /// and then repeatedly after repeat milliseconds.
  zs::error_result start_timer(const zs::object& uid, uint64_t repeat, uint64_t timeout = 0);

  /// Stop the timer, the callback will not be called anymore.
  zs::error_result stop_timer(const zs::object& uid);

  zs::error_result remove_timer(const zs::object& uid);

  /// Set the repeat interval value in milliseconds. The timer will be scheduled
  /// to run on the given interval, regardless of the callback execution duration,
  /// and will follow normal timer semantics in the case of a time-slice overrun.
  ///
  /// For example, if a 50ms repeating timer first runs for 17ms, it will be
  /// scheduled to run again 33ms later. If other tasks consume more than the
  /// 33ms following the first timer callback, then the callback will run as
  /// soon as possible.
  ///
  /// @note If the repeat value is set from a timer callback it does not
  /// immediately take effect. If the timer was non-repeating before, it will
  /// have been stopped. If it was repeating, then the old repeat value will
  /// have been used to schedule the next timeout.
  zs::error_result set_timer_repeat(const zs::object& uid, uint64_t repeat);

  static context* get_context(uv_loop_t* loop) { return (context*)uv_loop_get_data(loop); }

  template <class T>
  static context* get_context(T* handle) {
    return get_context(handle->loop);
  }

  void prep_cb(uv_prepare_t* handle);

  void idle_cb(uv_idle_t* handle);
  void check_cb(uv_check_t* handle);

  void timer_cb(uv_timer_t* handle);

  static constexpr std::string_view uid = "__async_loop_object__";

  zs::vm_ref _vm;
  uv_loop_t _loop;

  //  uv_prepare_t _prep_handle;
  //  uv_check_t _check_handle;

  zs::vector<zs::object> _timers;
  zs::vector<zs::object> _hooks;
  zs::object _timer_delegate;
  zs::object _hook_delegate;
  zs::object _self;

  static zs::object create_timer_object_delegate(zs::vm_ref vm);
  static zs::object create_hook_object_delegate(zs::vm_ref vm);

  zs::object create_timer_object(zs::object uid, zs::object callback);
  zs::object create_hook_object(zs::object uid, zs::object callback, hook_object::hook_type type);

  zs::vector<object>::iterator find_timer(const object& uid);
  timer_object* get_timer(const object& uid);

  zs::vector<object>::iterator find_hook(const object& uid);
  hook_object* get_hook(const object& uid);
};
} // namespace zs::async.
