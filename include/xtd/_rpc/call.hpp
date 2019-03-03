/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
    namespace rpc{
    /*
   * rpc_call
   */

    template <typename _impl_t, typename _return_t, typename ... _fnarg_ts> struct rpc_call<_impl_t, _return_t(_fnarg_ts...)> : std::function<_return_t(_fnarg_ts...)> {
      using _super_t = std::function<_return_t(_fnarg_ts...)>;
      using _my_t = rpc_call<_impl_t, _return_t(_fnarg_ts...)>;
      using return_type = _return_t;
      using impl_type = _impl_t;
      template <typename ... _arg_ts> rpc_call(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...) {}


      template <typename ... _arg_ts> void attach(_arg_ts&&...oArgs) {
        *this = _super_t(std::forward<_arg_ts>(oArgs)...);
      }


      template <typename, typename...> friend struct rpc_server;

      bool invoke(payload& oPayload) const {
        return _::invoker<_my_t, _return_t, _fnarg_ts...>::invoke(*this, oPayload);
      }
    };

    }
}