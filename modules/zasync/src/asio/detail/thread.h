//
// detail/thread.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_THREAD_HPP
#define ASIO_DETAIL_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"

#if !defined(ASIO_HAS_THREADS)
# include "asio/detail/null_thread.h"
#elif defined(ASIO_HAS_PTHREADS)
# include "asio/detail/posix_thread.h"
#elif defined(ASIO_WINDOWS)
# if defined(UNDER_CE)
#  include "asio/detail/wince_thread.h"
# elif defined(ASIO_WINDOWS_APP)
#  include "asio/detail/winapp_thread.h"
# else
#  include "asio/detail/win_thread.h"
# endif
#else
# include "asio/detail/std_thread.h"
#endif

namespace asio {
namespace detail {

#if !defined(ASIO_HAS_THREADS)
typedef null_thread thread;
#elif defined(ASIO_HAS_PTHREADS)
typedef posix_thread thread;
#elif defined(ASIO_WINDOWS)
# if defined(UNDER_CE)
typedef wince_thread thread;
# elif defined(ASIO_WINDOWS_APP)
typedef winapp_thread thread;
# else
typedef win_thread thread;
# endif
#else
typedef std_thread thread;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_THREAD_HPP