//
// detail/winrt_socket_connect_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WINRT_SOCKET_CONNECT_OP_HPP
#define ASIO_DETAIL_WINRT_SOCKET_CONNECT_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"

#if defined(ASIO_WINDOWS_RUNTIME)

#include "asio/detail/bind_handler.h"
#include "asio/detail/buffer_sequence_adapter.h"
#include "asio/detail/fenced_block.h"
#include "asio/detail/handler_alloc_helpers.h"
#include "asio/detail/handler_work.h"
#include "asio/detail/memory.h"
#include "asio/detail/winrt_async_op.h"
#include "asio/error.h"

#include "asio/detail/push_options.h"

namespace asio {
namespace detail {

template <typename Handler, typename IoExecutor>
class winrt_socket_connect_op :
  public winrt_async_op<void>
{
public:
  ASIO_DEFINE_HANDLER_PTR(winrt_socket_connect_op);

  winrt_socket_connect_op(Handler& handler, const IoExecutor& io_ex)
    : winrt_async_op<void>(&winrt_socket_connect_op::do_complete),
      handler_(static_cast<Handler&&>(handler)),
      work_(handler_, io_ex)
  {
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code&, std::size_t)
  {
    // Take ownership of the operation object.
    ASIO_ASSUME(base != 0);
    winrt_socket_connect_op* o(static_cast<winrt_socket_connect_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(
          o->work_));

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder1<Handler, asio::error_code>
      handler(o->handler_, o->ec_);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
      w.complete(handler, handler.handler_);
      ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  Handler handler_;
  handler_work<Handler, IoExecutor> executor_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.h"

#endif // defined(ASIO_WINDOWS_RUNTIME)

#endif // ASIO_DETAIL_WINRT_SOCKET_CONNECT_OP_HPP