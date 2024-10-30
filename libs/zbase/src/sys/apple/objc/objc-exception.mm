#include <zbase/objc.h>
#include <assert.h>

#ifdef __APPLE__

#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <random>

#include <Foundation/Foundation.h>

WP_BEGIN_NAMESPACE
namespace objc {

namespace detail {
  void handle_try_catch_wrapper(std::exception_ptr eptr) {

    //  objc::obj_t* ex = nullptr;

    objc::exception ex;
    try {
      std::rethrow_exception(eptr);
    } catch (NSException* e) {

      ex = objc::exception((obj_t*)e);
      //    if(cb) {
      //      cb((obj_t*)e);
      //    }
    }

    throw ex;
  }
} // namespace detail.

} // namespace objc.
WP_END_NAMESPACE
#endif // __APPLE__
