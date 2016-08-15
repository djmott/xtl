/** @file
lru cache
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <deque>

TODO("Write unit tests for lru_cache")

namespace xtd{

  template <typename _KeyT, typename _ValueT, size_t _CacheSize, typename _LoaderT = _ValueT(*)(const _KeyT&)>
  class lru_cache : protected std::deque<std::pair<_KeyT, _ValueT>>{
    using _super_t = std::deque<std::pair<_KeyT, _ValueT>>;
  public:
    using key_type = _KeyT;
    using value_type = _ValueT;
    using pair_type = std::pair<_KeyT, _ValueT>;
    using loader_type = _LoaderT;

    static const size_t cache_size = _CacheSize;

    explicit lru_cache(const loader_type& oLoader) : _loader(oLoader){}
    explicit lru_cache(loader_type&& oLoader) : _loader(std::move(oLoader)){}
    lru_cache() = delete;
    lru_cache(const lru_cache&) = delete;
    lru_cache(lru_cache&& src) : _super_t(std::move(src)), _loader(std::move(src._loader)){}
    ~lru_cache(){}

    _ValueT& operator[](const _KeyT& key){
      for (typename deque::iterator oItem = _super_t::begin(); _super_t::end() != oItem; ++oItem){
        if (key == oItem->first){
          value_type oRet = oItem->second;
          _super_t::erase(oItem);
          _super_t::push_front(std::make_pair(key, oRet));
          return _super_t::front().second;
        }
      }
      if (_super_t::size() >= cache_size){
        _super_t::pop_back();
      }
      _super_t::push_front(std::make_pair(key, _loader(key)));
      return _super_t::front().second;
    }

  protected:
    loader_type _loader;
  };
}
