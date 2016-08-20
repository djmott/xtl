/** @file
 * an object with dynamic properties and values.  The shape of the object is constructed at run time
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <map>
#include <vector>
#include <typeinfo>
#include <cassert>

#include <xtd/var.hpp>

namespace xtd{
  class dynamic_object{
    
    using map_type = std::map<size_t, xtd::var>;
    using pair = std::pair<size_t, xtd::var>;
    map_type _map;
  public:
    
    using vector = std::vector<dynamic_object>;

    template <typename ... _ArgTs> dynamic_object(_ArgTs&&...oArgs) : _map(std::forward<_ArgTs>(oArgs)...){}

    template <typename _Ty> _Ty& item(){
      auto oItem = _map.find(typeid(_Ty).hash_code());
      if (_map.end() != oItem){
        return oItem->second.as<_Ty>();
      }
      var oRet = _Ty();
      auto oTmp = _map.emplace(typeid(_Ty).hash_code(), oRet);
      assert(oTmp.second);
      return oTmp.first->second.as<_Ty>();
    }
    template <typename _Ty> const _Ty& item() const {
      auto oItem = _map.find(typeid(_Ty).hash_code());
      if (_map.end() != oItem){
        return oItem->second.as<_Ty>();
      }
      throw std::runtime_error("Attempt to modify a const object");
    }

    template <typename _Ty> bool has_item() const {
      auto oItem = _map.find(typeid(_Ty).hash_code());
      return (_map.end() != oItem);
    }

    template <typename _Ty> void remove_item() {
      _map.erase(typeid(_Ty).hash_code());
    }

  };


}