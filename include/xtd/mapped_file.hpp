/** @file
memory mapped files
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#if ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <fcntl.h>
#endif

#include <xtd/memory.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/exception.hpp>
#include <xtd/meta.hpp>

namespace xtd{

  namespace _{
    /// static page size
    template <size_t _PageSize>
    class mapped_file_base{
    public:
      static size_t page_size(){ return _PageSize; }
    };

    /// dynamic page size uses system page size
    template <>
    class mapped_file_base<-1>{
    public:
      static size_t page_size(){
        static size_t iRet = xtd::memory::page_size();
        return iRet;
      }
    };

  }

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
      struct stat oStat;
      xtd::crt_exception::throw_if(fstat(_FileNum, &oStat), [](int i){ return -1 == i; });
      off_t iLastByte = (pageNum * iPageSize) + iPageSize;
      if (oStat.st_size < iLastByte){
        xtd::crt_exception::throw_if(lseek(_FileNum, iLastByte, SEEK_SET), [](long l){ return -1==l;});
        xtd::crt_exception::throw_if(write(_FileNum, "", 1), [](int i){ return 1 != i;});
      }
      return mapped_page<_Ty>(
        xtd::crt_exception::throw_if(
          mmap(nullptr, iPageSize, PROT_READ|PROT_WRITE, MAP_SHARED,  _FileNum, (pageNum * iPageSize)),
          [](void*addr){ return nullptr==addr || MAP_FAILED==addr; }
        ));
    }
  };
#elif ((XTD_OS_MINGW|XTD_OS_WINDOWS) & XTD_OS)

  template <size_t _PageSize>
  class mapped_file : _::mapped_file_base<_PageSize>{
    using _super_t = _::mapped_file_base<_PageSize>;
    HANDLE _hFile;
    HANDLE _hMap;
  public:
    ~mapped_file(){
      CloseHandle(_hMap); 
      CloseHandle(_hFile);  
    }
    explicit mapped_file(const filesystem::path& Path)
      : _hFile(xtd::windows::exception::throw_if(CreateFileA(Path.string().c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr), [](HANDLE h){ return nullptr==h || INVALID_HANDLE_VALUE==h; }))
      , _hMap(xtd::windows::exception::throw_if(CreateFileMapping(_hFile, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(_super_t::page_size()), nullptr), [](HANDLE h){ return nullptr == h || INVALID_HANDLE_VALUE == h; }))
      {}

    template <typename _Ty> class mapped_page : public std::shared_ptr <_Ty>
    {
      using _super_t = std::shared_ptr<_Ty>;
      template <size_t> friend class mapped_file;
      explicit mapped_page(void * addr) : _super_t(reinterpret_cast<_Ty*>(addr), &UnmapViewOfFile){}
      template <typename ... _ArgTs> mapped_page(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}
    public:
   
      mapped_page() : _super_t(nullptr, &UnmapViewOfFile){}

      void flush(){
        xtd::windows::exception::throw_if(FlushViewOfFile(get(), _PageSize), [](BOOL b){return FALSE==b; });
      }


      template<typename _OtherT>
      mapped_page<_OtherT> cast(){
        return (mapped_page<_OtherT>(*this, static_cast<_OtherT*>(get())));
      }
    };

    template <typename _Ty> mapped_page<_Ty> get(size_t pageNum){
      size_t newSize = (pageNum * _super_t::page_size()) + _super_t::page_size();
      LARGE_INTEGER iSize;
      xtd::windows::exception::throw_if(GetFileSizeEx(_hFile, &iSize), [](BOOL b){return FALSE == b; });
      if (static_cast<size_t>(iSize.QuadPart) < newSize){
        xtd::windows::exception::throw_if(CloseHandle(_hMap), [](BOOL b){ return FALSE == b; });
        iSize.QuadPart = newSize;
        _hMap = xtd::windows::exception::throw_if(CreateFileMapping(_hFile, nullptr, PAGE_READWRITE, iSize.HighPart, iSize.LowPart, nullptr), [](HANDLE h){ return nullptr == h || INVALID_HANDLE_VALUE == h; });
      }
      iSize.QuadPart -= _super_t::page_size();
      return mapped_page<_Ty>(
        xtd::windows::exception::throw_if(
        MapViewOfFile(_hMap, FILE_MAP_READ|FILE_MAP_WRITE , iSize.HighPart, iSize.LowPart, _super_t::page_size()),
        [](void*addr){ return nullptr == addr; }
      ));
    }

    template <typename _Ty> mapped_page<_Ty> append(size_t& newpage){
      LARGE_INTEGER iSize;
      xtd::windows::exception::throw_if(GetFileSizeEx(_hFile, &iSize), [](BOOL b){return FALSE == b; });
      newpage = static_cast<size_t>((iSize.QuadPart + _super_t::page_size()) / _super_t::page_size());
      return get<_Ty>(newpage);
    }

  };
#endif
}

