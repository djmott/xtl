/** @file
* an object with dynamic properties and values.  The shape of the object is constructed at run time
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <map>
#include <vector>
#include <typeinfo>

#include <xtd/var.hpp>
#include <xtd/debug.hpp>

#define PROPERTY(_type) xtd::dynamic_object::property_type<_type, __COUNTER__>

namespace xtd{

  namespace _{
    template <typename> struct isa_dynamic_object_property_type : std::false_type{}; //default to false...true specialization is after the declaration at the end of this file

  }

  struct dynamic_object : private std::map<size_t, xtd::var>{

    template <typename _Ty> bool has() const{
      auto oItem = map_type::find(typeid(_Ty).hash_code());
      return (map_type::end() != oItem);
    }

    template <typename _Ty> void remove(){
      map_type::erase(typeid(_Ty).hash_code());
    }


    template <typename _Ty, size_t> struct property_type{
      using value_type = _Ty;
    };


    template <typename _Ty> typename _Ty::value_type& value(){
      static_assert(_::isa_dynamic_object_property_type<_Ty>::value, "Invalid parameter. Expecting a dynamic_object::property_type template parameter");
      auto oItem = map_type::find(typeid(_Ty).hash_code());
      if (map_type::end() != oItem){
        return oItem->second.as<typename _Ty::value_type>();
      }
      var oRet = typename _Ty::value_type();
      auto oTmp = map_type::emplace(typeid(_Ty).hash_code(), oRet);
      XTD_ASSERT(oTmp.second);
      return oTmp.first->second.as<typename _Ty::value_type>();
    }
    template <typename _Ty> const typename _Ty::value_type& value() const{
      static_assert(_::isa_dynamic_object_property_type<_Ty>::value, "Invalid parameter. Expecting a dynamic_object::property_type template parameter");
      auto oItem = map_type::find(typeid(_Ty).hash_code());
      if (map_type::end() != oItem){
        return oItem->second.as<typename _Ty::value_type>();
      }
      throw std::runtime_error("Attempt to modify a const object");
    }


	template <typename ... _ArgTs> static dynamic_object make(_ArgTs&&...oArgs) {
		dynamic_object oRet;
		oRet.add_items(std::forward<_ArgTs>(oArgs)...);
		return oRet;
	}


    template <typename ... _ArgTs> dynamic_object(_ArgTs&&...oArgs) : map_type(std::forward<_ArgTs>(oArgs)...){}

  private:

    using map_type = std::map<size_t, xtd::var>;
    using pair = std::pair<size_t, xtd::var>;

	void add_items(){}

	template <typename _HeadT, typename ... _TailT>
	void add_items(_HeadT oHead, _TailT...oTail) {
		map_type::insert(pair(typeid(_HeadT).hash_code(), xtd::var(oHead)));
		add_items(std::forward<_TailT>(oTail)...);
	}

  };

  namespace _{
    //true specialization is after the declaration
    template <typename _Ty, size_t _ctr> struct isa_dynamic_object_property_type<dynamic_object::property_type<_Ty, _ctr>> : std::true_type{};
  }

}
