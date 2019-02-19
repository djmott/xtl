/** @file
STL-ish map using on disk b-tree
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <xtd/lru_cache.hpp>
#include <xtd/mapped_file.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/debug.hpp>

#define __DEBUG_ME__ 1

namespace xtd{

  namespace _{
    namespace btree{

      enum class page_type{
        file_header_page,
        leaf_page,
        branch_page,
      };

      /** base class of btree pages
      @tparam _page_size size of the page to use or -1 to use the system default
      */
      template <size_t _page_size>
      class data_page{
      public:

        page_type _page_type;

        using pointer = mapped_page<data_page>;

        static size_t page_size(){ return _page_size; }

      };

      /** specialized data page, uses the system default page size
      */
      template <>
      class data_page<-1>{
      public:

        page_type _page_type;

        using pointer = mapped_page<data_page>;

        static size_t page_size(){ return xtd::memory::page_size(); }

      };

      template <size_t _page_size, size_t _cache_size> class lru_cache;

      template <size_t _page_size>
      class page_loader{
        template <size_t, size_t> friend class lru_cache;
        xtd::mapped_file<_page_size> _file;
      public:

        page_loader(const xtd::filesystem::path& oPath) : _file(oPath){}
        page_loader(const page_loader&) = delete;
        page_loader(page_loader&& src) : _file(std::move(src._file)){}

        typename data_page<_page_size>::pointer operator()(size_t pagenum){
          return _file.template get<data_page<_page_size>>(pagenum);
        }
      };

      template <size_t _page_size, size_t _cache_size>
      class lru_cache : public xtd::lru_cache<size_t, typename data_page<_page_size>::pointer, _cache_size, page_loader<_page_size> >{
        using _super_t = xtd::lru_cache<size_t, typename data_page<_page_size>::pointer, _cache_size, page_loader<_page_size> >;
      public:
        using value_type = typename data_page<_page_size>::pointer;
        static const size_t cache_size = _cache_size;
        explicit lru_cache(const xtd::filesystem::path& oPath) : _super_t(page_loader<_page_size>(oPath)) {}

        typename data_page<_page_size>::pointer append(size_t & newpage){
          if (_super_t::size() >= cache_size){
            _super_t::pop_back();
          }
          _super_t::push_front(std::make_pair(newpage, _super_t::_loader._file.template append<data_page<_page_size>>(newpage)));
          return _super_t::front().second;
        }
      };

      /** btree file header present only on the first page of the file
      @tparam _page_size size of btree page
      */
      template <size_t _page_size>
      class file_header : public data_page<_page_size>{
      public:
        using pointer = mapped_page<file_header>;
        static const page_type type = page_type::file_header_page;
        size_t _root_page;
        size_t _free_page;
        size_t _count;
      };

      /** leaf page contains all the keys and values
      @tparam _key_t key type
      @tparam _value_t value type
      @tparam _page_size size of the page
      */
      template <typename _key_t, typename _value_t, size_t _page_size>
      class leaf : public data_page<_page_size>{
        using _super_t = data_page<_page_size>;
      public:
        using key_type = _key_t;
        using value_type = _value_t;
        using pointer = mapped_page<leaf<_key_t, _value_t, _page_size>>;
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

      #if __DEBUG_ME__
        static inline size_t records_per_page(){
          return 5;
        }
      #else
        static inline size_t records_per_page(){
          static size_t iRet = (_super_t::page_size() - sizeof(page_header) - sizeof(data_page<_page_size>)) / sizeof(record);
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

        template <typename _cache_t>
        void split(_cache_t& oCache, size_t left_page_idx, key_type& left_page_key, size_t& right_page_idx){
          auto oNewPage = static_page_cast<leaf>(oCache[right_page_idx]);
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

      /** branch page contains keys and indexes to more branch or leaf pages
      @tparam _key_t key type
      @tparam _value_t value type
      @tparam _page_size size of the page
      */
      template <typename _key_t, typename _value_t, size_t _page_size>
      class branch : public data_page<_page_size>{
        using _super_t = data_page<_page_size>;
        using leaf_t = leaf<_key_t, _value_t, _page_size>;
      public:
        using pointer = mapped_page<branch<_key_t, _value_t, _page_size>>;
        using key_type = _key_t;
        using value_type = _value_t;
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
      #if __DEBUG_ME__
        static inline size_t records_per_page(){
          return 5;
        }
      #else
        static inline size_t records_per_page(){
          static size_t iRet = ((_super_t::page_size() - sizeof(page_header) - sizeof(data_page<_page_size>)) / sizeof(record));
          return iRet;
        }
      #endif

        template <typename _cache_t>
        inline bool insert(_cache_t& oCache, const key_type& key, const value_type& value){
          typename _super_t::pointer oPage;
          for(;;){
            TODO("binary search instead of full scan")
            for (size_t i = 0; i < _page_header._count; ++i){
              if (key <= _records[i]._key){
                oPage = static_page_cast<_super_t>(oCache[_records[i]._left]);
                if (page_type::leaf_page == oPage->_page_type){
                  auto oLeaf = xtd::static_page_cast<leaf_t>(oPage);
                  D_(const leaf_t * pLeaf = oLeaf.get());
                  if (oLeaf->_page_header._count >= leaf_t::records_per_page()){
                    _records[_page_header._count]._left = _records[i]._left;
                    oLeaf->split(oCache, _records[i]._left, _records[_page_header._count]._key, _records[i]._left);
                    std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs){ return lhs._key < rhs._key; });
                    _page_header._count++;
                    break;
                  }
                  return oLeaf->insert(key, value);
                } else{
                  auto oBranch = xtd::static_page_cast<branch>(oPage);
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
            oPage = static_page_cast<_super_t>(oCache[_page_header._right]);
            if (page_type::leaf_page == oPage->_page_type){
              auto oLeaf = xtd::static_page_cast<leaf_t>(oPage);
              D_(const leaf_t * pLeaf = oLeaf.get());
              if (oLeaf->_page_header._count >= leaf_t::records_per_page()){
                _records[_page_header._count]._left = _page_header._right;
                oLeaf->split(oCache, _page_header._right, _records[_page_header._count]._key, _page_header._right);
                std::sort(&_records[0], &_records[_page_header._count], [](const record& lhs, const record& rhs){ return lhs._key < rhs._key; });
                _page_header._count++;
                continue;
              }
              return oLeaf->insert(key, value);              
            } else {
              auto oBranch = xtd::static_page_cast<branch>(oPage);
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
        template <typename _cache_t>
        inline void split(_cache_t& oCache, key_type& left_page_key, size_t & right_page_idx){
          auto oNewPage = static_page_cast<branch>(oCache[right_page_idx]);
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
   * @tparam _key_t the key type
   * @tparam _value_t the value type
   */
  template <typename _key_t, typename _value_t, size_t _page_size = -1, size_t _cache_size = 20>
  class btree{
  public:
    using key_type = _key_t;
    using value_type = _value_t;

  private:
    _::btree::lru_cache<_page_size, _cache_size> _cache;
    using file_header = _::btree::file_header<_page_size>;
    using data_page_t = _::btree::data_page<_page_size>;
    using leaf_t = _::btree::leaf<_key_t, _value_t, _page_size>;
    using branch_t = _::btree::branch<_key_t, _value_t, _page_size>;
    using page_type = _::btree::page_type;

    typename file_header::pointer _file_header;
  public:

    btree(const xtd::filesystem::path& oPath) : _cache(oPath), _file_header(static_page_cast<file_header>(_cache[0])){}

    /** insert a value in the container
     *
     * @param key unique key associated with the value
     * @param value value to insert
     * @return true of insert was successful, false otherwise
     */
    bool insert(const key_type& key, const value_type& value){
      //no root page
      if (0 == _file_header->_root_page){
        auto oLeaf = static_page_cast<leaf_t>(_cache.append(_file_header->_root_page));
        D_(const leaf_t * pLeaf = oLeaf.get());
        if (oLeaf->insert(key, value)){
          _file_header->_count++;
          return true;
        }
        return false;
      }

      auto oRoot = static_page_cast<data_page_t>(_cache[_file_header->_root_page]);

      if (page_type::branch_page == oRoot->_page_type){ //root is a branch page
        //root page is full
        //auto oBranch = oRoot.template cast<branch_t>();
        auto oBranch = xtd::static_page_cast<branch_t>(oRoot);
        D_(const branch_t * pBranch = oBranch.get());
        if (oBranch->_page_header._count >= branch_t::records_per_page()){
          size_t iNewRoot;
          auto oNewRoot = static_page_cast<branch_t>(_cache[iNewRoot]);
          oBranch->split(_cache, oNewRoot->_records[0]._key, oNewRoot->_page_header._right);
          oNewRoot->_records[0]._left = _file_header->_root_page;
          oNewRoot->_page_header._count = 1;
          _file_header->_root_page = iNewRoot;
          oBranch = std::move(oNewRoot);
        }
        // root page is not full
        if (oBranch->insert(_cache, key, value)){
          _file_header->_count++;
          return true;
        }
        return false;
      }
      // root is a leaf page
      auto oLeaf = xtd::static_page_cast<leaf_t>(oRoot);
      D_(const leaf_t * pLeaf = oLeaf.get());
      //leaf is not full
      if (oLeaf->_page_header._count < leaf_t::records_per_page()){
        if (oLeaf->insert(key, value)){
          _file_header->_count++;
          return true;
        }
        return false;
      }
      //leaf is full
      auto iOldRootIdx = _file_header->_root_page;
      auto oNewRoot = static_page_cast<branch_t>(_cache[_file_header->_root_page]);
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