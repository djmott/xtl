/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif


#define RPC_CALL( _name , _sig ) \
  struct _name##_t : xtd::rpc::call< _name##_t, interface_type, _sig >{                                               \
    using _super_t = xtd::rpc::call< _name##_t, interface_type, _sig >;                                               \
    _name##_t( interface_type * const pParent) : _super_t( pParent ){}                                                \
    _name##_t () = delete;                                                                                            \
    _name##_t( _name##_t && src) : _super_t( std::move(src)){}                                                        \
    _name##_t( const _name##_t&) = delete;                                                                            \
    _name##_t & operator=(const _name##_t&) = delete;                                                                 \
    _name##_t & operator=( _name##_t && ) = delete;                                                                   \
    template <typename _fn_t> _name##_t & operator=(_fn_t fn) { _super_t::attach(fn); return *this; }};               \
    _name##_t _name = _name##_t(this);


namespace xtd {
  namespace rpc {

    struct icall{};

    template <typename _impl_t, typename _interface_t, typename _sig> struct call;


    template < typename _impl_t, typename _interface_t, typename _return_t, typename ... _arg_ts>
    struct call<_impl_t, _interface_t, _return_t(_arg_ts...)> : icall {
      using function_type = std::function<_return_t(server_context&, _arg_ts...)>;
      ~call() {
        std::remove_if(_parent->_calls.begin(), _parent->_calls.end(), [this](icall*pCall) { return this == pCall; });
      }
      explicit call(_interface_t* const pParent) : _parent(pParent) { 
        _parent->_calls.push_back(this);
      }
      call() = delete;
      call(const call&) = delete;
      call(call&& src) : _parent(src._parent){
        _parent->_calls.push_back(this);
      }
      call& operator=(call&&) = delete;
      call& operator=(const call&) = delete;

      template <typename _fn_t> void attach(_fn_t fn){ _handler = function_type(fn); }

      _return_t operator()(_arg_ts&&...args) {
        payload oPayload;
        oPayload.marshal(typeid(_impl_t).hash_code(), std::forward<_arg_ts>(args)...);
        _parent->write(oPayload);
        _parent->read(oPayload);
        auto iCallID = oPayload.unmarshal<size_t>();
        assert(typeid(_impl_t).hash_code() != iCallID);
        return oPayload.unmarshal<_return_t>();;
      }

    private:
      _interface_t * const _parent;
      function_type _handler;
    };



    template < typename _impl_t, typename _interface_t, typename ... _arg_ts>
    struct call<_impl_t, _interface_t, void(_arg_ts...)> : icall {
      using function_type = std::function<void(server_context&, _arg_ts...)>;
      explicit call(_interface_t* const pParent) : _parent(pParent) {
        _parent->_calls.push_back(this);
      }
      call() = delete;
      call(const call&) = delete;
      call(call&& src) : _parent(src._parent) {
        _parent->_calls.push_back(this);
      }
      call& operator=(call&&) = delete;
      call& operator=(const call&) = delete;

      template <typename _fn_t> void attach(_fn_t fn) { _handler = function_type(fn); }

      void operator()(_arg_ts&&...args) {
        payload oPayload;
        oPayload.marshal(typeid(_impl_t).hash_code(), std::forward<_arg_ts>(args)...);
        _parent->write(oPayload);
        _parent->read(oPayload);
        auto iCallID = oPayload.unmarshal<size_t>();
        assert(typeid(_impl_t).hash_code() != iCallID);
      }

    private:
      _interface_t * const _parent;
      function_type _handler;
    };



  }
}