/** @file
memory mapped vector
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/mapped_file.hpp>
#include <xtd/lru_cache.hpp>

namespace xtd{

  /**
  erase item from a mapped vector policy that shifts remaining items one place toward the erased item
  @tparam _MappedVectorT the mapped vector type
  */
  template <typename _MappedVectorT>
  class shift_erase_policy{

  };

  template <typename _Ty, size_t _page_size = ((size_t)(-1)), template <typename> class _ErasePolicyT = shift_erase_policy>
  class mapped_vector {
    friend class iterator;
    template <typename> friend class shift_erase_policy;
    using _super_t = mapped_file<_page_size>;
    using _erase_policy_t = _ErasePolicyT<mapped_vector>;

    struct page{
      using pointer = mapped_page<page>;
    };

    PACK_PUSH(1);
    struct file_header_page : page{
      size_t _count;
    };
    PACK_POP();

    PACK_PUSH(1);
    struct data_page : page{
      _Ty _values[1];
      //static size_t items_per_page(){ return _::mapped_file_base<_page_size>::page_size() / sizeof(_Ty); }
      static size_t items_per_page(){ return 5; }
    };
    PACK_POP();

    class page_loader{
      mapped_file<_page_size>& _file;
    public:
      explicit page_loader(mapped_file<_page_size>& oFile) : _file(oFile){}
      page_loader(const page_loader& src) : _file(src._file){}
      typename page::pointer operator()(size_t iPage){
        return _file.template get<page>(iPage);
      }
    };

    using cache_type = lru_cache<size_t, typename page::pointer, 3, page_loader>;

    mutable mapped_file<_page_size> _file;
    mutable cache_type _cache;

  public:
    using value_type = _Ty;
    static const size_t npos = -1;
    
    explicit mapped_vector(const xtd::filesystem::path& oPath) 
      : _file(oPath),
      _cache(page_loader(_file)){}

    class iterator{
      template <typename,size_t, template <typename> class> friend class mapped_vector;
      size_t _current_index;
      mapped_vector& _vector;

      iterator(size_t index, mapped_vector& oVector) : _current_index(index), _vector(oVector){}

    public:

      bool operator != (const iterator& rhs) const { return _current_index != rhs._current_index; }

      iterator& operator++(int){
        _current_index++;
        return *this;
      }

      iterator operator++(){
        iterator oRet(*this);
        this->operator++(0);
        return oRet;
      }

      value_type* get(){
        XTD_ASSERT(npos != _current_index);
        auto iPage = 1 + (_current_index / data_page::items_per_page());
        auto oPage = xtd::static_page_cast<data_page>(_vector._cache[iPage]);
        return &oPage->_values[_current_index % data_page::items_per_page()];
      }

      const value_type* get() const {
        XTD_ASSERT(npos != _current_index);
        auto iPage = 1 + (_current_index / data_page::items_per_page());
        auto oPage = xtd::static_page_cast<data_page>(_vector._cache[iPage]);
        return &oPage->_values[_current_index % data_page::items_per_page()];
      }
            
      value_type* operator->(){
        return get();
      }
      const value_type* operator->() const {
        return get();
      }

      value_type& operator*(){
        return *get();
      }

      const value_type& operator*() const {
        return *get();
      }


    };

    void push_back(const value_type& value){
      auto oRoot = xtd::static_page_cast<file_header_page>(_cache[0]);
      auto iPage = 1 + (oRoot->_count / data_page::items_per_page());
      auto oPage = xtd::static_page_cast<data_page>(_cache[iPage]);
      oPage->_values[ oRoot->_count % data_page::items_per_page() ] = value;
      oRoot->_count++;
    }

    size_t size() const {
      auto oRoot = xtd::static_page_cast<file_header_page>(_cache[0]);
      return oRoot->_count;
    }
    iterator end() { 
      auto oRoot = xtd::static_page_cast<file_header_page>(_cache[0]);
      return iterator(oRoot->_count, *this);
    }
    iterator begin(){
      auto oRoot = xtd::static_page_cast<file_header_page>(_cache[0]);
      return iterator( (oRoot->_count ? 0 : -1), *this); }

  };
}
