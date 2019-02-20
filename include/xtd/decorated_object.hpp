/** @file
* Hierarchy generation from behavioral policies
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <type_traits>
#include <utility>

namespace xtd{

  /** generic form
  @tparam _base_t top most parent base class of the inheritance chain
  @tparam ... List of behavioral policies to combine into concrete type. The policies are successively made super classes. The super class of the final item is _base_t
  */
  template <typename _base_t, template <typename> class ... _policy_list_ts> struct decorated_object;

#if !DOXY_INVOKED
  namespace _{
    //helper class to get the interface to a behavioral policy from the policy() member
    template <template <typename> class _target_t, typename _this_t> struct decorated_object_get_concrete_policy;

    template <template <typename> class _target_t, typename _base_t, template <typename> class ... _tail_t>
    struct decorated_object_get_concrete_policy<_target_t, decorated_object<_base_t, _target_t, _tail_t...>>{
      using type = _target_t<decorated_object<_base_t, _tail_t...>>;
    };

    template <template <typename> class _target_t, typename _base_t, template <typename> class _head_t, template <typename> class ... _tail_t>
    struct decorated_object_get_concrete_policy<_target_t, decorated_object<_base_t, _head_t, _tail_t...>>{
      using type = typename decorated_object_get_concrete_policy<_target_t, decorated_object<_base_t, _tail_t...>>::type;
    };

  }
#endif

  ///specialization terminates the recursive declaration of the policy chain and makes _base_t the super class
  template <typename _base_t> struct decorated_object<_base_t> : _base_t{

    using _my_t = decorated_object<_base_t>;
    using _super_t = _base_t;

    template <typename ... _arg_ts> typename std::result_of<_super_t(_arg_ts...)>::type operator()(_arg_ts&&...oArgs){
      _super_t & oSuper = static_cast<_super_t&>(*this);
      return oSuper(std::forward<_arg_ts>(oArgs)...);
    }

    template <typename ... _arg_ts> decorated_object(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

  };


  ///specialization that declares 1 or more behavioral policies as a subclass
  template <typename _base_t, template <class> class _policy_t, template <class> class ... _policy_list_ts>
  struct decorated_object<_base_t, _policy_t, _policy_list_ts...> : _policy_t<decorated_object<_base_t, _policy_list_ts...>>{

    using _my_t = decorated_object<_base_t, _policy_t, _policy_list_ts...>;
    using _super_t = _policy_t<decorated_object<_base_t, _policy_list_ts...>>;

    /** Gets the concrete interface of a behavioral policy at run-time
    @tparam _other_t the behavioral policy to return
    @return a reference to the concrete, instantiated and fully defined implementation of the specified interface 
    */
    template <template <typename> class _other_t>
    typename _::decorated_object_get_concrete_policy<_other_t, _my_t>::type& policy(){
      using return_type = typename _::decorated_object_get_concrete_policy<_other_t, _my_t>::type;
      return static_cast<return_type&>(*this);
    }


    ///passes parameters along to super class
    template <typename ... _arg_ts> decorated_object(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

  };




  template <typename _base_t, template <typename> class ... _policy_list_ts> struct decorated_functor;


  template <typename _base_t> struct decorated_functor<_base_t> : _base_t{
    using _my_t = decorated_functor<_base_t>;
    using _super_t = _base_t;
    template <typename _ty> using _return_t = typename std::result_of<_super_t(_ty&&)>::type;

    template <typename _ty> _return_t<_ty> operator()(_ty&& oArgs){
      _super_t & oSuper = static_cast<_super_t&>(*this);
      return oSuper(std::forward<_ty>(oArgs));
    }

    template <typename ... _arg_ts> decorated_functor(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

  };



  ///specialization that declares 1 or more behavioral policies as a subclass
  template <typename _base_t, template <class> class _policy_t, template <class> class ... _policy_list_ts>
  struct decorated_functor<_base_t, _policy_t, _policy_list_ts...> : _policy_t<decorated_functor<_base_t, _policy_list_ts...>>{

    using _my_t = decorated_functor<_base_t, _policy_t, _policy_list_ts...>;
    using _next_t = decorated_functor<_base_t, _policy_list_ts...>;
    using _super_t = _policy_t<decorated_functor<_base_t, _policy_list_ts...>>;

    template <typename _ty> using _next_return_t = typename _next_t::template _return_t<_ty>;

    template <typename _ty> using _return_t = typename std::result_of<_super_t(_next_return_t<_ty>&&)>::type;

    /** Gets the concrete interface of a behavioral policy at run-time
    @tparam _other_t the behavioral policy to return
    @return a reference to the concrete, instantiated and fully defined implementation of the specified interface
    */
    template <template <typename> class _other_t>
    typename _::decorated_object_get_concrete_policy<_other_t, _my_t>::type& policy(){
      using return_type = typename _::decorated_object_get_concrete_policy<_other_t, _my_t>::type;
      return static_cast<return_type&>(*this);
    }


    ///passes parameters along to super class
    template <typename ... _arg_ts> decorated_functor(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}


    template <typename _ty> _return_t<_ty> operator()(_ty&& oArg){
      auto & oNext = static_cast<_next_t&>(*this);
      auto & oSuper = static_cast<_super_t&>(*this);
      return oSuper(oNext(std::forward<_ty>(oArg)));
    }

    template <typename _ty> _return_t<_ty> operator()(_ty&& oArg) const{
      auto & oNext = static_cast<const _next_t&>(*this);
      auto & oSuper = static_cast<const _super_t&>(*this);
      return oSuper(oNext(std::forward<_ty>(oArg)));
    }

  };




}
