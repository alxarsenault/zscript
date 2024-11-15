//
// detail/timer_scheduler.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_TIMER_SCHEDULER_HPP
#define ASIO_DETAIL_TIMER_SCHEDULER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"
#include "asio/detail/timer_scheduler_fwd.h"

#if defined(ASIO_WINDOWS_RUNTIME)
# include "asio/detail/winrt_timer_scheduler.h"
#elif defined(ASIO_HAS_IOCP)
# include "asio/detail/win_iocp_io_context.h"
#elif defined(ASIO_HAS_IO_URING_AS_DEFAULT)
# include "asio/detail/io_uring_service.h"
#elif defined(ASIO_HAS_EPOLL)
# include "asio/detail/epoll_reactor.h"
#elif defined(ASIO_HAS_KQUEUE)
# include "asio/detail/kqueue_reactor.h"
#elif defined(ASIO_HAS_DEV_POLL)
# include "asio/detail/dev_poll_reactor.h"
#else
# include "asio/detail/select_reactor.h"
#endif

#endif // ASIO_DETAIL_TIMER_SCHEDULER_HPP
