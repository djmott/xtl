/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <vector>
#include <functional>
#include <cassert>

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#define __XTD_RPC_HPP_INCLUDED__

namespace xtd {
  namespace rpc {

    enum class stub {
      client,
      server,
    };
  }
}

#include "payload.hpp"
#include "server_context.hpp"
#include "call.hpp"
#include "client.hpp"
#include "server.hpp"
#include "interface.hpp"
#include "dummy_xport.hpp"
#include "named_pipe_xport.hpp"
#include "tcp_xport.hpp"

namespace xtd {
  namespace rpc {

    template <template <template <typename> typename> typename _interface_t, template <typename> typename _transport_t, typename ... _arg_ts>
    inline server<_interface_t<_transport_t>> make_server(_arg_ts&&...args) {
      return server<_interface_t<_transport_t>>(std::forward<_arg_ts>(args)...);
    }

    template <template <template <typename> typename> typename _interface_t, template <typename> typename _transport_t, typename ... _arg_ts>
    inline client<_interface_t<_transport_t>> make_client(_arg_ts&&...args) {
      return client<_interface_t<_transport_t>>(std::forward<_arg_ts>(args)...);
    }

  }
}


#endif