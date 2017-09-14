/** @file
Single producer - multiple subscriber callback
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
@example example_callback.cpp
*/

#pragma once
#include <xtd/xtd.hpp>

#include <memory>
#include <vector>


namespace xtd{
#if (!DOXY_INVOKED)
  template <typename> class callback;
#endif 
  /** Notifies multiple targets of an event

  The callbacks are performed serially within the invoking thread. Receivers can be static functions, member functions or lambdas.
  Callbacks with a return value can be invoked with a policy describing which receiver contains the return value of interest.
  The template is parameterized with a function signature that defines the invocation and attached receivers must match the signature 
  or all parameters must be implicitly convertible from the signature's parameters.

  @tparam _return_t return type
  @tparam ... parameters
  */
  template <typename _return_t, typename ... _arg_ts> class callback < _return_t(_arg_ts...) >{
    class invoker{
    public:
      using ptr = std::unique_ptr<invoker>;
      using vector = std::vector<ptr>;
      virtual ~invoker() = default;
      virtual _return_t invoke(_arg_ts...) const = 0;
    };

    template <typename _method_t, _method_t * _method> class method_invoker : public invoker{
    public:
      ~method_invoker() override = default;
      virtual _return_t invoke(_arg_ts...oArgs) const override { return (*_method)(std::forward<_arg_ts>(oArgs)...); }
    };

    template <typename _lambda_t> class lamdba_invoker : public invoker{
    public:
      ~lamdba_invoker() override = default;
      virtual _return_t invoke(_arg_ts...oArgs) const override { return _lambda(std::forward<_arg_ts>(oArgs)...); }
      explicit lamdba_invoker(_lambda_t oLambda) : _lambda(oLambda){} //NOSONAR
      lamdba_invoker(lamdba_invoker&& src) : _lambda(std::move(src._lambda)){} //NOSONAR
      lamdba_invoker(const lamdba_invoker& src) : _lambda(src._lambda){} //NOSONAR
      lamdba_invoker& operator=(const lamdba_invoker& src){
        if (this != &src){
          _lambda = src._lambda;
        }
        return *this;
      }
      lamdba_invoker& operator=(lamdba_invoker&&) = delete;
      _lambda_t _lambda;
    };

    template <typename _dest_t, _return_t(_dest_t::*_member)(_arg_ts...)> class member_invoker : public invoker{
    public:
      ~member_invoker() override = default;
      member_invoker() = delete;
      member_invoker& operator=(const member_invoker&) = delete;
      member_invoker(member_invoker&& oSrc) : _dest(oSrc._dest){}  //NOSONAR
      member_invoker(const member_invoker& oSrc) : _dest(oSrc._dest){}  //NOSONAR
      explicit member_invoker(_dest_t* dest) : _dest(dest){} //NOSONAR
      virtual _return_t invoke(_arg_ts...oArgs) const override { return (_dest->*_member)(oArgs...); }
      _dest_t * _dest;
    };

    typename invoker::vector _invokers;

  public:
    /// behavior of invocation when multiple receivers are attached 
    enum result_policy{
      return_first, ///< return the result of the first attached target
      return_last ///< return the result of the last attached target
    };
    ///{@
    callback() = default;
    callback(callback&& src) : _invokers(std::move(src._invokers)){}
    callback(const callback&) = delete;
    ///@}
    ~callback() = default;
    callback& operator=(const callback&) = delete;
    /// Invokes all the attached targets and returns the result of the target as specified by the result policy
    _return_t operator()(result_policy result, _arg_ts...oArgs) const{
      _return_t oRet;
      typename invoker::vector::size_type i = 0;
      for (const auto & oInvoker : _invokers){
        if ((result_policy::first == result && 0 == i) || (result_policy::last == result && (_invokers.size() - 1) == i)){
          oRet = oInvoker->invoke(std::forward<_arg_ts>(oArgs)...);
        } else{
          oInvoker->invoke(oArgs...);
        }
        ++i;
      }
      return oRet;
    }
    //invokes all the attached targets and returns the result of the last target
    _return_t operator()(_arg_ts...oArgs) const{
      _return_t oRet;
      for (const auto & oInvoker : _invokers){
        oRet = oInvoker->invoke(oArgs...);
      }
      return oRet;
    }

    /// connect a class instance and member function
    template <typename _class_t, _return_t(_class_t::*_member_t)(_arg_ts...)>
    void connect(_class_t *pClass){ _invokers.emplace_back(new member_invoker<_class_t, _member_t>(pClass)); }

    /// connect a lambda 
    template <typename method>
    void connect(method oMethod){ _invokers.emplace_back(new lamdba_invoker< method >(oMethod)); }

    /// connect a static method
    template <_return_t(*method)(_arg_ts...)>
    void connect(){ _invokers.emplace_back(new method_invoker<_return_t(_arg_ts...), method>()); }

    /// connect a static method or lambda 

    template <typename _ty>
    callback& operator += (_ty&& addend){ connect(std::forward<_ty>(addend)); return *this; }

  };

 
#if (!DOXY_INVOKED)
  template <typename ... _arg_ts> class callback < void(_arg_ts...) >{

    using return_type = void;

    class invoker{
    public:
      using ptr = std::unique_ptr<invoker>;
      using vector = std::vector<ptr>;
      virtual ~invoker() = default;
      virtual return_type invoke(_arg_ts...) const = 0;
    };

    template <typename _method_t, _method_t * _method> class method_invoker : public invoker{
    public:
      ~method_invoker() override = default;
      virtual return_type invoke(_arg_ts...oArgs) const override { (*_method)(std::forward<_arg_ts>(oArgs)...); }
    };

    template <typename _lambda_t> class lamdba_invoker : public invoker{
    public:
      ~lamdba_invoker() override = default;
      virtual return_type invoke(_arg_ts...oArgs) const override { _lambda(std::forward<_arg_ts>(oArgs)...); }
      explicit lamdba_invoker(_lambda_t oLambda) : _lambda(oLambda){}
      lamdba_invoker(lamdba_invoker&& src) : _lambda(std::move(src._lambda)){}
      lamdba_invoker(const lamdba_invoker& src) : _lambda(src._lambda){}
      lamdba_invoker& operator=(const lamdba_invoker& src){
        if (this != &src){
          _lambda = src._lambda;
        }
        return *this;
      }
      lamdba_invoker& operator=(lamdba_invoker&&) = delete;
      _lambda_t _lambda;
    };

    template <typename _dest_t, return_type(_dest_t::*_member)(_arg_ts...)> class member_invoker : public invoker{
    public:
      ~member_invoker() override = default;
      member_invoker() = delete;
      member_invoker& operator=(const member_invoker&) = delete;
      member_invoker(member_invoker&& oSrc) : _dest(oSrc._dest){}
      member_invoker(const member_invoker& oSrc) : _dest(oSrc._dest){}
      explicit member_invoker(_dest_t* dest) : _dest(dest){}
      virtual return_type invoke(_arg_ts...oArgs)const override { (_dest->*_member)(oArgs...); }
      _dest_t * _dest;
    };

    typename invoker::vector _invokers;

  public:
    ~callback() = default;
    callback() : _invokers(){}
    callback(const callback&) = delete;
    callback(callback&& src) : _invokers(std::move(src._invokers)){}
    callback& operator=(const callback&) = delete;


    void operator()(_arg_ts...oArgs) const{
      for (auto & oInvoker : _invokers){
        oInvoker->invoke(oArgs...);
      }
    }

    template <typename _class_t, return_type(_class_t::*_MemberT)(_arg_ts...)>
    void connect(_class_t *pClass){ _invokers.emplace_back(new member_invoker<_class_t, _MemberT>(pClass)); }

    template <typename method>
    void connect(method oMethod){ _invokers.emplace_back(new lamdba_invoker< method >(oMethod)); }

    template <return_type(*method)(_arg_ts...)>
    void connect(){ _invokers.emplace_back(new method_invoker<void(_arg_ts...), method>()); }

    template <typename _ty>
    callback& operator += (_ty&& addend){ connect(std::forward<_ty>(addend)); return *this; }


  };
#endif
}
