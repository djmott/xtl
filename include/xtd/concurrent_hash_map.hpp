/** @file
concurrently insert, query and delete items in an unordered hash map
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

  namespace concurrent{

    template <typename _KeyT, typename _ValueT> class hash_map_iterator{
      using move_fn = std::function<_ValueT*(_KeyT&)>;
      template <typename, typename, int> friend class hash_map;
      _ValueT * _Ptr;
      _KeyT _Key;
      move_fn _Next;
      move_fn _Prev;
      hash_map_iterator(_ValueT* Ptr, _KeyT Key, move_fn Next, move_fn Prev) : _Ptr(Ptr), _Key(Key), _Next(Next), _Prev(Prev){}
    public:
      using value_type = _ValueT;
      using pointer = _ValueT *;
      using const_pointer = const _ValueT *;
      using reference = _ValueT &;
      using const_reference = const _ValueT &;

      hash_map_iterator(const hash_map_iterator& src) : _Ptr(src._Ptr), _Key(src._Key), _Next(src._Next), _Prev(src._Prev){}
      hash_map_iterator& operator=(const hash_map_iterator& src){
        if (this == &src) return *this;
        _Ptr = src._Ptr;
        _Key = src._Key;
        _Prev = src._Prev;
        _Next = src._Next;
        return *this;
      }

      pointer get(){ return _Ptr; }
      const_pointer get() const{ return _Ptr; }

      pointer operator->(){ return _Ptr; }
      const_pointer operator->() const{ return _Ptr; }

      reference operator*(){ return *_Ptr; }
      const_reference operator*() const{ return *_Ptr; }

      hash_map_iterator& operator++(){
        _Ptr = _Next(_Key);
        return *this;
      }
      const hash_map_iterator operator++(int){
        hash_map_iterator oRet(*this);
        ++*this;
        return oRet;
      }
      hash_map_iterator& operator--(){
        _Ptr = _Prev(_Key);
        return *this;
      }
      const hash_map_iterator operator--(int){
        hash_map_iterator oRet(*this);
        --*this;
        return oRet;
      }

      bool operator==(const hash_map_iterator<_KeyT, _ValueT>& rhs) const{
        return (_Ptr == rhs._Ptr && _Key == rhs._Key);
      }

      bool operator !=(const hash_map_iterator<_KeyT, _ValueT>& rhs) const{
        return !(*this == rhs);
      }
    };

    template <typename _KeyT, typename _ValueT, int _NibblePos = sizeof(_KeyT) * 2> class hash_map {
    public:
      using value_type = _ValueT;
      using key_type = _KeyT;
      using child_bucket_type = hash_map<_KeyT, _ValueT, _NibblePos - 1>;
      using iterator_type = hash_map_iterator<_KeyT, _ValueT>;

      hash_map(){
        for (auto & oItem : _Buckets){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, value_type&& Value){
        int Index = (Key & 0xf);
        auto pChild = _Buckets[Index].load();
        if (!pChild) {
          pChild = new child_bucket_type;
          child_bucket_type * pNullBucket = nullptr;
          if (!_Buckets[Index].compare_exchange_strong(pNullBucket, pChild)) {
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

      iterator_type end() const { return iterator_type(nullptr, (_KeyT)-1, [](_KeyT&)->_ValueT*{ return nullptr; }, [](_KeyT&)->_ValueT*{ return nullptr; }); }


      iterator_type begin() const{
        return _begin(0);
      }

    private:
      static const uint32_t nibble_count = 16;
      std::atomic<child_bucket_type*> _Buckets[nibble_count];

      iterator_type _begin(key_type oKey) const{
        child_bucket_type * pBucket;
        for (int i = 0; i < nibble_count; ++i){
          if (pBucket = _Buckets[i].load()){
            return pBucket->_begin((oKey << 4) | i);
          }
        }
        return end();
      }

    };



    template <typename _KeyT, typename _ValueT> class hash_map<_KeyT, _ValueT, 1>{

    public:
      using value_type = _ValueT;
      using key_type = _KeyT;
      using iterator_type = hash_map_iterator<_KeyT, _ValueT>;

      
      hash_map(){
        for (auto & oItem : _Values){
          oItem.store(nullptr);
        }
      }

      bool insert(const key_type& Key, value_type&& Value){
        int Index = (Key & 0xf);
        auto pValue = _Values[Index].load();
        if (pValue) {
          return false;
        }
        pValue = new value_type(std::forward<value_type>(Value));
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

    private:
      static const uint32_t nibble_count = 16;
      std::atomic<value_type*> _Values[nibble_count];

      iterator_type _begin(key_type oKey) const{

        auto PrevFn = [&, this](_KeyT& oKey)->_ValueT*{
        };
        auto NextFn = [&, this](_KeyT& oKey)->_ValueT*{};

        value_type * pValue;
        for (int i = 0; i < nibble_count; ++i){
          if (pValue = _Values[i].load()){
            return iterator_type(pValue, (oKey << 4) | i, PrevFn, NextFn);
          }
        }
        return end();
      }

    };

  }

}

