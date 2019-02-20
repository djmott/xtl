/** @file
* an object with dynamic properties and values.  The shape of the object is constructed at run time
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <map>

#include <xtd/var.hpp>

#define PROPERTY(_type) xtd::dynamic_object::property_type<_type, __COUNTER__>

namespace xtd{

  namespace _{
    template <typename> struct isa_dynamic_object_property_type : std::false_type{}; //default to false...true specialization is after the declaration at the end of this file

  }

  struct dynamic_object : private std::map<size_t, xtd::var>{

    template <typename _ty> bool has() const{
      auto oItem = map_type::find(typeid(_ty).hash_code());
      return (map_type::end() != oItem);
    }

    template <typename _ty> void remove(){
      map_type::erase(typeid(_ty).hash_code());
    }


    template <typename _ty, size_t> struct property_type{
      using value_type = _ty;
    };


    template <typename _ty> typename _ty::value_type& value(){
      static_assert(_::isa_dynamic_object_property_type<_ty>::value, "Invalid parameter. Expecting a dynamic_object::property_type template parameter");
      auto oItem = map_type::find(typeid(_ty).hash_code());
      if (map_type::end() != oItem){
        return oItem->second.as<typename _ty::value_type>();
      }
      var oRet = typename _ty::value_type();
      auto oTmp = map_type::emplace(typeid(_ty).hash_code(), oRet);
      XTD_ASSERT(oTmp.second);
      return oTmp.first->second.as<typename _ty::value_type>();
    }
    template <typename _ty> const typename _ty::value_type& value() const{
      static_assert(_::isa_dynamic_object_property_type<_ty>::value, "Invalid parameter. Expecting a dynamic_object::property_type template parameter");
      auto oItem = map_type::find(typeid(_ty).hash_code());
      if (map_type::end() != oItem){
        return oItem->second.as<typename _ty::value_type>();
      }
      throw std::runtime_error("Attempt to modify a const object");
    }


	template <typename ... _arg_ts> static dynamic_object make(_arg_ts&&...oArgs) {
		dynamic_object oRet;
		oRet.add_items(std::forward<_arg_ts>(oArgs)...);
		return oRet;
	}


  template <typename ... _arg_ts> dynamic_object(_arg_ts&&...oArgs) : map_type(std::forward<_arg_ts>(oArgs)...){}

  private:

    using map_type = std::map<size_t, xtd::var>;
    using pair = std::pair<size_t, xtd::var>;

	void add_items(){}

	template <typename _head_t, typename ... _tail_t>
	void add_items(_head_t oHead, _tail_t...oTail) {
		map_type::insert(pair(typeid(_head_t).hash_code(), xtd::var(oHead)));
		add_items(std::forward<_tail_t>(oTail)...);
	}

  };

  namespace _{
    //true specialization is after the declaration
    template <typename _ty, size_t _ctr> struct isa_dynamic_object_property_type<dynamic_object::property_type<_ty, _ctr>> : std::true_type{};
  }

}
