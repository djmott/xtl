/** @file
memory mapped files
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#if ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
  #include <sys/mman.h>
  #include <fcntl.h>
#endif

#include <xtd/memory.hpp>
#include <xtd/debug.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/exception.hpp>

namespace xtd{

#if ((XTD_OS_CYGWIN|XTD_OS_MSYS|XTD_OS_LINUX) & XTD_OS)
  class mapped_file{
    int _FileNum;
    size_t _PageSize;
  public:

    ~mapped_file(){ close(_FileNum); }

    explicit mapped_file(const filesystem::path& Path)
    : _FileNum(xtd::crt_exception::throw_if(open(Path.string().c_str(), O_CREAT|O_RDWR), [](int i){ return -1==i; }))
    {}



    template <typename _Ty> class mapped_page{
    public:
      using value_type = _Ty;

      ~mapped_page(){ munmap(_Page, memory::page_size()); }

      mapped_page(const mapped_page&) = delete;
      mapped_page& operator=(const mapped_page&) = delete;

      mapped_page(mapped_page&& src) : _Page(nullptr){
        std::swap(_Page, src._Page);
      }
      mapped_page& operator=(mapped_page&& src){
        if (this != &src) {
          std::swap(_Page, src._Page);
        }
        return *this;
      }

      value_type* get() { return _Page; }
      const value_type* get() const { return _Page; }

      value_type* operator->() { return get();}
      const value_type* operator->() const { return get();}

      value_type& operator*() { XTD_ASSERT(get()); return *get();}
      const value_type& operator*() const { XTD_ASSERT(get()); return *get();}

      void flush(){
        xtd::crt_exception::throw_if(msync(_Page, memory::page_size(), MS_SYNC), [](int i){return 0 != i;});
      }

    protected:
      friend class mapped_file;
      explicit mapped_page(void * addr) : _Page(reinterpret_cast<value_type*>(addr)){}
      value_type* _Page;
    };

    template <typename _Ty> mapped_page<_Ty> get(size_t pageNum){
      auto iPageSize = memory::page_size();
      DUMP(iPageSize);
      return mapped_page<_Ty>(
        xtd::crt_exception::throw_if(
          mmap(nullptr, iPageSize, PROT_READ|PROT_WRITE, MAP_SHARED,  _FileNum, (pageNum * iPageSize)),
          [](void*addr){ DUMP(addr); return nullptr==addr || MAP_FAILED==addr; }
        ));
    }
  };
#elif ((XTD_OS_MINGW|XTD_OS_WINDOWS) & XTD_OS)
  class mapped_file{
    HANDLE _hFile;
    HANDLE _hMap;
  public:
    ~mapped_file(){
      CloseHandle(_hMap); 
      CloseHandle(_hFile);  
    }
    explicit mapped_file(const filesystem::path& Path)
      : _hFile(xtd::windows::exception::throw_if(CreateFileA(Path.string().c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr), [](HANDLE h){ return nullptr==h || INVALID_HANDLE_VALUE==h; }))
      , _hMap(xtd::windows::exception::throw_if(CreateFileMapping(_hFile, nullptr, PAGE_READWRITE, 0, xtd::memory::page_size(), nullptr), [](HANDLE h){ return nullptr == h || INVALID_HANDLE_VALUE == h; }))      
      {}

    template <typename _Ty> class mapped_page{
    public:
      using value_type = _Ty;

      ~mapped_page(){ UnmapViewOfFile(_Page); }

      mapped_page(const mapped_page&) = delete;
      mapped_page& operator=(const mapped_page&) = delete;

      mapped_page(mapped_page&& src) : _Page(nullptr){
        std::swap(_Page, src._Page);
      }
      mapped_page& operator=(mapped_page&& src){
        if (this != &src){
          std::swap(_Page, src._Page);
        }
        return *this;
      }

      value_type* get(){ return _Page; }
      const value_type* get() const{ return _Page; }

      value_type* operator->(){ return get(); }
      const value_type* operator->() const{ return get(); }

      value_type& operator*(){ XTD_ASSERT(get()); return *get(); }
      const value_type& operator*() const{ XTD_ASSERT(get()); return *get(); }

      void flush(){
        xtd::windows::exception::throw_if(FlushViewOfFile(_Page, memory::page_size()), [](BOOL b){return FALSE==b; });
      }

    protected:
      friend class mapped_file;
      explicit mapped_page(void * addr) : _Page(reinterpret_cast<value_type*>(addr)){}
      value_type* _Page;
    };

    template <typename _Ty> mapped_page<_Ty> get(size_t pageNum){
      pageNum *= memory::page_size();
      return mapped_page<_Ty>(
        xtd::windows::exception::throw_if(
        MapViewOfFile(_hMap, FILE_MAP_READ|FILE_MAP_WRITE , hidword(pageNum), lodword(pageNum), memory::page_size()),
        [](void*addr){ return nullptr == addr; }
      ));
    }

  };
#endif
}

