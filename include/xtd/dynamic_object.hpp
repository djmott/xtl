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

namespace xtd{
  class dynamic_object : std::map<size_t, xtd::var>{
    using _super_t = std::map<size_t, xtd::var>;
    using pair = std::pair<size_t, xtd::var>;
  public:
    
    using vector = std::vector<dynamic_object>;

    template <typename ... _ArgTs> dynamic_object(_ArgTs&&...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}

    template <typename _Ty> _Ty& item(){
      auto oItem = _super_t::find(typeid(_Ty).hash_code());
      if (_super_t::end() != oItem){
        return oItem->second.as<_Ty>();
      }
      var oRet = _Ty();
      _super_t::emplace(typeid(_Ty).hash_code(), oRet);
      return _super_t::operator[](typeid(_Ty).hash_code()).as<_Ty>();
    }
    template <typename _Ty> const _Ty& item() const {
      auto oItem = _super_t::find(typeid(_Ty).hash_code());
      if (_super_t::end() != oItem){
        return oItem->second.as<_Ty>();
      }
      throw std::runtime_error("Attempt to modify a const object");
    }

  };


}