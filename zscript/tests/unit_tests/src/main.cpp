#define ZTESTS_MAIN
#include <ztests/ztests.h>
#include <zscript/zscript.h>

__attribute__((constructor)) static void print_zscript_version() { zb::print(zs::version()); }
