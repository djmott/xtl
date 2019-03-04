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
        server(server&& src) : _impl_t(std::move(src)){
          _stop.store(false);
        }

        void start(bool async){
          auto client_handler = [this]() -> BOOL {
            payload oPayload;
            payload::size_type iLen;
            size_t CallID;
            if (!peek(iLen) || iLen < sizeof(size_t)) return false;
            oPayload.resize(iLen);
            read(oPayload);
            CallID = oPayload.unmarshal<size_t>();
            auto oCall = std::find_if(_calls.begin(), _calls.end(), [CallID](icall* pCall) { return pCall->callid() == CallID; });
            if (_calls.end() == oCall) {
              TODO("Return bad call exception");
            }
            (*oCall)->invoke(oPayload);
            return (!_stop.load());
          };
          if (async) {
            auto x = std::async(std::launch::async, [&, this]() { 
              get_client(client_handler); 
            });
            Sleep(1000);
            _service_thread = std::move(x);
          }
          else {
            get_client(client_handler);
          }
        }

        void stop(bool wait){ 
          _stop.store(true);
          _service_thread.get();
        }

      private:
        std::future<void> _service_thread;
        std::atomic<bool> _stop;
      };
    }
}