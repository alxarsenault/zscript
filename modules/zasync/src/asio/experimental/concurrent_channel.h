//
// experimental/concurrent_channel.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP
#define ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"
#include "asio/any_io_executor.h"
#include "asio/detail/type_traits.h"
#include "asio/execution/executor.h"
#include "asio/is_executor.h"
#include "asio/experimental/basic_concurrent_channel.h"
#include "asio/experimental/channel_traits.h"

#include "asio/detail/push_options.h"

namespace asio {
namespace experimental {
namespace detail {

template <typename ExecutorOrSignature, typename = void>
struct concurrent_channel_type
{
  template <typename... Signatures>
  struct inner
  {
    typedef basic_concurrent_channel<any_io_executor, channel_traits<>,
        ExecutorOrSignature, Signatures...> type;
  };
};

template <typename ExecutorOrSignature>
struct concurrent_channel_type<ExecutorOrSignature,
    enable_if_t<
      is_executor<ExecutorOrSignature>::value
        || execution::is_executor<ExecutorOrSignature>::value
    >>
{
  template <typename... Signatures>
  struct inner
  {
    typedef basic_concurrent_channel<ExecutorOrSignature,
        channel_traits<>, Signatures...> type;
  };
};

} // namespace detail

/// Template type alias for common use of channel.
template <typename ExecutorOrSignature, typename... Signatures>
using concurrent_channel = typename detail::concurrent_channel_type<
    ExecutorOrSignature>::template inner<Signatures...>::type;

} // namespace experimental
} // namespace asio

#include "asio/detail/pop_options.h"

#endif // ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP
