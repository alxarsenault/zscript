//
// append.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2024 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_APPEND_HPP
#define ASIO_APPEND_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.h"
#include <tuple>
#include "asio/detail/type_traits.h"

#include "asio/detail/push_options.h"

namespace asio {

/// Completion token type used to specify that the completion handler
/// arguments should be passed additional values after the results of the
/// operation.
template <typename CompletionToken, typename... Values>
class append_t
{
public:
  /// Constructor.
  template <typename T, typename... V>
  constexpr explicit append_t(T&& completion_token, V&&... values)
    : token_(static_cast<T&&>(completion_token)),
      values_(static_cast<V&&>(values)...)
  {
  }

//private:
  CompletionToken token_;
  std::tuple<Values...> values_;
};

/// Completion token type used to specify that the completion handler
/// arguments should be passed additional values after the results of the
/// operation.
template <typename CompletionToken, typename... Values>
ASIO_NODISCARD inline constexpr
append_t<decay_t<CompletionToken>, decay_t<Values>...>
append(CompletionToken&& completion_token, Values&&... values)
{
  return append_t<decay_t<CompletionToken>, decay_t<Values>...>(
      static_cast<CompletionToken&&>(completion_token),
      static_cast<Values&&>(values)...);
}

} // namespace asio

#include "asio/detail/pop_options.h"

#include "asio/impl/append.h"

#endif // ASIO_APPEND_HPP