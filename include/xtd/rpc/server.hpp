/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif

namespace xtd{
    namespace rpc{
      template <typename _impl_t> struct server : _impl_t {
        template <typename ... _arg_ts> server(_arg_ts&&...args) : _impl_t(std::forward<_arg_ts>(args)...){}
        server(const server&) = delete;
        server& operator=(const server&) = delete;
        server(server&& src) : _impl_t(std::move(src)){}

        void start(bool async){}

        void stop(bool wait){}
      };
    }
}