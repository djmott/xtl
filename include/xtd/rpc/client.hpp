/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif

namespace xtd{
    namespace rpc{
      template <typename _impl_t> struct client : _impl_t {
        template <typename ... _arg_ts> client(_arg_ts&&...args) : _impl_t(std::forward<_arg_ts>(args)...) {}
        client(const client&) = delete;
        client& operator=(const client&) = delete;
        client(client&& src) : _impl_t(std::move(src)) {}

        void start(bool async) {}

        void stop(bool wait) {}
      };

    }
}