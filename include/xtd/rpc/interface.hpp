/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif

#define BEGIN_RPC_INTERFACE( _name ) template <template <typename> typename _transport_t> struct _name    \
  : xtd::rpc::rpc_interface< _name < _transport_t > , _transport_t>{                                      \
  using _super_t = xtd::rpc::rpc_interface< _name < _transport_t >, _transport_t>;                        \
  using interface_type = _name < _transport_t >;                                                          \
  template <typename ... _arg_ts> _name( _arg_ts&&...args )                                               \
    : _super_t(std::forward<_arg_ts>(args)... ){}                                                         \

#define END_RPC_INTERFACE() };

namespace xtd {
  namespace rpc {


/*

    template <typename _interface_t>
    struct call_reg {
      call_reg(_interface_t* pInterface) : _interface(pInterface){}
      void queue(icall*) {}
      _interface_t * _interface;
    };
*/


    template <typename _impl_t, template <typename> typename _transport_t> struct rpc_interface :_transport_t<_impl_t> {
      using _super_t = _transport_t<_impl_t>;
      template <typename ... _arg_ts> rpc_interface(_arg_ts&&...args) : _super_t(std::forward<_arg_ts>(args)...) {}
      rpc_interface(const rpc_interface&) = delete;

      std::vector<icall*> _calls;
    };

  }
}