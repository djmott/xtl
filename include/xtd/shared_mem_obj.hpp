/** @file
creates or loads singleton objects in shared memory regions
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/
#pragma once
#include <xtd/xtd.hpp>
#include <cstddef>

namespace xtd{
  template <typename _ty> class shared_mem_obj{
    volatile bool _initialized; 
    size_t _ObjectID;
  public:
    shared_mem_obj() = delete;
    shared_mem_obj(const shared_mem_obj&) = delete;
    shared_mem_obj& operator=(const shared_mem_obj&) = delete;
    void * operator new(size_t,const char * name, size_t len);
    void operator delete(void* addr);
    virtual void initialize() = 0;
    virtual void destroy() = 0;
  };
}
