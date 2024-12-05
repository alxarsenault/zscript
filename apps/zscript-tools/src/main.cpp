#include <zscript.h>
#include "zvirtual_machine.h"
#include "lang/zpreprocessor.h"
#include "options.h"



zs::error_result compile_file(zs::vm_ref vm, const options& args, zs::object& closure) {
  zs::engine* eng = vm.get_engine();

  if (!args.include_directories.empty()) {
    for (const auto& dir : args.include_directories) {
      eng->add_import_directory(dir);
    }
  }

  if (!args.defines.empty()) {
    for (const auto& def : args.defines) {
      vm->get_root().as_table()[def.first] = zs::_s(vm, def.second);
    }
  }
 
  std::string code;
  
  {
    zs::file_loader loader(eng);
    
    if (auto err = loader.open(args.filepath.c_str())) {
      return err;
    }
    
    code = loader.content();
  }
  
  {
    zs::object code_output;
    zs::preprocessor preproc(eng);
    
    if (auto err = preproc.preprocess(code, args.filepath.c_str(), code_output)) {
      zb::stream_print<"", "\n">(std::cerr, "Preprocessor error: ", preproc.get_error());
      return err;
    }
    
    if (!code_output.is_string()) {
      zb::stream_print(std::cerr, "Preprocessor failed\n");
      return zs::errc::invalid_type;
    }
    
    code = code_output.get_string_unchecked();
  }
 
  if(auto err = vm->compile_buffer(code, args.filepath.c_str(), closure, true)) {
    zb::stream_print<"", "\n">(std::cerr, "Compiler error: ", vm.get_error());
    return err;
  }
 
  if (!closure.is_closure()) {
    zb::stream_print(std::cerr, "Invalid compile result type.\n");
    return zs::errc::invalid_type;
  }
 
 

  
  
  
  
  
  
  return {};
}

 

  
zs::error_result run_file(zs::vm_ref vm, const options& args, const zs::object& closure, zs::object& result_value) {
  zs::engine* eng = vm.get_engine();

  zs::object args_array = zs::_a(eng, args.args);
  
  zs::object env = vm->create_this_table_from_root();
  
  // Call the compiled file.
  if (auto err = vm->call(closure, { env , args_array}, result_value)) {
    zb::stream_print (std::cerr, "Virtual machine error: ", vm.get_error(), "\n");
    return err;
  }

  closure.as_closure().set_env(std::move(env));
  
  return {};
}
 
zs::error_result call_main(zs::vm_ref vm, const options& args, const zs::object& module_obj, zs::object& result_value) {
  zs::engine* eng = vm.get_engine();
 
  zs::object main_closure;
 
  if(auto err = vm->get(module_obj, zs::_s(eng, args.main_function), main_closure)) {
    //zb::stream_print(std::cerr, "Could not find main function\n");
    return err;
  }
 
  zs::object args_array = zs::_a(eng, args.args);
 
  if (!main_closure.is_closure()) {
    return {};
  }

  if (auto err = vm->call(main_closure, { vm->get_root(), args_array }, result_value)) {
    zb::stream_print(std::cerr, "Virtual machine error: ", vm.get_error(), "\n");
    return err;
  }
 
  return {};
}

int main(int argc, char* argv[]) {
 
  options args = parse_opts(argc, argv);

  zs::vm vm;

  zs::object closure;
  if(auto err = compile_file(vm, args, closure)) {
    return -1;
  }

  zs::object module_obj;
  if(auto err = run_file(vm, args, closure, module_obj)) {
    return -1;
  }
  
  zs::object result_value;
  if(auto err = call_main(vm, args, module_obj, result_value)) {
    return -1;
  }
  
    if (result_value.is_integer()) {
      return (int)result_value._int;
    }
  
  return 0;
}








//#include <zbase/sys/apple/objc/objc.h>
//#include <CoreFoundation/CoreFoundation.h>
//#include <CoreGraphics/CoreGraphics.h>

//namespace objc = zb::apple::objc;
//using namespace objc::literals;
//
//static objc::obj_t* get_shared_application() {
//  return objc::get_class_property("NSApplication", "sharedApplication");
//  //    return objc::msg_send<objc::obj_t*>("NSApplication"_cls, "sharedApplication"_sel);
//}
//
//void stop_run_loop() {
//   objc::autoreleasepool arp;
//   objc::obj_t* app = get_shared_application();
//   // Request the run loop to stop. This doesn't immediately stop the loop.
//
//   objc::call<void, objc::obj_t*>(app, "stop:", nullptr);
//
//   // The run loop will stop after processing an NSEvent.
//   // Event type: NSEventTypeApplicationDefined (macOS 10.12+),
//   //             NSApplicationDefined (macOS 10.0–10.12)
//   int type = 15;
//   objc::obj_t* event = objc::msg_send<objc::obj_t*>("NSEvent"_cls,
//       "otherEventWithType:location:modifierFlags:timestamp:windowNumber:context:subtype:data1:data2:"_sel,
//       type, CGPointMake(0, 0), 0, 0, 0, nullptr, 0, 0, 0);
//   objc::msg_send(app, "postEvent:atStart:"_sel, event, true);
// }
//
// objc::class_t* create_app_delegate_class() {
//    objc::autoreleasepool arp;
//    constexpr auto class_name = "WpAppDelegate";
//
//    // Avoid crash due to registering same class twice.
//    objc::class_t* cls = objc::get_class(class_name);
////
//    if (!cls) {
//      // Note: Avoid registering the class name "AppDelegate" as it is the
//      // default name in projects created with Xcode, and using the same name
//      // causes objc_registerClassPair to crash.
//
//      cls = objc::allocate_class(objc::get_class("NSResponder"), class_name);
// 
//      objc::add_class_method(
//          cls, "applicationShouldTerminateAfterLastWindowClosed:",
//          +[](objc::obj_t* self, objc::selector_t*, objc::obj_t*) -> bool {
//            return true;
//          },
//          "c@:@");
//
//      objc::add_class_method(
//          cls, "applicationDidFinishLaunching:",
//          +[](objc::obj_t* self, objc::selector_t*, objc::obj_t* notification) {
////            objc::obj_t* app = objc::call<objc::obj_t*>(notification, "object"_sel);
////
////            //            objc::obj_t* app = objc::msg_send<objc::obj_t*>(notification, "object"_sel);
////
////            pimpl* w = get_associated_webview(self);
////            w->on_application_did_finish_launching(self, app);
//            zb::print("DLSKDJSKJDLSKD");
//            stop_run_loop();
//            
//          },
//          "v@:@");
// 
//
//      //
//
//      objc::register_class(cls);
//    }
//
//    return cls;
////    //    return objc::msg_send<objc::obj_t*>(cls, "new"_sel);
//  }
//




//
//int main(int argc, char* argv[]) {
//
//
////  objc::obj_t* shared_app = get_shared_application();
////
////      // Only set the app delegate if it hasn't already been set.
////      objc::obj_t* delegate = objc::call<objc::obj_t*>(shared_app, "delegate"_sel);
////      if (delegate) {
////        //      set_up_window();
////        return;
////      }
////
////      objc::class_t* app_delegate_cls = create_app_delegate_class();
////    objc::obj_t* app_delegate = objc::create_object(app_delegate_cls);
//// 
////    objc::call<void, objc::obj_t*>(shared_app, "setDelegate:", app_delegate);
////    objc::call(shared_app, "run");
//  
////
//  
//  
////  {
////  
////    objc::obj_t* app = get_shared_application();
////     objc::call(app, "run");
////    
////  }
//  
//  options args = parse_opts(argc, argv);
//
//  zs::vm vm;
//
//  if (!args.include_directories.empty()) {
//    for (const auto& dir : args.include_directories) {
//      vm.get_engine()->add_import_directory(dir);
//    }
//  }
//
//  if (!args.defines.empty()) {
//    for (const auto& def : args.defines) {
//      vm->get_root().as_table()[def.first] = zs::_s(vm, def.second);
//    }
//  }
//
//  zs::object closure;
// 
//  // Compile the file.
//  if (zs::error_result err = vm->compile_file(args.filepath, args.filepath.c_str(), closure, true)) {
//    zb::print("Compiler error:", vm->get_error());
//    return -1;
//  }
////  if (zs::error_result err = vm->compile_file(args.filepath, args.filepath.stem().c_str(), closure, true)) {
////    zb::print("Compiler error:", vm->get_error());
////    return -1;
////  }
//
//  if (!closure.is_closure()) {
//    zb::print("Invalid compile result type.");
//    return -1;
//  }
// 
//  zs::object args_array = zs::_a(vm.get_engine(), args.args);
//  
//  // Call the compiled file.
//  zs::object result_value;
//
//  if (zs::error_result err = vm->call(closure, { vm->get_root() , args_array}, result_value)) {
//    zb::print(vm.get_error());
//    return -1;
//  }
//
//  zs::object main_closure = vm->get_root().as_table()[args.main_function];
//
//  //  if (zs::object_ptr f = closure.as_closure().get_proto().find_function(args.main_function);
//  //      f.is_function_prototype()) {
//  //    main_closure = zs::object_ptr::create_closure(vm.get_engine(), f, vm->get_root());
//  //  }
//
//  if (!main_closure.is_closure()) {
//    return 0;
//  }
//
////  zs::object args_array = zs::_a(vm.get_engine(), args.args);
//
//  if (zs::error_result err = vm->call(main_closure, { vm->get_root(), args_array }, result_value)) {
//    zb::print(vm.get_error());
//    return -1;
//  }
//
//  if (result_value.is_integer()) {
//    return (int)result_value._int;
//  }
//
//  return 0;
//}

//
//  zb::print("TOP", vm->stack_size(), vm->stack_get(-2).get_type());
//
//  for (zs::int_t i = 0; i < vm->stack_size(); i++) {
//    zb::print("----", i, vm->stack_get(i).get_type(),
//    vm->stack_get(i).to_debug_string());
//  }
//
//  zb::print(zs::k_instruction_sizes.size(), (size_t)zs::opcode::count);
//
//  zs::vector<zs::local_var_info_t>& vlocals =
//  zs::object_proxy::as_function_prototype(res)->_vlocals; for (const auto& l :
//  vlocals) {
//    zb::print(l._name.get_string_unchecked(), l._start_op, l._end_op, l._pos);
//  }
//
//  zs::instruction_vector& ivector =
//  zs::object_proxy::as_function_prototype(res)->_instructions;
//  zb::print(ivector._data.size());
//
//  for (auto it = ivector.begin(); it != ivector.end(); ++it) {
//
//    switch (it.get_opcode()) {
// #define ZS_DECL_OPCODE(name) \
//  case zs::opcode::op_##name: \
//    zb::print(it.get_ref<zs::opcode::op_##name>());
// break;
//
// #include "lang/zopcode_def.h"
// #undef ZS_DECL_OPCODE
//
//    default:
//      zb::print(it.get_opcode());
//    }
//  }
