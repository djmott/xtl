/** @file
concurrently insert, query and delete items in an unordered hash map
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

  namespace concurrent{
    /** @addtogroup Concurrent
    @{*/

    /** Unsafe iterator
    iterating should be done on a constant hash_map since it's not thread-safe to use with insertion/deletion.
    @tparam _HashMapT the hash_map type associated with this iterator.
    */
    template <typename _HashMapT>
    class hash_map_iterator{
      template <typename, typename, int> friend struct hash_map;
      static const int key_nibbles = sizeof(typename _HashMapT::key_type) * 2;
      const _HashMapT * _Map;
      typename _HashMapT::value_type * _Current;
      std::vector<int8_t> _Key;
      hash_map_iterator(const _HashMapT * pMap, typename _HashMapT::value_type * pCurrent, const std::vector<int8_t>& oKey) : _Map(pMap), _Current(pCurrent), _Key(oKey){}
    public:
      using value_type = typename _HashMapT::value_type;

      hash_map_iterator(const hash_map_iterator& src) : _Map(src._Map), _Current(src._Current), _Key(src._Key){}
      hash_map_iterator(hash_map_iterator&& src) : _Map(src._Map), _Current(src._Current), _Key(std::move(src._Key)){}

      hash_map_iterator() : _Map(nullptr), _Current(nullptr), _Key(key_nibbles, -1){}

      hash_map_iterator& operator=(const hash_map_iterator& src){
        if (this == &src){
          return *this;
        }
        _Map = src._Map;
        _Current = src._Current;
        _Key = src._Key;
      }
      hash_map_iterator& operator==(hash_map_iterator&& src){
        if (this == &src){
          return *this;
        }
        _Map = src._Map;
        _Current = src._Current;
        _Key = std::move(src._Key);
      }

      bool operator==(const hash_map_iterator& rhs) const{
        return (_Current == rhs._Current);
      }

      bool operator!=(const hash_map_iterator& rhs) const{
        return (_Current != rhs._Current);
      }

      value_type * get(){ return _Current; }
      const value_type * get() const{ return _Current; }

      value_type * operator->(){ return _Current; }
      const value_type * operator->() const{ return _Current; }

      value_type& operator*(){ assert(_Current); return *_Current; }
      const value_type& operator*() const{ assert(_Current); return *_Current; }

      hash_map_iterator& operator++(){
        ++_Key.back();
        _Current = _Map->_next(&_Key[0]);
        return *this;
      }
      hash_map_iterator operator++(int){
        hash_map_iterator oRet(*this);
        ++*this;
        return oRet;
      }
      hash_map_iterator& operator--(){
        --_Key.back();
        _Current = _Map->_prev(&_Key[0]);
        return *this;
      }
      hash_map_iterator operator--(int){
        hash_map_iterator oRet(*this);
        --*this;
        return oRet;
      }

    };

    /** thread-safe key-value pair container
    insertion and removal from multiple threads is safe but invalidates iterators.
    iteration from multiple threads is also safe but should not be done while mixing insertion and removal since they invalidate iterators.
    @tparam _KeyT The key type
    @tparam _ValueT The value type
    @tparam _NibblePos don't use this parameter, leave the default alone
    */
    template <typename _KeyT, typename _ValueT, int _NibblePos = sizeof(_KeyT) * 2> class hash_map{
      using _my_t = hash_map<_KeyT, _ValueT, _NibblePos>;
      using child_bucket_type = hash_map<_KeyT, _ValueT, _NibblePos - 1>;
      static const int nibble_pos = _NibblePos;
    public:
      using value_type = _ValueT;
      using key_type = _KeyT;
      using iterator_type = hash_map_iterator<_my_t>;

      hash_map(){
        for (auto & oItem : _Buckets){
          oItem.store(nullptr);
        }
      }
      hash_map(const hash_map&) = delete;
      hash_map& operator=(const hash_map&) = delete;
      /** concurrently insert a new value associated with a key
      @param Key key to use for indexing
      @param Value the value to insert
      @returns true if insert was successful
      */
      bool insert(const key_type& Key, value_type&& Value){
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pChild = _Buckets[Index].load();
        if (!pChild){
          pChild = new child_bucket_type;
          child_bucket_type * pNullBucket = nullptr;
          if (!_Buckets[Index].compare_exchange_strong(pNullBucket, pChild)){
            delete pChild;
          }
        }
        x >>= 4;
        return pChild->insert(reinterpret_cast<key_type>(x), std::forward<value_type>(Value));
      }

      /** concurrently search for an existing key
      @param Key the key to search for
      @returns true if the item exists in the map
      */
      bool exists(const key_type& Key) const {
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pChild = _Buckets[Index].load();
        x >>= 4;
        return (pChild ? pChild->exists(reinterpret_cast<key_type>(x)) : false);
      }

      /** concurrently remove a value
      @param Key key of the item to remove
      @returns true if the item was removed
      */
      bool remove(const key_type& Key){
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pChild = _Buckets[Index].load();
        if (pChild){
          x >>= 4;
          return pChild->remove(reinterpret_cast<key_type>(x));
        }
        return false;
      }

      /** unsafe access an item by key
      If they value does not exist a default is created with the specified key
      @param Key key of the item
      @returns reference to the value
       */
      value_type & operator[](const key_type& Key){
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pChild = _Buckets[Index].load();
        if (!pChild){
          insert(Key, value_type());
          pChild = _Buckets[Index].load();
        }
        x >>= 4;
        return pChild->operator[](reinterpret_cast<key_type>(x));
      }

      /// unsafe get an iterator to the first element
      iterator_type begin() const{
        std::vector<int8_t> oKey(iterator_type::key_nibbles, -1);
        return iterator_type(this, _begin(&oKey[0]), oKey);
      }
      /// unsafe get an iterator past the last element
      iterator_type end() const{ return iterator_type(this, nullptr, std::vector<int8_t>(iterator_type::key_nibbles, -1)); }

      /// unsafe get an iterator to the last element
      iterator_type back() const{
        std::vector<int8_t> oKey(iterator_type::key_nibbles, -1);
        return iterator_type(this, _back(&oKey[0]), oKey);
      }

    private:
      template <typename> friend class hash_map_iterator;
      template <typename, typename, int> friend class hash_map;
      static const int8_t nibble_count = 16;
      std::atomic<child_bucket_type*> _Buckets[nibble_count];

      value_type * _begin(int8_t * pKey) const{
        child_bucket_type* pChildBucket;
        for (*pKey = 0; *pKey < nibble_count; ++*pKey){
          value_type * pRet;
          if ((pChildBucket = _Buckets[*pKey].load()) && (pRet = pChildBucket->_begin(1 + pKey))){
            return pRet;
          }
        }
        return nullptr;
      }
      value_type * _back(int8_t * pKey) const{
        child_bucket_type* pChildBucket;
        for (*pKey = nibble_count-1; *pKey >= 0; --*pKey){
          value_type * pRet;
          if ((pChildBucket = _Buckets[*pKey].load()) && (pRet = pChildBucket->_back(1 + pKey))){
            return pRet;
          }
        }
        return nullptr;
      }
      value_type * _next(int8_t * pKey) const{
        child_bucket_type* pChildBucket;
        if (*pKey < 0 || *pKey >= nibble_count){
          *pKey = 0;
        }
        for (; *pKey < nibble_count; ++*pKey){
          value_type * pRet;
          if ((pChildBucket = _Buckets[*pKey].load()) && (pRet = pChildBucket->_next(1 + pKey))){
            return pRet;
          }
        }
        *pKey = -1;
        return nullptr;
      }
      value_type * _prev(int8_t * pKey) const{
        child_bucket_type* pChildBucket;
        if (*pKey < 0 || *pKey >= nibble_count){
          *pKey = nibble_count - 1;
        }
        for (; *pKey >= 0; --*pKey){
          value_type * pRet;
          if ((pChildBucket = _Buckets[*pKey].load()) && (pRet = pChildBucket->_prev(1 + pKey))){
            return pRet;
          }
        }
        *pKey = -1;
        return nullptr;
      }

    };

#if (!DOXY_INVOKED)
    template <typename _KeyT, typename _ValueT> class hash_map<_KeyT, _ValueT, 1>{
    public:
      using _my_t = hash_map<_KeyT, _ValueT, 1>;
      using value_type = _ValueT;
      using key_type = _KeyT;


      hash_map(){
        for (auto & oItem : _Values){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, value_type&& Value){
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pValue = _Values[Index].load();
        if (pValue){
          return false;
        }
        pValue = new value_type(std::forward<value_type>(Value));
        value_type * pNullValue = nullptr;
        if (!_Values[Index].compare_exchange_strong(pNullValue, pValue)){
          delete pValue;
          return false;
        }
        return true;
      }

      bool remove(const key_type& Key){
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pVal = _Values[Index].load();
        value_type * pNullValue = nullptr;
        if (pVal && _Values[Index].compare_exchange_strong(pVal, pNullValue)){
          delete pVal;
          return true;
        }
        return false;
      }

      bool exists(const key_type& Key) const{
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pVal = _Values[Index].load();
        return (pVal ? true : false);
      }

      value_type & operator[](const key_type& Key){
        auto x = intrinsic_cast(Key);
        int Index = (x & 0xf);
        auto pRet = _Values[Index].load();
        if (!pRet){
          pRet = new value_type;
          value_type * pNull = nullptr;
          if (!_Values[Index].compare_exchange_strong(pNull, pRet)){
            delete pRet;
            pRet = _Values[Index].load();
          }
        }
        XTD_ASSERT(pRet);
        return *pRet;
      }

    private:
      template <typename, typename, int> friend class hash_map;
      static const int nibble_pos = 1;
      static const int8_t nibble_count = 16;
      std::atomic<value_type*> _Values[nibble_count];

      value_type * _begin(int8_t * pKey) const{
        for (*pKey = 0; *pKey < nibble_count; ++*pKey){
          value_type * pRet;
          if ((pRet = _Values[*pKey].load())){
            return pRet;
          }
        }
        return nullptr;
      }
      value_type * _back(int8_t * pKey) const{
        for (*pKey = nibble_count - 1; *pKey >= 0; --*pKey){
          value_type * pRet;
          if ((pRet = _Values[*pKey].load())){
            return pRet;
          }
        }
        return nullptr;
      }

      value_type * _next(int8_t * pKey) const{
        if (*pKey < 0 || *pKey >= nibble_count){
          *pKey = 0;
        }
        for (; *pKey < nibble_count; ++*pKey){
          value_type * pRet;
          if ((pRet = _Values[*pKey].load())){
            return pRet;
          }
        }
        *pKey = -1;
        return nullptr;
      }

      value_type * _prev(int8_t * pKey) const{
        if (*pKey < 0 || *pKey >= nibble_count){
          *pKey = nibble_count - 1;
        }
        for (; *pKey >= 0; --*pKey){
          value_type * pRet;
          if ((pRet = _Values[*pKey].load())){
            return pRet;
          }
        }
        *pKey = -1;
        return nullptr;
      }

    };
#endif

    ///@}
  }
  
}

