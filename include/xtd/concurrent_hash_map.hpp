/** @file
concurrently insert, query and delete items in an unordered hash map
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

  namespace concurrent{

    template <typename _KeyT, typename _ValueT, int _NibblePos = sizeof(_KeyT) * 2> struct hash_map {
      using value_type = _ValueT;
      using key_type = _KeyT;
      using child_bucket_type = hash_map<_KeyT, _ValueT, _NibblePos-1>;
      std::atomic<child_bucket_type*> _Buckets[16];
    public:


      hash_map(){
        for (auto & oItem : _Buckets){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, const value_type& Value){
        int Index = (Key & 0xf);
        auto pChild = _Buckets[Index].load();
        if (!pChild) {
          pChild = new child_bucket_type;
          child_bucket_type * pNullBucket = nullptr;
          if (!_Buckets[Index].compare_exchange_strong(pNullBucket, pChild)) {
            delete pChild;
          }
        }
        return pChild->insert(Key >> 4, Value);
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
    };

    template <typename _KeyT, typename _ValueT> class hash_map<_KeyT, _ValueT, 1>{
      using value_type = _ValueT;
      using key_type = _KeyT;
      std::atomic<value_type*> _Values[16];
    public:

      hash_map(){
        for (auto & oItem : _Values){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, const value_type& Value){
        int Index = (Key & 0xf);
        auto pValue = _Values[Index].load();
        if (pValue) {
          return false;
        }
        pValue = new value_type(Value);
        value_type * pNullValue = nullptr;
        if (!_Values[Index].compare_exchange_strong(pNullValue, pValue)) {
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
        auto pRet = _Values[Index].load() ;
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
    };

  }

}

