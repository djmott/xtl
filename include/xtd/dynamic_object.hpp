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

namespace xtd{

  struct dynamic_object{

    template <typename _Ty> bool has() const{
      auto oItem = _map.find(typeid(_Ty).hash_code());
      return (_map.end() != oItem);
    }

    template <typename _Ty> void remove(){
      _map.erase(typeid(_Ty).hash_code());
    }


    template <typename _Ty, size_t> struct property_type{
      using value_type = _Ty;
    };


    template <typename _Ty> typename _Ty::value_type& property(){
      auto oItem = _map.find(typeid(_Ty).hash_code());
      if (_map.end() != oItem){
        return oItem->second.as<typename _Ty::value_type>();
      }
      var oRet = typename _Ty::value_type();
      auto oTmp = _map.emplace(typeid(_Ty).hash_code(), oRet);
      XTD_ASSERT(oTmp.second);
      return oTmp.first->second.as<typename _Ty::value_type>();
    }
    template <typename _Ty> const typename _Ty::value_type& property() const {
      auto oItem = _map.find(typeid(_Ty).hash_code());
      if (_map.end() != oItem) {
        return oItem->second.as<typename _Ty::value_type>();
      }
      throw std::runtime_error("Attempt to modify a const object");
    }
      template <typename ... _ArgTs> dynamic_object(_ArgTs&&...oArgs) : _map(std::forward<_ArgTs>(oArgs)...){}

    private:

      using map_type = std::map<size_t, xtd::var>;
      using pair = std::pair<size_t, xtd::var>;
      map_type _map;

  };


}
