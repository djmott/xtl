/** @file
concurrently insert, query and delete items in an unordered hash map
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

  namespace _{

    template <typename _KeyT, typename _ValueT, int _NibblePos> struct hash_map_bucket {
      using value_type = _ValueT;
      using key_type = _KeyT;
      using child_bucket_type = hash_map_bucket<_KeyT, _ValueT, _NibblePos-1>;
      std::atomic<child_bucket_type*> _Buckets[16];
    public:


      hash_map_bucket(){
        for (auto & oItem : _Buckets){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, const value_type& Value){
        int Index = (Key & 0xf);
        if (!_Buckets[Index]) {
          auto pNewBucket = new child_bucket_type;
          if (!_Buckets[Index].compare_exchange_strong(nullptr, pNewBucket)) {
            delete pNewBucket;
          }
        }
        return _Buckets[Index]->Insert(Key >> 4, Value);
      }

      void remove(const key_type& Key){
        int Index = (Key & 0xf);
        if (_Buckets[Index]){
          _Buckets[Index]->remove(Key >> 4);
        }
      }
    };

    template <typename _KeyT, typename _ValueT> class hash_map_bucket<_KeyT, _ValueT, 1>{
      using value_type = _ValueT;
      using key_type = _KeyT;
      std::atomic<value_type*> _Values[16];
    public:

      hash_map_bucket(){
        for (auto & oItem : _Values){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, const value_type& Value){
        int Index = (Key & 0xf);
        if (_Values[Index]) {
          return false;
        }
        auto pNewVal = new value_type(Value);
        if (!_Values[Index].compare_exchange_strong(nullptr, pNewVal)) {
          delete pNewVal;
          return false;
        }
        return true;
      }

      void remove(const key_type& Key){
        int Index = (Key & 0xf);
        auto pVal = _Values[Index];
        if (pVal && _Values[Index].compare_exchange_strong(nullptr, pVal)){
          delete pVal;
        }
      }
    };

  }


  template <typename _KeyT, typename _ValueT> class concurrent_hash_map{
  public:
    using key_type = _KeyT;
    using value_type = _ValueT;
  protected:


    _::hash_map_bucket<_KeyT, _ValueT, sizeof(key_type)*2> _RootBucket;
  public:
    bool insert(const key_type& Key, const value_type& Value){
      return _RootBucket.insert(Key, Value);
    }
  };

}

