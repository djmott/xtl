/** @file
* Hierarchy generation from behavioral policies
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <type_traits>

#include <xtd/dynamic_object.hpp>


namespace xtd{

  /** generic form
  @tparam _BaseT top most parent base class of the inheritance chain
  @tparam ... List of behavioral policies to combine into concrete type. The policies are successively made super classes. The super class of the final item is _BaseT
  */
  template <typename _BaseT, template <typename> class ... _PolicyList> struct decorated_object;

#if !DOXY_INVOKED
  namespace _{
    //helper class to get the interface to a behavioral policy from the policy() member
    template <template <typename> class _TargetT, typename _ThisT> struct decorated_object_get_concrete_policy;

    template <template <typename> class _TargetT, typename _BaseT, template <typename> class ... _TailT>
    struct decorated_object_get_concrete_policy<_TargetT, decorated_object<_BaseT, _TargetT, _TailT...>>{
      using type = _TargetT<decorated_object<_BaseT, _TailT...>>;
    };

    template <template <typename> class _TargetT, typename _BaseT, template <typename> class _HeadT, template <typename> class ... _TailT>
    struct decorated_object_get_concrete_policy<_TargetT, decorated_object<_BaseT, _HeadT, _TailT...>>{
      using type = typename decorated_object_get_concrete_policy<_TargetT, decorated_object<_BaseT, _TailT...>>::type;
    };

  }
#endif

  ///specialization terminates the recursive declaration of the policy chain and makes _BaseT the super class
  template <typename _BaseT> struct decorated_object<_BaseT> : _BaseT{

    using _my_t = decorated_object<_BaseT>;
    using _super_t = _BaseT;

    template <typename ... _ArgTs> typename std::result_of<_super_t(_ArgTs...)>::type operator()(_ArgTs...oArgs){
      _super_t & oSuper = static_cast<_super_t&>(*this);
      return oSuper(std::forward<_ArgTs>(oArgs)...);
    }

    template <typename ... _ArgTs> decorated_object(_ArgTs&&...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}

  };


  ///specialization that declares 1 or more behavioral policies as a subclass
  template <typename _BaseT, template <class> class _PolicyT, template <class> class ... _PolicyList>
  struct decorated_object<_BaseT, _PolicyT, _PolicyList...> : _PolicyT<decorated_object<_BaseT, _PolicyList...>>{

    using _my_t = decorated_object<_BaseT, _PolicyT, _PolicyList...>;
    using _super_t = _PolicyT<decorated_object<_BaseT, _PolicyList...>>;

    /** Gets the concrete interface of a behavioral policy at run-time
    @tparam _OtherP the behavioral policy to return
    @return a reference to the concrete, instantiated and fully defined implementation of the specified interface 
    */
    template <template <typename> class _OtherP>
    typename _::decorated_object_get_concrete_policy<_OtherP, _my_t>::type& policy(){
      using return_type = typename _::decorated_object_get_concrete_policy<_OtherP, _my_t>::type;
      return static_cast<return_type&>(*this);
    }


    ///passes parameters along to super class
    template <typename ... _Args> decorated_object(_Args&&...oArgs) : _super_t(std::forward<_Args>(oArgs)...){}

  };




  template <typename _BaseT, template <typename> class ... _PolicyList> struct decorated_functor;


  template <typename _BaseT> struct decorated_functor<_BaseT> : _BaseT{
    using _my_t = decorated_functor<_BaseT>;
    using _super_t = _BaseT;
    template <typename _ArgT> using _return_t = typename std::result_of<_super_t(_ArgT&&)>::type;

    template <typename _ArgT> _return_t<_ArgT> operator()(_ArgT&& oArgs){
      _super_t & oSuper = static_cast<_super_t&>(*this);
      return oSuper(std::forward<_ArgT>(oArgs));
    }

    template <typename ... _ArgTs> decorated_functor(_ArgTs&&...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}

  };



  ///specialization that declares 1 or more behavioral policies as a subclass
  template <typename _BaseT, template <class> class _PolicyT, template <class> class ... _PolicyList>
  struct decorated_functor<_BaseT, _PolicyT, _PolicyList...> : _PolicyT<decorated_functor<_BaseT, _PolicyList...>>{

    using _my_t = decorated_functor<_BaseT, _PolicyT, _PolicyList...>;
    using _next_t = decorated_functor<_BaseT, _PolicyList...>;
    using _super_t = _PolicyT<decorated_functor<_BaseT, _PolicyList...>>;

    template <typename _ArgT> using _next_return_t = typename _next_t::template _return_t<_ArgT>;

    template <typename _ArgT> using _return_t = typename std::result_of<_super_t(_next_return_t<_ArgT>&&)>::type;

    /** Gets the concrete interface of a behavioral policy at run-time
    @tparam _OtherP the behavioral policy to return
    @return a reference to the concrete, instantiated and fully defined implementation of the specified interface
    */
    template <template <typename> class _OtherP>
    typename _::decorated_object_get_concrete_policy<_OtherP, _my_t>::type& policy(){
      using return_type = typename _::decorated_object_get_concrete_policy<_OtherP, _my_t>::type;
      return static_cast<return_type&>(*this);
    }


    ///passes parameters along to super class
    template <typename ... _Args> decorated_functor(_Args&&...oArgs) : _super_t(std::forward<_Args>(oArgs)...){}


    template <typename _ArgT> _return_t<_ArgT> operator()(_ArgT&& oArg){
      auto & oNext = static_cast<_next_t&>(*this);
      auto & oSuper = static_cast<_super_t&>(*this);
      return oSuper(oNext(std::forward<_ArgT>(oArg)));
    }

    template <typename _ArgT> _return_t<_ArgT> operator()(_ArgT&& oArg) const{
      auto & oNext = static_cast<const _next_t&>(*this);
      auto & oSuper = static_cast<const _super_t&>(*this);
      return oSuper(oNext(std::forward<_ArgT>(oArg)));
    }

  };




}
