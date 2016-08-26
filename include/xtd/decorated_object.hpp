/** @file
* fun with templates
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>


#include <type_traits>

#include <xtd/dynamic_object.hpp>


namespace xtd{


    template <template <typename> class ... _PolicyList> struct decorated_object;

    namespace _{

      template <template <typename> class _TargetT, typename _ThisT> struct decorated_object_get_concrete_policy;

      template <template <typename> class _TargetT, template <typename> class ... _TailT>
      struct decorated_object_get_concrete_policy<_TargetT, decorated_object<_TargetT, _TailT...>>{
        using type = _TargetT<decorated_object<_TailT...>>;
      };

      template <template <typename> class _TargetT, template <typename> class _HeadT, template <typename> class ... _TailT> 
      struct decorated_object_get_concrete_policy<_TargetT, decorated_object<_HeadT, _TailT...>>{
        using type = typename decorated_object_get_concrete_policy<_TargetT, decorated_object<_TailT...>>::type;
      };

    }


    template <> struct decorated_object<> : xtd::dynamic_object{

      using _my_t = decorated_object<>;
      using _super_t = dynamic_object;

      template <typename ... _Args> decorated_object(_Args&&...oArgs) : _super_t(std::forward<_Args>(oArgs)...){}

    };



    template <template <class> class _PolicyT, template <class> class ... _PolicyList>
    struct decorated_object<_PolicyT, _PolicyList...> : _PolicyT<decorated_object<_PolicyList...>>{

      using _my_t = decorated_object<_PolicyT, _PolicyList...>;
      using _super_t = _PolicyT<decorated_object<_PolicyList...>>;

      template <template <typename> class _OtherP>
      typename _::decorated_object_get_concrete_policy<_OtherP, _my_t>::type& policy(){
        using return_type = typename _::decorated_object_get_concrete_policy<_OtherP, _my_t>::type;
        return static_cast<return_type&>(*this);
      }

      template <typename ... _Args> decorated_object(_Args&&...oArgs) : _super_t(std::forward<_Args>(oArgs)...){}

    };

}
