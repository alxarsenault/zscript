#include <zscript.h>

namespace zs {

void throw_error(zs::error_code ec) {
#if ZS_CONFIG_USE_EXCEPTION
  throw zs::exception(ec);
#else
  fprintf(stderr, "%s\n", zs::error_result(ec).message());
  ::abort();
#endif // ZS_CONFIG_USE_EXCEPTION.
}

} // namespace zs.
