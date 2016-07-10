/** @file
concurrently insert, query and delete items in an unordered hash map
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

  namespace concurrent{
  #if !defined(DOXY_INVOKED)
    namespace _{
      template <typename _HashMapT> struct hash_map_iterator_begin;
      template <typename _HashMapT> struct hash_map_iterator_end;
    }
  #endif


    template <typename _HashMapT> class hash_map_iterator;

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

      bool insert(const key_type& Key, value_type&& Value){
        int Index = (Key & 0xf);
        auto pChild = _Buckets[Index].load();
        if (!pChild){
          pChild = new child_bucket_type;
          child_bucket_type * pNullBucket = nullptr;
          if (!_Buckets[Index].compare_exchange_strong(pNullBucket, pChild)){
            delete pChild;
          }
        }
        return pChild->insert(Key >> 4, std::forward<value_type>(Value));
      }

      bool remove(const key_type& Key){
        int Index = (Key & 0xf);
        auto pChild = _Buckets[Index].load();
        if (pChild){
          return pChild->remove(Key >> 4);
        }
        return false;
      }

      value_type & operator[](const key_type& Key){
        int Index = (Key & 0xf);
        auto pChild = _Buckets[Index].load();
        if (!pChild){
          insert(Key, value_type());
          pChild = _Buckets[Index].load();
        }
        return pChild->operator[](Key >> 4);
      }


      iterator_type begin() const{ return hash_map_iterator<_my_t>::begin(*this); }
      iterator_type end() const{ return hash_map_iterator<_my_t>::end(*this); }
    private:
      template <typename> friend class hash_map_iterator;
      static const uint32_t nibble_count = 16;
      std::atomic<child_bucket_type*> _Buckets[nibble_count];

    };


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
        int Index = (Key & 0xf);
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
        int Index = (Key & 0xf);
        auto pVal = _Values[Index].load();
        value_type * pNullValue = nullptr;
        if (pVal && _Values[Index].compare_exchange_strong(pVal, pNullValue)){
          delete pVal;
          return true;
        }
        return false;
      }

      value_type & operator[](const key_type& Key){
        int Index = (Key & 0xf);
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
      template <typename> friend class hash_map_iterator;
      static const int nibble_pos = 1;
      static const uint32_t nibble_count = 16;
      std::atomic<value_type*> _Values[nibble_count];

    };


    template <typename _HashMapT>
    class hash_map_iterator{
    public:

      using hash_map_type = _HashMapT;
      using value_type = typename hash_map_type::value_type;
      using key_type = typename hash_map_type::key_type;

      bool operator==(const hash_map_iterator& rhs) const{
        if (_Ptr == rhs._Ptr) return true;
        for (int i = 0; i < index_count; ++i){
          if (_ItemIndexes[i] != rhs._ItemIndexes[i]){
            return false;
          }
        }
        return true;
      }

      bool operator != (const hash_map_iterator& rhs) const{
        return !(*this == rhs);
      }

      value_type * get(){ return _Ptr; }
      const value_type * get() const{ return _Ptr; }

      value_type * operator->(){ return _Ptr; }
      const value_type * operator->() const{ return _Ptr; }

      value_type& operator *(){ return *_Ptr; }
      const value_type& operator *() const { return *_Ptr; }



      hash_map_iterator& operator++(){
        next();
        return *this;
      }

      hash_map_iterator operator++(int){
        hash_map_iterator oRet(*this);
        next();
        return oRet;
      }

      hash_map_iterator& operator--(){
        prev();
        return *this;
      }

      hash_map_iterator operator--(int){
        hash_map_iterator oRet(*this);
        prev();
        return oRet;
      }

    protected:
      template <typename, typename, int> friend class hash_map;
      static const int index_count = sizeof(key_type) * 2;
      static const int nibble_count = 16;
      uint8_t _ItemIndexes[index_count];

      const hash_map_type& _HashMap;
      value_type * _Ptr;

      hash_map_iterator(const hash_map_type& oHashMap) : _HashMap(oHashMap), _Ptr(nullptr){}

      static value_type * _begin(const hash_map_type& oHashMap, const std::atomic<value_type*> * pValues, uint8_t * pItemIndexes){
        value_type * pItem;
        for (*pItemIndexes = 0; *pItemIndexes < nibble_count; ++*pItemIndexes){
          if (pItem = pValues[*pItemIndexes].load()){
            return pItem;
          }
        }
        return nullptr;
      }

      template <typename _BucketT>
      static value_type * _begin(const hash_map_type& oHashMap, typename std::enable_if<_BucketT::nibble_pos >=2, const _BucketT&>::type oBucket, uint8_t * pItemIndexes){
        using child_bucket_type = typename _BucketT::child_bucket_type;
        const child_bucket_type * pChildBucket;
        for (*pItemIndexes = 0; *pItemIndexes < nibble_count; ++*pItemIndexes){
          if (pChildBucket = oBucket._Buckets[*pItemIndexes].load()){
            return _begin(oHashMap, pChildBucket, ++pItemIndexes);
          }
        }
        return nullptr;
      }

      static hash_map_iterator begin(const hash_map_type& oHashMap){
        hash_map_iterator oRet(oHashMap);
        TODO("implement me")
        //oRet._Ptr = _begin(oHashMap, oHashMap._Buckets, oRet._ItemIndexes);
        return oRet;
      }

      static hash_map_iterator end(const hash_map_type& oHashMap){ return hash_map_iterator(oHashMap); }

      void next(){}
      void prev(){}
    };


  }
  ///@}
}

