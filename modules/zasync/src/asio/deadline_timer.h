//
// deadline_timer.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DEADLINE_TIMER_HPP
#define ASIO_DEADLINE_TIMER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"

#if !defined(ASIO_NO_DEPRECATED)

#if defined(ASIO_HAS_BOOST_DATE_TIME) \
  || defined(GENERATING_DOCUMENTATION)

#include "asio/detail/socket_types.h" // Must come before posix_time.
#include "asio/basic_deadline_timer.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace asio {

/// (Deprecated: Use system_timer.) Typedef for the typical usage of timer. Uses
/// a UTC clock.
typedef basic_deadline_timer<boost::posix_time::ptime> deadline_timer;

} // namespace asio

#endif // defined(ASIO_HAS_BOOST_DATE_TIME)
       // || defined(GENERATING_DOCUMENTATION)

#endif // !defined(ASIO_NO_DEPRECATED)

#endif // ASIO_DEADLINE_TIMER_HPP