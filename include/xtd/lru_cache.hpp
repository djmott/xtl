/** @file
  @brief Least Recently Used (LRU) cache implementation
  
  Provides a fixed-size cache that evicts the least recently used items when
  the cache reaches its capacity limit.
  
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <deque>

TODO("Write unit tests for lru_cache")

namespace xtd{

  /** @brief Least Recently Used cache implementation
   * 
   * Implements a fixed-size cache with LRU eviction policy. When the cache
   * reaches its maximum capacity, the least recently accessed item is removed
   * before adding a new entry.
   * 
   * @tparam _key_t The key type for cache lookups
   * @tparam _value_t The value type stored in the cache
   * @tparam _cache_size Maximum number of items in the cache
   * @tparam _loader_t Function type for loading values when cache miss occurs. Defaults to function pointer.
   */
  template <typename _key_t, typename _value_t, size_t _cache_size, typename _loader_t = _value_t(*)(const _key_t&)>
  class lru_cache : protected std::deque<std::pair<_key_t, _value_t>>{
    using _super_t = std::deque<std::pair<_key_t, _value_t>>;
  public:
    using key_type = _key_t;
    using value_type = _value_t;
    using pair_type = std::pair<_key_t, _value_t>;
    using loader_type = _loader_t;

    static const size_t cache_size = _cache_size;

    /** @brief Constructs an LRU cache with a loader function
     * @param oLoader Function used to load values when cache miss occurs
     */
    explicit lru_cache(const loader_type& oLoader) : _loader(oLoader){}
    
    /** @brief Constructs an LRU cache with a moveable loader function
     * @param oLoader R-value reference to loader function
     */
    explicit lru_cache(loader_type&& oLoader) : _loader(std::move(oLoader)){}
    lru_cache() = delete;
    lru_cache(const lru_cache&) = delete;
    lru_cache(lru_cache&& src) : _super_t(std::move(src)), _loader(std::move(src._loader)){}
    ~lru_cache(){}

    /** @brief Accesses a value by key, loading if not in cache
     * 
     * If the key exists in the cache, it is moved to the front (most recently used).
     * If the key is not found, it is loaded using the loader function and added to
     * the cache. If the cache is full, the least recently used item is evicted.
     * 
     * @param key The key to look up
     * @return Reference to the cached value
     * @note This operation has O(n) complexity where n is the cache size.
     *       Consider using a hash-based cache for O(1) lookup.
     */
    [[nodiscard]] _value_t& operator[](const _key_t& key){
      for (typename _super_t::iterator oItem = _super_t::begin(); _super_t::end() != oItem; ++oItem){
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
