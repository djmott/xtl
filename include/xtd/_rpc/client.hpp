/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace rpc{
    /*
    * rpc_client
    */
    template <typename _transport_t> struct rpc_client < _transport_t> : _transport_t {
      template <typename ... _arg_ts> rpc_client(_arg_ts&&...oArgs) : _transport_t(std::forward<_arg_ts>(oArgs)...) {}
      template <typename _impl_t> using client_from_impl = rpc_client < _transport_t>;
    };

    template <typename _transport_t, typename _head_t, typename ... _tail_ts> 
    struct rpc_client<_transport_t, _head_t, _tail_ts...> : rpc_client<_transport_t, _tail_ts...> {
      using transport_type = _transport_t;
      using _super_t = rpc_client<_transport_t, _tail_ts...>;
      using _this_t = rpc_client<transport_type, _head_t, _tail_ts...>;
      template <typename ... _arg_ts> rpc_client(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...) {}

      template <typename _impl_t> using client_from_impl = typename std::conditional< std::is_same<_impl_t, _head_t>::value, _this_t, typename _super_t::template client_from_impl<_impl_t>>::type;

      template <typename _ty, typename ... _arg_ts> typename _ty::return_type call(_arg_ts&&...oArgs) {
        return static_cast<client_from_impl<_ty>&>(*this).template _call<typename _ty::return_type>(std::forward<_arg_ts>(oArgs)...);
      }

    protected:
      template <typename, typename...> friend struct rpc_client;

      template <typename _return_t, typename ... _arg_ts> _return_t _call(_arg_ts&&...oArgs) {
        payload oPayload;
        marshaler<false, size_t>::marshal(oPayload, typeid(_head_t).hash_code());
        marshaler<false, _arg_ts...>::marshal(oPayload, std::forward<_arg_ts>(oArgs)...);
        transport_type::transact(oPayload);
        _return_t oRet;
        if (typeid(_head_t).hash_code() != oPayload.peek<size_t>()) {
          assert(false);
          //TODO: unmarshal exception and throw
        }
        size_t icalltypeid;
        marshaler<false, size_t>::unmarshal(oPayload, icalltypeid);
        marshaler<false, _return_t&>::unmarshal(oPayload, oRet);
        marshaler<true, _arg_ts...>::unmarshal(oPayload, std::forward<_arg_ts>(oArgs)...);
        return oRet;
      }

    };

  }
}