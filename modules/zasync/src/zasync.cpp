
#include <zasync/zasync.h>
#include "zasync_context.h"
#include "zvirtual_machine.h"

namespace zs::async {
zs::object create_async_context(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(context));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) context(vm);

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    context* t = (context*)ptr;
    t->~context();
  });

  uobj->set_uid(zs::_sv(context::uid));
  uobj->set_typeid(zs::_sv(context::uid));
  return obj;
}

zs::object create_zs_async_lib(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();
  zs::object async_module = zs::_t(eng);

  zs::table_object& async_module_tbl = async_module.as_table();

  async_module_tbl["hook_type"] = zs::_t(eng,
      {
          { zs::_ss("idle"), (int_t)hook_object::hook_type::idle },
          { zs::_ss("check"), (int_t)hook_object::hook_type::check },
          { zs::_ss("prepare"), (int_t)hook_object::hook_type::prepare },
      });
  if (auto err = vm->make_enum_table(async_module_tbl["hook_type"])) {
    return err;
  }

  async_module_tbl["context"] = +[](zs::vm_ref vm) -> zs::int_t {
    zs::object obj = create_async_context(vm);

    zs::object delegate = zs::_t(vm.get_engine());
    delegate.as_table()["__typeof"] = zs::_ss("async_context");

    delegate.as_table()["run"] = +[](zs::vm_ref vm) -> zs::int_t {
      vm[0].as_udata().data_ref<context>().run();
      return 0;
    };

    delegate.as_table()["stop"] = +[](zs::vm_ref vm) -> zs::int_t {
      vm[0].as_udata().data_ref<context>().stop();
      return 0;
    };

    delegate.as_table()["create_timer"] = +[](zs::vm_ref vm) -> zs::int_t {
      int_t nargs = vm.stack_size();

      if (nargs < 3) {
        vm.set_error("Invalid async::create_timer().");
        return -1;
      }

      const object& uid = vm[1];
      const object& callback = vm[2];
      object timer_obj;

      if (auto err = vm[0].as_udata().data_ref<context>().create_timer(uid, callback, timer_obj)) {
        vm.set_error(err.message());
        return -1;
      }

      return vm.push(timer_obj);
    };

    delegate.as_table()["start_timer"] = +[](zs::vm_ref vm) -> zs::int_t {
      int_t nargs = vm.stack_size();

      if (nargs < 3) {
        vm.set_error("Invalid async::start_timer().");
        return -1;
      }

      const object& uid = vm[1];
      const object& delta_ms = vm[2];

      if (!delta_ms.is_number()) {
        vm.set_error("Invalid delta ms in async::start_timer().");
        return -1;
      }

      int_t delta_ms_int = delta_ms.convert_to_integer_unchecked();

      int_t timeout_ms_int = 0;

      if (nargs >= 4) {
        const object& timeout_ms = vm[3];

        if (!timeout_ms.is_number()) {
          vm.set_error("Invalid timeout ms in async::start_timer().");
          return -1;
        }

        timeout_ms_int = timeout_ms.convert_to_integer_unchecked();
      }

      if (auto err = vm[0].as_udata().data_ref<context>().start_timer(uid, delta_ms_int, timeout_ms_int)) {
        vm.set_error(err.message());
        return -1;
      }

      return 0;
    };

    delegate.as_table()["stop_timer"] = +[](zs::vm_ref vm) -> zs::int_t {
      int_t nargs = vm.stack_size();

      if (nargs < 2) {
        vm.set_error("Invalid async::stop_timer().");
        return -1;
      }

      const object& uid = vm[1];
      if (auto err = vm[0].as_udata().data_ref<context>().stop_timer(uid)) {
        vm.set_error(err.message());
        return -1;
      }

      return 0;
    };

    delegate.as_table()["create_hook"] = +[](zs::vm_ref vm) -> zs::int_t {
      int_t nargs = vm.stack_size();

      if (nargs != 4) {
        vm.set_error("Invalid async::create_hook().");
        return -1;
      }

      const object& uid = vm[1];
      const object& hook_type_obj = vm[2];
      const object& callback = vm[3];

      if (!(hook_type_obj.is_integer() and hook_type_obj._int <= (int_t)hook_object::hook_type::prepare)) {
        vm.set_error("Invalid async::create_hook().");
        return -1;
      }

      hook_object::hook_type hook_type = (hook_object::hook_type)hook_type_obj._int;

      object hook_obj;

      if (auto err = vm[0].as_udata().data_ref<context>().create_hook(uid, callback, hook_type, hook_obj)) {
        vm.set_error(err.message());
        return -1;
      }

      return vm.push(hook_obj);
    };

    //    delegate.as_table()["set_timer_callback"] = +[](zs::vm_ref vm) -> zs::int_t {
    //      vm[0].as_udata().data_ref<context>()._timer_callback = vm[1];
    //      return 0;
    //    };

    obj.as_udata().set_delegate(delegate);

    obj.as_udata().data_ref<context>()._self = obj.get_weak_ref();

    return vm.push(obj);
  };

  return async_module;
}
} // namespace zs::async.
