/** @file
STL-ish map using on disk b-tree
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <deque>
#include <cassert>

#include <xtd/mapped_file.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/debug.hpp>

#define _DEBUG_ME_ 1

namespace xtd{

  namespace _{
    namespace btree{

      enum class page_type{
        file_header_page,
        leaf_page,
        branch_page,
      };

      template <size_t _PageSize>
      class data_page{
      public:

        page_type _page_type;

        using pointer = typename xtd::mapped_file<_PageSize>::template mapped_page<data_page>;

        static size_t page_size(){ return _PageSize; }

      };

      template <>
      class data_page<-1>{
      public:

        page_type _page_type;

        using pointer = typename xtd::mapped_file<-1>::template mapped_page<data_page>;

        static size_t page_size(){ return xtd::memory::page_size(); }

      };


      template <size_t _PageSize, size_t _CacheSize>
      class lru_cache : public std::deque< std::pair<size_t, typename data_page<_PageSize>::pointer> >{
        using _super_t = std::deque< std::pair<size_t, typename data_page<_PageSize>::pointer> >;
        xtd::mapped_file<_PageSize> _File;
      public:

        lru_cache(const xtd::filesystem::path& oPath) : _File(oPath){}

        template <typename _Ty>
        typename _Ty::pointer get(size_t Page){
          for (_super_t::iterator oItem = _super_t::begin(); _super_t::end() != oItem; ++oItem){
            if (std::get<0>(*oItem) == Page){
              if (oItem == _super_t::begin()){
                return oItem->second.cast<_Ty>();
              }
              data_page<_PageSize>::pointer oRet = std::get<1>(*oItem);
              _super_t::erase(oItem);
              _super_t::push_front(std::make_pair(Page, oRet));
              return oRet.cast<_Ty>();
            }
          }
          if (size() >= _CacheSize) _super_t::pop_back();
          _super_t::push_front(std::make_pair(Page, _File.get<data_page<_PageSize>>(Page)));
          return _super_t::front().second.cast<_Ty>();
        }

        template <typename _Ty>
        typename _Ty::pointer append(size_t& newpage){
          if (size() >= _CacheSize) _super_t::pop_back();
          auto pNewPage = _File.append<data_page<_PageSize>>(newpage);
          _super_t::push_front(std::make_pair(newpage, pNewPage));
          pNewPage->_page_type = _Ty::type;
          return pNewPage.cast<_Ty>();
        }

      };

      template <typename _KeyT, typename _ValueT, size_t _PageSize>
      class file_header : public data_page<_PageSize>{
      public:
        using key_type = _KeyT;
        using value_type = _ValueT;
        using pointer = typename xtd::mapped_file<_PageSize>::template mapped_page<file_header<_KeyT, _ValueT, _PageSize>>;
        static const page_type type = page_type::file_header_page;
        size_t _root_page;
        size_t _free_page;
        size_t _count;
      };


      template <typename _KeyT, typename _ValueT, size_t _PageSize>
      class leaf : public data_page<_PageSize>{
        using _super_t = data_page<_PageSize>;
      public:
        using key_type = _KeyT;
        using value_type = _ValueT;
        using pointer = typename xtd::mapped_file<_PageSize>::template mapped_page<leaf<_KeyT, _ValueT, _PageSize>>;
        static const page_type type = page_type::leaf_page;
        struct page_header{
          size_t _count;
          size_t _prev_page;
          size_t _next_page;
        };

        struct record{
          key_type _key;
          value_type _value;
        };

      #if _DEBUG_ME_
        static inline size_t records_per_page(){
          return 5;
        }
      #else
        static inline size_t records_per_page(){
          static size_t iRet = (_super_t::page_size() - sizeof(page_header) - sizeof(data_page<_PageSize>)) / sizeof(record);
          return iRet;
        }
      #endif

        bool insert(const key_type& key, const value_type& value){
          _records[_page_header._count]._key = key;
          _records[_page_header._count]._value = value;
          std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs)->bool{ return lhs._key < rhs._key; });
          _page_header._count++;
          return true;
        }

        template <typename _CacheT>
        void split(_CacheT& oCache, size_t left_page_idx, key_type& left_page_key, size_t& right_page_idx){
          auto oNewPage = oCache.append<leaf>(right_page_idx);
          D_(const leaf * pNewPage = oNewPage.get());
          left_page_key = _records[records_per_page() / 2]._key;
          for (size_t i = 1+(records_per_page() / 2); i < records_per_page(); ++i, ++oNewPage->_page_header._count, --_page_header._count){
            oNewPage->_records[oNewPage->_page_header._count] = _records[i];
          }
          _page_header._next_page = right_page_idx;
          oNewPage->_page_header._prev_page = left_page_idx;
        }

        page_header _page_header;
        record _records[1];
      };

      template <typename _KeyT, typename _ValueT, size_t _PageSize>
      class branch : public data_page<_PageSize>{
        using _super_t = data_page<_PageSize>;
        using leaf = leaf<_KeyT, _ValueT, _PageSize>;
        using data_page = data_page<_PageSize>;
      public:
        using pointer = typename xtd::mapped_file<_PageSize>::template mapped_page<branch<_KeyT, _ValueT, _PageSize>>;
        using key_type = _KeyT;
        using value_type = _ValueT;
        static const page_type type = page_type::branch_page;
        struct record{
          key_type _key;
          size_t _left;
        };
        struct page_header{
          size_t _count;
          size_t _right;
        };

        page_header _page_header;
        record _records[1];
      #if _DEBUG_ME_
        static inline size_t records_per_page(){
          return 5;
        }
      #else
        static inline size_t records_per_page(){
          static size_t iRet = ((_super_t::page_size() - sizeof(page_header) - sizeof(data_page<_PageSize>)) / sizeof(record));
          return iRet;
        }
      #endif

        template <typename _CacheT>
        inline bool insert(_CacheT& oCache, const key_type& key, const value_type& value){
          typename data_page::pointer oPage;
          for(;;){
            for (size_t i = 0; i < _page_header._count; ++i){
              if (key <= _records[i]._key){
                oPage = oCache.get<data_page>(_records[i]._left);
                if (page_type::leaf_page == oPage->_page_type){
                  auto oLeaf = oPage.cast<leaf>();
                  D_(const leaf * pLeaf = oLeaf.get());
                  if (oLeaf->_page_header._count >= leaf::records_per_page()){
                    _records[_page_header._count]._left = _records[i]._left;
                    oLeaf->split(oCache, _records[i]._left, _records[_page_header._count]._key, _records[i]._left);
                    std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs){ return lhs._key < rhs._key; });
                    _page_header._count++;
                    break;
                  }
                  return oLeaf->insert(key, value);
                } else{
                  auto oBranch = oPage.cast<branch>();
                  D_(const branch * pBranch = oBranch.get());
                  if (oBranch->_page_header._count >= branch::records_per_page()){
                    _records[_page_header._count] = _records[i];
                    oBranch->split(oCache, _records[_page_header._count]._key, _records[i]._left);
                    std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs){ return lhs._key < rhs._key; });
                    _page_header._count++;
                    break;
                  }
                  return oBranch->insert(oCache, key, value);
                }                
              }
            }
            oPage = oCache.get<data_page>(_page_header._right);
            if (page_type::leaf_page == oPage->_page_type){
              auto oLeaf = oPage.cast<leaf>();
              D_(const leaf * pLeaf = oLeaf.get());
              if (oLeaf->_page_header._count >= leaf::records_per_page()){
                _records[_page_header._count]._left = _page_header._right;
                oLeaf->split(oCache, _page_header._right, _records[_page_header._count]._key, _page_header._right);
                std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs){ return lhs._key < rhs._key; });
                _page_header._count++;
                continue;
              }
              return oLeaf->insert(key, value);              
            } else {
              auto oBranch = oPage.cast<branch>();
              D_(const branch * pBranch = oBranch.get());
              if (oBranch->_page_header._count < branch::records_per_page()){
                _records[_page_header._count]._left = _page_header._right;
                oBranch->split(oCache, _records[_page_header._count]._key, _page_header._right);
                std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs){ return lhs._key < rhs._key; });
                _page_header._count++;
                continue;
              }
              return oBranch->insert(oCache, key, value);
            }
          }
          return false;
        }
        template <typename _CacheT>
        inline void split(_CacheT& oCache, key_type& left_page_key, size_t & right_page_idx){
          auto oNewPage = oCache.append<branch>(right_page_idx);
          D_(const branch * pNewPage = oNewPage.get());
          TODO("convert to memcpy")
          for (size_t i = 1+records_per_page() / 2; i < records_per_page(); ++i, ++oNewPage->_page_header._count, --_page_header._count){
            oNewPage->_records[oNewPage->_page_header._count] = _records[i];
          }
          left_page_key = _records[records_per_page() / 2]._key;
        }

      };


    }
  }

  /** B-Tree key-value container
   * @tparam _KeyT the key type
   * @tparam _ValueT the value type
   */
  template <typename _KeyT, typename _ValueT, size_t _PageSize = -1, size_t _CacheSize = 20> class btree{
  public:
    using key_type = _KeyT;
    using value_type = _ValueT;

  private:
    _::btree::lru_cache<_PageSize, _CacheSize> _cache;
    using file_header = _::btree::file_header<_KeyT, _ValueT, _PageSize>;
    using data_page = _::btree::data_page<_PageSize>;
    using leaf = typename _::btree::leaf<_KeyT, _ValueT, _PageSize>;
    using branch = _::btree::branch<_KeyT, _ValueT, _PageSize>;
    using page_type = _::btree::page_type;

    typename file_header::pointer _file_header;
  public:

    btree(const xtd::filesystem::path& oPath) : _cache(oPath), _file_header(_cache.get<file_header>(0)){}

    /** insert a value in the container
     *
     * @param key unique key associated with the value
     * @param value value to insert
     * @return true of insert was successful, false otherwise
     */
    bool insert(const key_type& key, const value_type& value){
      //no root page
      if (0 == _file_header->_root_page){
        auto oLeaf = _cache.append<leaf>(_file_header->_root_page);
        D_(const leaf * pLeaf = oLeaf.get());
        if (oLeaf->insert(key, value)){
          _file_header->_count++;
          return true;
        }
        return false;
      }

      auto oRoot = _cache.get<data_page>(_file_header->_root_page);

      if (page_type::branch_page == oRoot->_page_type){ //root is a branch page
        //root page is full
        auto oBranch = oRoot.cast<branch>();
        D_(const branch * pBranch = oBranch.get());
        if (oBranch->_page_header._count >= branch::records_per_page()){
          size_t iNewRoot;
          auto oNewRoot = _cache.append<branch>(iNewRoot);
          oBranch->split(_cache, oNewRoot->_records[0]._key, oNewRoot->_page_header._right);
          oNewRoot->_records[0]._left = _file_header->_root_page;
          oNewRoot->_page_header._count = 1;
          _file_header->_root_page = iNewRoot;
          oBranch = oNewRoot;
        }
        // root page is not full
        if (oBranch->insert(_cache, key, value)){
          _file_header->_count++;
          return true;
        }
        return false;
      }
      // root is a leaf page
      auto oLeaf = oRoot.cast<leaf>();
      D_(const leaf * pLeaf = oLeaf.get());
      //leaf is not full
      if (oLeaf->_page_header._count < leaf::records_per_page()){
        if (oLeaf->insert(key, value)){
          _file_header->_count++;
          return true;
        }
        return false;
      }
      //leaf is full
      auto iOldRootIdx = _file_header->_root_page;
      auto oNewRoot = _cache.append<branch>(_file_header->_root_page);
      oNewRoot->_records[0]._left = iOldRootIdx;
      oLeaf->split(_cache, iOldRootIdx, oNewRoot->_records[0]._key, oNewRoot->_page_header._right);
      oNewRoot->_page_header._count = 1;
      if (oNewRoot->insert(_cache, key, value)){
        _file_header->_count++;
        return true;
      } 
      return false;
    }
  };
}