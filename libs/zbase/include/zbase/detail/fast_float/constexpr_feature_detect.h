#pragma once

#include <zbase/zbase.h>

#if ZBASE_HAS_INCLUDE(<version>)
#include <version>
#endif

#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
#define FASTFLOAT_HAS_BIT_CAST 1
#else
#define FASTFLOAT_HAS_BIT_CAST 0
#endif
