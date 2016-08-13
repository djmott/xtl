/** @file
memory mapped vector
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/mapped_file.hpp>

namespace xtd{

  template <typename _Ty, size_t _page_size = -1>
  class mapped_vector : mapped_file<_page_size>{
    friend class iterator;
    using _super_t = mapped_file<_page_size>;

    struct file_header_page{
      using pointer = typename _super_t::template mapped_page<file_header_page>;
      size_t _count;
    };

    struct data_page{
      using pointer = typename _super_t::template mapped_page<data_page>;
      _Ty _values[1];
      static size_t items_per_page(){ return _::mapped_file_base<_page_size>::page_size() / sizeof(_Ty); }
    };

    typename file_header_page::pointer _root;

  public:
    using value_type = _Ty;
    template <typename ... _ArgTs> mapped_vector(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...), _root(_super_t::template get<file_header_page>(0)){}

    class iterator{
      template <typename,size_t> friend class mapped_vector;
      using data_page_t = typename mapped_vector<_Ty, _page_size>::data_page;
      mapped_vector& _file;
      size_t _current_index;
      typename _super_t::template mapped_page<data_page_t> _current_page;
      iterator(size_t index, mapped_vector& oFile) : _current_index(index), _file(oFile){
        if (-1 != _current_index){
          _current_page = _file.get(1 + _current_index / _super_t::page_size());
        }
      }
    public:

    };

    void push_back(value_type& value){
      auto oPage = _super_t::template get<data_page>(1 + _root->_count / data_page::items_per_page());
      oPage->_values[_root->_count % data_page::items_per_page()] = value;
      _root->_count++;
    }

    iterator end() { return iterator(-1, *this);}
    iterator begin(){ return iterator( (_root->_count ? 0 : -1), *this); }

  };
}
