/**@file
transport neutral light weight IPC/RPC library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <cassert>
#include <vector>
#include <xtd/concurrent/hash_map.hpp>

#if(XTD_OS_WINDOWS & XTD_OS)
  #include <xtd/windows/pipe.hpp>
#endif

namespace xtd{
  namespace rpc {
    template <typename, typename> struct rpc_call;

    template <typename, typename...> struct rpc_server;
    template <typename, typename...> struct rpc_client;

    namespace _ {
      /*
       * invoker
       */
      template <typename _function_t, typename _return_t, typename ... _fnarg_ts> struct invoker;

      template <typename _function_t, typename _return_t> struct invoker<_function_t, _return_t> {

        template <typename ... _arg_ts>
        static bool invoke(_function_t& oFN, payload& oPayload, _arg_ts&&...oArgs) {
          _return_t oRet = oFN(std::forward<_arg_ts>(oArgs)...);
          oPayload = payload();
          marshaler<false, size_t>::marshal(oPayload, typeid(typename _function_t::impl_type).hash_code());
          marshaler<false, _return_t>::marshal(oPayload, oRet);
          return true;
        }
      };

      template <typename _function_t, typename _return_t, typename _head_t, typename ... _tail_ts>
      struct invoker<_function_t, _return_t, _head_t, _tail_ts...> {

        template <typename ... _arg_ts>
        static bool invoke(_function_t& oFN, payload& oPayload, _arg_ts&&...oArgs) {
          typename std::remove_reference<_head_t>::type oHead;
          marshaler<false, _head_t>::unmarshal(oPayload, oHead);
          return invoker<_function_t, _return_t, _tail_ts...>::template invoke(oFN, oPayload, std::forward<_arg_ts>(oArgs)..., oHead);
        }
      };
    }



  }
}
