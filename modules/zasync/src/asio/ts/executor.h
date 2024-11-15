//
// ts/executor.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TS_EXECUTOR_HPP
#define ASIO_TS_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/async_result.h"
#include "asio/associated_allocator.h"
#include "asio/execution_context.h"
#include "asio/is_executor.h"
#include "asio/associated_executor.h"
#include "asio/bind_executor.h"
#include "asio/executor_work_guard.h"
#include "asio/system_executor.h"
#include "asio/executor.h"
#include "asio/any_io_executor.h"
#include "asio/dispatch.h"
#include "asio/post.h"
#include "asio/defer.h"
#include "asio/strand.h"
#include "asio/packaged_task.h"
#include "asio/use_future.h"

#endif // ASIO_TS_EXECUTOR_HPP
