/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif

#if (XTD_OS_WINDOWS & XTD_OS)

#include <xtd/windows/pipe.hpp>

namespace xtd{
    namespace rpc{
      template <stub _stub, typename _impl_t>
      struct named_pipe_transport : xtd::windows::pipe{
        virtual ~named_pipe_transport() = default;
        using _super_t = xtd::windows::pipe;
        named_pipe_transport(named_pipe_transport&& src) : xtd::windows::pipe(std::move(src)){}
        template <typename ... _arg_ts> named_pipe_transport(_arg_ts&&...args) : _super_t(std::forward<_arg_ts>(args)...){
        }

      protected:
        template <typename _handler_fn> void get_client(_handler_fn handler_fn) {
          BOOL bRet = true;
          while (bRet) {
            TODO("throw if connect fails")
            bRet = handler_fn();
          }
          
          
        }
      };
    }
}



#endif