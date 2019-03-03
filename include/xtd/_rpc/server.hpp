/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
    namespace rpc{
    /*
    * rpc_server
    */

    template <typename _transport_t> struct rpc_server < _transport_t> : _transport_t {
      template <typename _impl_t> using server_from_impl = rpc_server < _transport_t>;

      rpc_server(const rpc_server&) = delete;
      template <typename ... _arg_ts> rpc_server(_arg_ts&&...oArgs) : _transport_t(std::forward<_arg_ts>(oArgs)...) {}

      bool invoke(payload& oPayload) const {
        return false;
      }
    };

    template <typename _transport_t, typename _head_t, typename ... _tail_ts> struct rpc_server<_transport_t, _head_t, _tail_ts...> 
      : rpc_server<_transport_t, _tail_ts...> {
      using transport_type = _transport_t;
      using _this_t = rpc_server<_transport_t, _head_t, _tail_ts...>;
      using _super_t = rpc_server<_transport_t, _tail_ts...>;
      using client_type = rpc_client<transport_type, _head_t, _tail_ts...>;
      using call_type = _head_t;
      template <typename _impl_t> using server_from_impl = typename std::conditional< std::is_same<_impl_t, _head_t>::value, _this_t, typename _super_t::template server_from_impl<_impl_t>>::type;

      rpc_server(const rpc_server&) = delete;
      template <typename ... _arg_ts> rpc_server(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...) {}

      template <typename ... _arg_ts> void attach(_arg_ts&&...oArgs) {
        _call.attach(std::forward<_arg_ts>(oArgs)...);
      }

      template <typename _impl_t> server_from_impl<_impl_t>& get() { return static_cast<server_from_impl<_impl_t>&>(*this); }

      void start_server() { transport_type::template start_server<_this_t>(*this); }
      void stop_server() { transport_type::stop_server(); }

      friend struct pipe_transport;
      friend struct pipe_transport;
      call_type _call;

      bool invoke(payload& oPayload) const {
        if (typeid(_head_t).hash_code() != oPayload.peek<size_t>()) return _super_t::invoke(oPayload);
        size_t iCallID;
        marshaler<false, size_t>::unmarshal(oPayload, iCallID);
        return _call.invoke(oPayload);
      }
    };

    }
}