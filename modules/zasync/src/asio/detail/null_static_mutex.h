//
// detail/null_static_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_NULL_STATIC_MUTEX_HPP
#define ASIO_DETAIL_NULL_STATIC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"

#if !defined(ASIO_HAS_THREADS)

#include "asio/detail/scoped_lock.h"

#include "asio/detail/push_options.h"

namespace asio {
namespace detail {

struct null_static_mutex
{
  typedef asio::detail::scoped_lock<null_static_mutex> scoped_lock;

  // Initialise the mutex.
  void init()
  {
  }

  // Try to lock the mutex without blocking.
  bool try_lock()
  {
    return true;
  }

  // Lock the mutex.
  void lock()
  {
  }

  // Unlock the mutex.
  void unlock()
  {
  }

  int unused_;
};

#define ASIO_NULL_STATIC_MUTEX_INIT { 0 }

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.h"

#endif // !defined(ASIO_HAS_THREADS)

#endif // ASIO_DETAIL_NULL_STATIC_MUTEX_HPP