//
// detail/event.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_EVENT_HPP
#define ASIO_DETAIL_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"

#if !defined(ASIO_HAS_THREADS)
# include "asio/detail/null_event.h"
#elif defined(ASIO_WINDOWS)
# include "asio/detail/win_event.h"
#elif defined(ASIO_HAS_PTHREADS)
# include "asio/detail/posix_event.h"
#else
# include "asio/detail/std_event.h"
#endif

namespace asio {
namespace detail {

#if !defined(ASIO_HAS_THREADS)
typedef null_event event;
#elif defined(ASIO_WINDOWS)
typedef win_event event;
#elif defined(ASIO_HAS_PTHREADS)
typedef posix_event event;
#else
typedef std_event event;
#endif

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_EVENT_HPP
