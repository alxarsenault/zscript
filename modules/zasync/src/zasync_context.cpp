#include "zasync_context.h"
#include "zvirtual_machine.h"
#include "objects/zfunction_prototype.h"

namespace zs::async {
static constexpr std::string_view s_timer_object_typeid = "__async_timer_object__";
static constexpr std::string_view s_hook_object_typeid = "__async_hook_object__";

static timer_object& get_timer_object(const object& obj) { return obj.as_udata().data_ref<timer_object>(); }
static hook_object& get_hook_object(const object& obj) { return obj.as_udata().data_ref<hook_object>(); }

context* timer_object::get_context() const { return (context*)uv_loop_get_data(handle.loop); }

context* hook_object::get_context() const { return (context*)uv_loop_get_data(idle_handle.loop); }

zs::object context::create_timer_object(zs::object uid, zs::object callback) {
  zs::engine* eng = _vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(timer_object));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) timer_object{ std::move(uid), std::move(callback) };

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    timer_object* t = (timer_object*)ptr;
    t->~timer_object();
  });

  uobj->set_uid(zs::_sv(s_timer_object_typeid));
  uobj->set_typeid(zs::_sv(s_timer_object_typeid));
  uobj->set_delegate(_timer_delegate);

  return obj;
}

zs::object context::create_hook_object(zs::object uid, zs::object callback, hook_object::hook_type type) {
  zs::engine* eng = _vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(hook_object));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) hook_object{ std::move(uid), std::move(callback), type };

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    hook_object* t = (hook_object*)ptr;
    t->~hook_object();
  });

  uobj->set_uid(zs::_sv(s_hook_object_typeid));
  uobj->set_typeid(zs::_sv(s_hook_object_typeid));
  uobj->set_delegate(_hook_delegate);

  return obj;
}

context::context(zs::vm_ref vm)
    : _vm(vm)
    , _timers((zs::allocator<object>(vm.get_engine())))
    , _hooks((zs::allocator<object>(vm.get_engine()))) {

  _timer_delegate = create_timer_object_delegate(_vm);
  _hook_delegate = create_hook_object_delegate(_vm);

  if (uv_loop_init(&_loop)) {
    zb::print("ERROR");
  }

  uv_loop_set_data(&_loop, this);
  //           uv_unref((uv_handle_t*) &_timer_handle);
}

context::~context() {

  //    zb::print("uv_has_ref", uv_has_ref((const uv_handle_t *)&_timer_handle));
  uv_loop_close(&_loop);
}

int context::run() { return uv_run(&_loop, uv_run_mode::UV_RUN_DEFAULT); }

void context::stop() { uv_stop(&_loop); }

zs::error_result context::create_hook(
    zs::object uid, zs::object callback, hook_object::hook_type type, zs::object& hook_obj_output) {

  if (hook_object* hook_obj = get_hook(uid)) {
    return zs::errc::already_exists;
  }

  object obj = create_hook_object(std::move(uid), std::move(callback), type);
  hook_object& hook_obj = get_hook_object(obj);

  switch (type) {
  case hook_object::hook_type::idle:
    uv_idle_init(&_loop, &hook_obj.idle_handle);
    hook_obj.idle_handle.data = obj.as_udata().data();
    break;

  case hook_object::hook_type::check:
    uv_check_init(&_loop, &hook_obj.check_handle);
    hook_obj.check_handle.data = obj.as_udata().data();
    break;

  case hook_object::hook_type::prepare:
    uv_prepare_init(&_loop, &hook_obj.prep_handle);
    hook_obj.prep_handle.data = obj.as_udata().data();
    break;
  }

  _hooks.push_back(obj);

  hook_obj_output = obj;

  return {};
}

zs::error_result context::start_hook(const zs::object& uid) {

  hook_object* hook_obj = get_hook(uid);
  if (!hook_obj) {
    return zs::errc::not_found;
  }

  switch (hook_obj->type) {
  case hook_object::hook_type::idle:
    if (int res = uv_idle_start(
            &hook_obj->idle_handle, [](uv_idle_t* handle) { get_context(handle)->idle_cb(handle); })) {
      return zs::errc::unknown;
    }
    break;

  case hook_object::hook_type::check:
    if (int res = uv_check_start(
            &hook_obj->check_handle, [](uv_check_t* handle) { get_context(handle)->check_cb(handle); })) {
      return zs::errc::unknown;
    }
    break;

  case hook_object::hook_type::prepare:
    if (int res = uv_prepare_start(
            &hook_obj->prep_handle, [](uv_prepare_t* handle) { get_context(handle)->prep_cb(handle); })) {
      return zs::errc::unknown;
    }
    break;
  }

  return {};
}

zs::error_result context::stop_hook(const zs::object& uid) {

  hook_object* hook_obj = get_hook(uid);
  if (!hook_obj) {
    return zs::errc::not_found;
  }

  switch (hook_obj->type) {
  case hook_object::hook_type::idle:
    if (int res = uv_idle_stop(&hook_obj->idle_handle)) {
      return zs::errc::unknown;
    }
    break;

  case hook_object::hook_type::check:
    if (int res = uv_check_stop(&hook_obj->check_handle)) {
      return zs::errc::unknown;
    }
    break;

  case hook_object::hook_type::prepare:
    if (int res = uv_prepare_stop(&hook_obj->prep_handle)) {
      return zs::errc::unknown;
    }
    break;
  }
  return {};
}

zs::error_result context::remove_hook(const zs::object& uid) {
  auto it = find_hook(uid);

  if (it == _hooks.end()) {
    return zs::errc::not_found;
  }

  hook_object& hook_obj = get_hook_object(*it);

  switch (hook_obj.type) {
  case hook_object::hook_type::idle:
    if (uv_is_active((const uv_handle_t*)&hook_obj.idle_handle)) {
      if (int res = uv_idle_stop(&hook_obj.idle_handle)) {
        zb::print("ERROR", res);
      }
    }
    break;

  case hook_object::hook_type::check:
    if (uv_is_active((const uv_handle_t*)&hook_obj.check_handle)) {
      if (int res = uv_check_stop(&hook_obj.check_handle)) {
        zb::print("ERROR", res);
      }
    }
    break;

  case hook_object::hook_type::prepare:
    if (uv_is_active((const uv_handle_t*)&hook_obj.prep_handle)) {
      if (int res = uv_prepare_stop(&hook_obj.prep_handle)) {
        zb::print("ERROR", res);
      }
    }
    break;
  }

  _hooks.erase(it);
  return {};
}

zs::error_result context::create_timer(zs::object uid, zs::object callback, zs::object& timer_obj_output) {

  if (timer_object* tm_obj = get_timer(uid)) {
    return zs::errc::already_exists;
  }

  object obj = create_timer_object(std::move(uid), std::move(callback));
  timer_object& tm_obj = get_timer_object(obj);

  uv_timer_init(&_loop, &tm_obj.handle);
  tm_obj.handle.data = obj.as_udata().data();

  _timers.push_back(obj);

  timer_obj_output = obj;

  return {};
}

zs::error_result context::start_timer(const zs::object& uid, uint64_t repeat, uint64_t timeout) {

  timer_object* tm_obj = get_timer(uid);
  if (!tm_obj) {
    return zs::errc::not_found;
  }

  if (int res = uv_timer_start(
          &tm_obj->handle, [](uv_timer_t* handle) { get_context(handle)->timer_cb(handle); }, timeout,
          repeat)) {
    return zs::errc::unknown;
  }

  return {};
}

zs::error_result context::stop_timer(const zs::object& uid) {

  timer_object* tm_obj = get_timer(uid);
  if (!tm_obj) {
    return zs::errc::not_found;
  }

  if (int res = uv_timer_stop(&tm_obj->handle)) {
    return zs::errc::unknown;
  }

  return {};
}

zs::error_result context::remove_timer(const zs::object& uid) {
  auto it = find_timer(uid);

  if (it == _timers.end()) {
    return zs::errc::not_found;
  }

  timer_object& timer_obj = get_timer_object(*it);

  if (uv_is_active((const uv_handle_t*)&timer_obj.handle)) {
    if (int res = uv_timer_stop(&timer_obj.handle)) {
      zb::print("ERROR", res);
    }
  }

  _timers.erase(it);
  return {};
}

zs::error_result context::set_timer_repeat(const zs::object& uid, uint64_t repeat) {
  timer_object* tm_obj = get_timer(uid);
  if (!tm_obj) {
    return zs::errc::not_found;
  }
  uv_timer_set_repeat(&tm_obj->handle, repeat);
  return {};
}

void context::idle_cb(uv_idle_t* handle) {

  hook_object* hook_obj = (hook_object*)handle->data;

  auto it = _hooks.find_if([&](const object& t) { return get_hook_object(t).uid == hook_obj->uid; });

  if (it == _hooks.end()) {
    //    return zs::errc::not_found;
    return;
  }

  if (hook_obj->callback.is_function()) {
    zs::object ret;

    if (hook_obj->callback.is_closure()) {
      if (hook_obj->callback.as_closure().get_proto()._parameter_names.size() == 1) {
        if (auto err = _vm->call(hook_obj->callback, _vm->get_root(), ret)) {
          zb::print("ERROR:", err.message());
        }
        return;
      }
    }

    if (auto err = _vm->call(hook_obj->callback, { _vm->get_root(), *it }, ret)) {
      zb::print("ERROR:", err.message());
    }
  }
}

void context::prep_cb(uv_prepare_t* handle) {
  hook_object* hook_obj = (hook_object*)handle->data;

  auto it = _hooks.find_if([&](const object& t) { return get_hook_object(t).uid == hook_obj->uid; });

  if (it == _hooks.end()) {
    //    return zs::errc::not_found;
    return;
  }

  if (hook_obj->callback.is_function()) {
    zs::object ret;

    if (hook_obj->callback.is_closure()) {
      if (hook_obj->callback.as_closure().get_proto()._parameter_names.size() == 1) {
        if (auto err = _vm->call(hook_obj->callback, _vm->get_root(), ret)) {
          zb::print("ERROR:", err.message());
        }
        return;
      }
    }

    if (auto err = _vm->call(hook_obj->callback, { _vm->get_root(), *it }, ret)) {
      zb::print("ERROR:", err.message());
    }
  }
}

void context::check_cb(uv_check_t* handle) {
  hook_object* hook_obj = (hook_object*)handle->data;

  auto it = _hooks.find_if([&](const object& t) { return get_hook_object(t).uid == hook_obj->uid; });

  if (it == _hooks.end()) {
    //    return zs::errc::not_found;
    return;
  }

  if (hook_obj->callback.is_function()) {
    zs::object ret;

    if (hook_obj->callback.is_closure()) {
      if (hook_obj->callback.as_closure().get_proto()._parameter_names.size() == 1) {
        if (auto err = _vm->call(hook_obj->callback, _vm->get_root(), ret)) {
          zb::print("ERROR:", err.message());
        }
        return;
      }
    }

    if (auto err = _vm->call(hook_obj->callback, { _vm->get_root(), *it }, ret)) {
      zb::print("ERROR:", err.message());
    }
  }
}

void context::timer_cb(uv_timer_t* handle) {
  //  zb::print("timer", uv_now(handle->loop));

  //  context* self = get_context(handle->loop);

  timer_object* tm_obj = (timer_object*)handle->data;

  auto it = _timers.find_if(
      [&](const object& t) { return t.as_udata().data_ref<timer_object>().uid == tm_obj->uid; });

  if (it == _timers.end()) {
    //    return zs::errc::not_found;
    return;
  }

  if (tm_obj->callback.is_function()) {
    zs::object ret;

    if (tm_obj->callback.is_closure()) {
      if (tm_obj->callback.as_closure().get_proto()._parameter_names.size() == 1) {
        if (auto err = _vm->call(tm_obj->callback, _vm->get_root(), ret)) {
          zb::print("ERROR:", err.message());
        }
        return;
      }
    }

    if (auto err = _vm->call(tm_obj->callback, { _vm->get_root(), *it }, ret)) {
      zb::print("ERROR:", err.message());
    }
  }

  //  if (_counter++ >= 10) {
  //    stop();
  //  }
}

zs::object context::create_timer_object_delegate(zs::vm_ref vm) {

  zs::object timer_delegate = zs::_t(vm.get_engine());
  zs::table_object& tbl = timer_delegate.as_table();

  tbl["uid"] = +[](zs::vm_ref vm) -> zs::int_t {
    const timer_object& tm_obj = get_timer_object(vm[0]);

    return vm.push(tm_obj.uid);
  };

  tbl["stop"] = +[](zs::vm_ref vm) -> zs::int_t {
    const timer_object& tm_obj = get_timer_object(vm[0]);
    tm_obj.get_context()->stop_timer(tm_obj.uid);
    return 0;
  };

  tbl["context"] = +[](zs::vm_ref vm) -> zs::int_t {
    const timer_object& tm_obj = get_timer_object(vm[0]);

    return vm.push(tm_obj.get_context()->_self.get_weak_ref_value());
  };

  tbl["__typeof"] = zs::_ss("timer_object");

  tbl["start"] = +[](zs::vm_ref vm) -> zs::int_t {
    int_t nargs = vm.stack_size();

    if (nargs < 2) {
      vm.set_error("Invalid timer_object::start().");
      return -1;
    }

    const timer_object& tm_obj = get_timer_object(vm[0]);

    const object& delta_ms = vm[1];

    if (!delta_ms.is_number()) {
      vm.set_error("Invalid delta ms in timer_object::start().");
      return -1;
    }

    int_t delta_ms_int = delta_ms.convert_to_integer_unchecked();

    int_t timeout_ms_int = 0;

    if (nargs >= 3) {
      const object& timeout_ms = vm[2];

      if (!timeout_ms.is_number()) {
        vm.set_error("Invalid timeout ms in timer_object::start().");
        return -1;
      }

      timeout_ms_int = timeout_ms.convert_to_integer_unchecked();
    }

    if (auto err = tm_obj.get_context()->start_timer(tm_obj.uid, delta_ms_int, timeout_ms_int)) {
      vm.set_error(err.message());
      return -1;
    }

    return 0;
  };

  tbl["set_repeat"] = +[](zs::vm_ref vm) -> zs::int_t {
    int_t nargs = vm.stack_size();

    if (nargs != 2) {
      vm.set_error("Invalid timer_object::set_repeat().");
      return -1;
    }

    const timer_object& tm_obj = get_timer_object(vm[0]);

    const object& delta_ms = vm[1];

    if (!delta_ms.is_number()) {
      vm.set_error("Invalid delta ms in timer_object::set_repeat().");
      return -1;
    }

    int_t delta_ms_int = delta_ms.convert_to_integer_unchecked();

    if (auto err = tm_obj.get_context()->set_timer_repeat(tm_obj.uid, delta_ms_int)) {
      vm.set_error(err.message());
      return -1;
    }

    return 0;
  };

  return timer_delegate;
}

zs::object context::create_hook_object_delegate(zs::vm_ref vm) {

  zs::object hook_delegate = zs::_t(vm.get_engine());
  zs::table_object& tbl = hook_delegate.as_table();

  tbl["uid"] = +[](zs::vm_ref vm) -> zs::int_t {
    const hook_object& hook_obj = get_hook_object(vm[0]);
    return vm.push(hook_obj.uid);
  };

  tbl["context"] = +[](zs::vm_ref vm) -> zs::int_t {
    const hook_object& hook_obj = get_hook_object(vm[0]);
    return vm.push(hook_obj.get_context()->_self.get_weak_ref_value());
  };

  tbl["__typeof"] = zs::_ss("hook_object");

  tbl["type"] = +[](zs::vm_ref vm) -> zs::int_t {
    const hook_object& hook_obj = get_hook_object(vm[0]);
    return vm.push((int_t)hook_obj.type);
  };

  tbl["start"] = +[](zs::vm_ref vm) -> zs::int_t {
    int_t nargs = vm.stack_size();

    if (nargs != 1) {
      vm.set_error("Invalid hook_object::start().");
      return -1;
    }

    const hook_object& hook_obj = get_hook_object(vm[0]);

    if (auto err = hook_obj.get_context()->start_hook(hook_obj.uid)) {
      vm.set_error(err.message());
      return -1;
    }

    return 0;
  };

  tbl["stop"] = +[](zs::vm_ref vm) -> zs::int_t {
    const hook_object& hook_obj = get_hook_object(vm[0]);
    hook_obj.get_context()->stop_hook(hook_obj.uid);
    return 0;
  };

  tbl["remove"] = +[](zs::vm_ref vm) -> zs::int_t {
    const hook_object& hook_obj = get_hook_object(vm[0]);
    hook_obj.get_context()->remove_hook(hook_obj.uid);
    return 0;
  };

  return hook_delegate;
}

zs::vector<object>::iterator context::find_timer(const object& uid) {
  return _timers.find_if([&](const object& t) { return get_timer_object(t).uid == uid; });
}

timer_object* context::get_timer(const object& uid) {
  auto it = find_timer(uid);
  return it == _timers.end() ? nullptr : &get_timer_object(*it);
}

zs::vector<object>::iterator context::find_hook(const object& uid) {
  return _hooks.find_if([&](const object& t) { return get_hook_object(t).uid == uid; });
}

hook_object* context::get_hook(const object& uid) {
  auto it = find_hook(uid);
  return it == _hooks.end() ? nullptr : &get_hook_object(*it);
}
} // namespace zs::async.
