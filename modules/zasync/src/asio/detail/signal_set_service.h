//
// detail/signal_set_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SIGNAL_SET_SERVICE_HPP
#define ASIO_DETAIL_SIGNAL_SET_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"

#include <cstddef>
#include <signal.h>
#include "asio/associated_cancellation_slot.h"
#include "asio/cancellation_type.h"
#include "asio/error.h"
#include "asio/execution_context.h"
#include "asio/signal_set_base.h"
#include "asio/detail/handler_alloc_helpers.h"
#include "asio/detail/memory.h"
#include "asio/detail/op_queue.h"
#include "asio/detail/signal_handler.h"
#include "asio/detail/signal_op.h"
#include "asio/detail/socket_types.h"

#if defined(ASIO_HAS_IOCP)
# include "asio/detail/win_iocp_io_context.h"
#else // defined(ASIO_HAS_IOCP)
# include "asio/detail/scheduler.h"
#endif // defined(ASIO_HAS_IOCP)

#if !defined(ASIO_WINDOWS) && !defined(__CYGWIN__)
# if defined(ASIO_HAS_IO_URING_AS_DEFAULT)
#  include "asio/detail/io_uring_service.h"
# else // defined(ASIO_HAS_IO_URING_AS_DEFAULT)
#  include "asio/detail/reactor.h"
# endif // defined(ASIO_HAS_IO_URING_AS_DEFAULT)
#endif // !defined(ASIO_WINDOWS) && !defined(__CYGWIN__)

#include "asio/detail/push_options.h"

namespace asio {
namespace detail {

#if defined(NSIG) && (NSIG > 0)
enum { max_signal_number = NSIG };
#else
enum { max_signal_number = 128 };
#endif

extern ASIO_DECL struct signal_state* get_signal_state();

extern "C" ASIO_DECL void asio_signal_handler(int signal_number);

class signal_set_service :
  public execution_context_service_base<signal_set_service>
{
public:
  // Type used for tracking an individual signal registration.
  class registration
  {
  public:
    // Default constructor.
    registration()
      : signal_number_(0),
        queue_(0),
        undelivered_(0),
        next_in_table_(0),
        prev_in_table_(0),
        next_in_set_(0)
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class signal_set_service;

    // The signal number that is registered.
    int signal_number_;

    // The waiting signal handlers.
    op_queue<signal_op>* queue_;

    // The number of undelivered signals.
    std::size_t undelivered_;

    // Pointers to adjacent registrations in the registrations_ table.
    registration* next_in_table_;
    registration* prev_in_table_;

    // Link to next registration in the signal set.
    registration* next_in_set_;
  };

  // The implementation type of the signal_set.
  class implementation_type
  {
  public:
    // Default constructor.
    implementation_type()
      : signals_(0)
    {
    }

  private:
    // Only this service will have access to the internal values.
    friend class signal_set_service;

    // The pending signal handlers.
    op_queue<signal_op> queue_;

    // Linked list of registered signals.
    registration* signals_;
  };

  // Constructor.
  ASIO_DECL signal_set_service(execution_context& context);

  // Destructor.
  ASIO_DECL ~signal_set_service();

  // Destroy all user-defined handler objects owned by the service.
  ASIO_DECL void shutdown();

  // Perform fork-related housekeeping.
  ASIO_DECL void notify_fork(
      asio::execution_context::fork_event fork_ev);

  // Construct a new signal_set implementation.
  ASIO_DECL void construct(implementation_type& impl);

  // Destroy a signal_set implementation.
  ASIO_DECL void destroy(implementation_type& impl);

  // Add a signal to a signal_set.
  asio::error_code add(implementation_type& impl,
      int signal_number, asio::error_code& ec)
  {
    return add(impl, signal_number, signal_set_base::flags::dont_care, ec);
  }

  // Add a signal to a signal_set with the specified flags.
  ASIO_DECL asio::error_code add(implementation_type& impl,
      int signal_number, signal_set_base::flags_t f,
      asio::error_code& ec);

  // Remove a signal to a signal_set.
  ASIO_DECL asio::error_code remove(implementation_type& impl,
      int signal_number, asio::error_code& ec);

  // Remove all signals from a signal_set.
  ASIO_DECL asio::error_code clear(implementation_type& impl,
      asio::error_code& ec);

  // Cancel all operations associated with the signal set.
  ASIO_DECL asio::error_code cancel(implementation_type& impl,
      asio::error_code& ec);

  // Cancel a specific operation associated with the signal set.
  ASIO_DECL void cancel_ops_by_key(implementation_type& impl,
      void* cancellation_key);

  // Start an asynchronous operation to wait for a signal to be delivered.
  template <typename Handler, typename IoExecutor>
  void async_wait(implementation_type& impl,
      Handler& handler, const IoExecutor& io_ex)
  {
    associated_cancellation_slot_t<Handler> slot
      = asio::get_associated_cancellation_slot(handler);

    // Allocate and construct an operation to wrap the handler.
    typedef signal_handler<Handler, IoExecutor> op;
    typename op::ptr p = { asio::detail::addressof(handler),
      op::ptr::allocate(handler), 0 };
    p.p = new (p.v) op(handler, io_ex);

    // Optionally register for per-operation cancellation.
    if (slot.is_connected())
    {
      p.p->cancellation_key_ =
        &slot.template emplace<signal_op_cancellation>(this, &impl);
    }

    ASIO_HANDLER_CREATION((scheduler_.context(),
          *p.p, "signal_set", &impl, 0, "async_wait"));

    start_wait_op(impl, p.p);
    p.v = p.p = 0;
  }

  // Deliver notification that a particular signal occurred.
  ASIO_DECL static void deliver_signal(int signal_number);

private:
  // Helper function to add a service to the global signal state.
  ASIO_DECL static void add_service(signal_set_service* service);

  // Helper function to remove a service from the global signal state.
  ASIO_DECL static void remove_service(signal_set_service* service);

  // Helper function to create the pipe descriptors.
  ASIO_DECL static void open_descriptors();

  // Helper function to close the pipe descriptors.
  ASIO_DECL static void close_descriptors();

  // Helper function to start a wait operation.
  ASIO_DECL void start_wait_op(implementation_type& impl, signal_op* op);

  // Helper class used to implement per-operation cancellation
  class signal_op_cancellation
  {
  public:
    signal_op_cancellation(signal_set_service* s, implementation_type* i)
      : service_(s),
        implementation_(i)
    {
    }

    void operator()(cancellation_type_t type)
    {
      if (!!(type &
            (cancellation_type::terminal
              | cancellation_type::partial
              | cancellation_type::total)))
      {
        service_->cancel_ops_by_key(*implementation_, this);
      }
    }

  private:
    signal_set_service* service_;
    implementation_type* implementation_;
  };

  // The scheduler used for dispatching handlers.
#if defined(ASIO_HAS_IOCP)
  typedef class win_iocp_io_context scheduler_impl;
#else
  typedef class scheduler scheduler_impl;
#endif
  scheduler_impl& scheduler_;

#if !defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_RUNTIME) \
  && !defined(__CYGWIN__)
  // The type used for processing pipe readiness notifications.
  class pipe_read_op;

# if defined(ASIO_HAS_IO_URING_AS_DEFAULT)
  // The io_uring service used for waiting for pipe readiness.
  io_uring_service& io_uring_service_;

  // The per I/O object data used for the pipe.
  io_uring_service::per_io_object_data io_object_data_;
# else // defined(ASIO_HAS_IO_URING_AS_DEFAULT)
  // The reactor used for waiting for pipe readiness.
  reactor& reactor_;

  // The per-descriptor reactor data used for the pipe.
  reactor::per_descriptor_data reactor_data_;
# endif // defined(ASIO_HAS_IO_URING_AS_DEFAULT)
#endif // !defined(ASIO_WINDOWS)
       //   && !defined(ASIO_WINDOWS_RUNTIME)
       //   && !defined(__CYGWIN__)

  // A mapping from signal number to the registered signal sets.
  registration* registrations_[max_signal_number];

  // Pointers to adjacent services in linked list.
  signal_set_service* next_;
  signal_set_service* prev_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.h"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/signal_set_service.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_SIGNAL_SET_SERVICE_HPP