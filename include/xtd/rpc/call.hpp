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

    namespace _ {
      template <size_t argc, typename _return_t, typename ... _fnarg_ts> struct call_handler;
    }

    struct icall{
      using vector = std::vector<icall*>;
      virtual ~icall() = default;
      virtual size_t callid() const = 0;
      virtual void invoke(payload&) const = 0;
    };

    template <typename _impl_t, typename _interface_t, typename _sig> struct call;

    template < typename _impl_t, typename _interface_t, typename _return_t, typename ... _arg_ts>
    struct call<_impl_t, _interface_t, _return_t(_arg_ts...)> : icall {
      using function_type = std::function<_return_t(_arg_ts...)>;
      ~call() { _parent->remove_call(this); }
      explicit call(_interface_t* const pParent) : _parent(pParent) { _parent->add_call(this); }
      call() = delete;
      call(const call&) = delete;
      call(call&& src) : _parent(src._parent) { _parent->add_call(this); }
      call& operator=(call&&) = delete;
      call& operator=(const call&) = delete;

      template <typename _fn_t> void attach(_fn_t fn){ _handler = function_type(fn); }

      _return_t operator()(_arg_ts&&...args) {
        payload oPayload;
        oPayload.marshal(std::forward<_arg_ts>(args)..., typeid(_impl_t).hash_code());
        _parent->write(oPayload);
        _parent->read(oPayload);
        auto iCallID = oPayload.unmarshal<size_t>();
        assert(typeid(_impl_t).hash_code() != iCallID);
        return oPayload.unmarshal<_return_t>();;
      }

      size_t callid() const override { return typeid(_impl_t).hash_code(); }

      void invoke(payload& oPayload) const override {
        oPayload.marshal(_::call_handler<sizeof...(_arg_ts)-1, _return_t, _arg_ts...>::invoke(oPayload, _handler));
      }

    private:
      _interface_t * const _parent;
      function_type _handler;
    };


    //void() specialization
    template < typename _impl_t, typename _interface_t, typename ... _arg_ts>
    struct call<_impl_t, _interface_t, void(_arg_ts...)> : icall {
      using function_type = std::function<void(_arg_ts...)>;
      ~call() { _parent->remove_call(this); }
      explicit call(_interface_t* const pParent) : _parent(pParent) { _parent->add_call(this); }
      call() = delete;
      call(const call&) = delete;
      call(call&& src) : _parent(src._parent) { _parent->add_call(this); }
      call& operator=(call&&) = delete;
      call& operator=(const call&) = delete;

      template <typename _fn_t> void attach(_fn_t fn) { _handler = function_type(fn); }

      void operator()(_arg_ts&&...args) {
        payload oPayload;
        oPayload.marshal(std::forward<_arg_ts>(args)..., typeid(_impl_t).hash_code());
        _parent->write(oPayload);
        _parent->read(oPayload);
        auto iCallID = oPayload.unmarshal<size_t>();
        assert(typeid(_impl_t).hash_code() != iCallID);
      }

      size_t callid() const override { return typeid(_impl_t).hash_code(); }

      void invoke(payload& oPayload) const override {
        _::call_handler<sizeof...(_arg_ts)-1, void, _arg_ts...>::invoke(oPayload, _handler);
      }

    private:
      _interface_t * const _parent;
      function_type _handler;
    };


    namespace _ {

      template <size_t, typename ...> struct argn;

      template <typename _head_t, typename ... _tail_t> struct argn<0, _head_t, _tail_t...> {
        using type = _head_t;
      };

      template <size_t N, typename _head_t, typename ... _tail_t> struct argn<N, _head_t, _tail_t...> {
        using type = typename argn<N - 1, _tail_t...>::type;
      };

      template <typename _return_t, typename ... _arg_ts> struct call_handler<0, _return_t, _arg_ts...> {
        template <typename ... _invoke_args>
        static _return_t invoke(payload& oPayload, const std::function<_return_t(_arg_ts...)>& oFN, _invoke_args&&...args) {
          return oFN(oPayload.unmarshal< typename argn<0,_arg_ts...>::type >(), std::forward<_invoke_args>(args)...);
        }
      };

      template <size_t argc, typename _return_t, typename ... _arg_ts> struct call_handler{
        template <typename ... _invoke_args>
        static _return_t invoke(payload& oPayload, const std::function<_return_t(_arg_ts...)>& oFN, _invoke_args&&...args) {
          return call_handler<argc - 1, _return_t, _arg_ts...>::invoke(oPayload, oFN, 
            oPayload.unmarshal<typename argn<argc, _arg_ts...>::type>(), std::forward<_invoke_args>(args)...);
        }
      };

      //void specialization

      template <typename ... _arg_ts> struct call_handler<0, void, _arg_ts...> {
        template <typename ... _invoke_args>
        static void invoke(payload& oPayload, const std::function<void(_arg_ts...)>& oFN, _invoke_args&&...args) {
          oFN(oPayload.unmarshal< typename argn<0, _arg_ts...>::type >(), std::forward<_invoke_args>(args)...);
        }
      };

      template <size_t argc, typename ... _arg_ts> struct call_handler<argc, void, _arg_ts...> {
        template <typename ... _invoke_args>
        static void invoke(payload& oPayload, const std::function<void(_arg_ts...)>& oFN, _invoke_args&&...args) {
          call_handler<argc - 1, void, _arg_ts...>::invoke(oPayload, oFN,
            oPayload.unmarshal<typename argn<argc, _arg_ts...>::type>(), std::forward<_invoke_args>(args)...);
        }
      };

    }



  }
}