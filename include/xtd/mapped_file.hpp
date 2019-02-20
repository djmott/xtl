/** @file
memory mapped files
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <fcntl.h>
#endif

#include <xtd/memory.hpp>
#include <xtd/filesystem.hpp>

namespace xtd{

  namespace _{
    /// static page size
    template <size_t _page_size>
    class mapped_file_base{
    public:
      static size_t page_size(){ return _page_size; }
    };

    /// dynamic page size uses system page size
    template <>
    class mapped_file_base<((size_t)-1)>{
    public:
      static size_t page_size(){
        static size_t iRet = xtd::memory::page_size();
        return iRet;
      }
    };
  }

#if (XTD_OS_UNIX & XTD_OS)


  template <typename _ty> class mapped_page : public std::shared_ptr <_ty>{

    using _super_t = std::shared_ptr<_ty>;
    template <size_t> friend class mapped_file;
    explicit mapped_page(void * addr) : _super_t(reinterpret_cast<_ty*>(addr), [](void*addr) { munmap(addr, 1);}) {}
  public:
    template <typename ... _arg_ts> mapped_page(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

    mapped_page& operator=(const mapped_page& src){
      _super_t::operator =(src);
      return *this;
    }
    mapped_page& operator=(mapped_page&& rhs){
      _super_t::operator =(std::move(rhs));
      return *this;
    }

    void flush(){
      xtd::crt_exception::throw_if(msync(_super_t::get(), 1, MS_SYNC), [](int i){return 0 != i;});
    }

  };

  template <size_t _page_size>
  class mapped_file : _::mapped_file_base<_page_size>{
    using _super_t = _::mapped_file_base<_page_size>;
    int _file_num;
  public:

    ~mapped_file(){ close(_file_num); }

    explicit mapped_file(const filesystem::path& Path)
    : _file_num(xtd::crt_exception::throw_if(open(Path.string().c_str(), O_CREAT|O_RDWR), [](int i){ return -1==i; }))
    {}




    template <typename _ty> mapped_page<_ty> get(size_t pageNum){
      auto iPageSize = _super_t::page_size();
      struct stat oStat;
      xtd::crt_exception::throw_if(fstat(_file_num, &oStat), [](int i){ return -1 == i; });
      off_t iLastByte = (pageNum * iPageSize) + iPageSize;
      if (oStat.st_size < iLastByte){
        xtd::crt_exception::throw_if(lseek(_file_num, iLastByte, SEEK_SET), [](long l){ return -1==l;});
        xtd::crt_exception::throw_if(write(_file_num, "", 1), [](int i){ return 1 != i;});
      }
      return mapped_page<_ty>(
        xtd::crt_exception::throw_if(
          mmap(nullptr, iPageSize, PROT_READ|PROT_WRITE, MAP_SHARED,  _file_num, (pageNum * iPageSize)),
          [](void*addr){ return nullptr==addr || MAP_FAILED==addr; }));
    }

    template <typename _ty> mapped_page<_ty> append(size_t& newpage){
      auto iPageSize = _super_t::page_size();
      struct stat oStat;
      xtd::crt_exception::throw_if(fstat(_file_num, &oStat), [](int i){ return -1 == i; });
      newpage = oStat.st_size / iPageSize;
      xtd::crt_exception::throw_if(lseek(_file_num, oStat.st_size + iPageSize, SEEK_SET), [](long l){ return -1==l;});
      xtd::crt_exception::throw_if(write(_file_num, "", 1), [](int i){ return 1 != i;});

      return mapped_page<_ty>(
          xtd::crt_exception::throw_if(
              mmap(nullptr, iPageSize, PROT_READ|PROT_WRITE, MAP_SHARED,  _file_num, oStat.st_size),
              [](void*addr){ return nullptr==addr || MAP_FAILED==addr; }), 
        [](_ty*addr){ munmap(addr, _::mapped_file_base<_page_size>::page_size()); });
    }
  };
#elif (XTD_OS_WINDOWS & XTD_OS)



  template <typename _ty> class mapped_page : public std::shared_ptr <_ty>{
    using _super_t = std::shared_ptr<_ty>;
    template <size_t> friend class mapped_file;
    explicit mapped_page(void * addr) : _super_t(reinterpret_cast<_ty*>(addr), &UnmapViewOfFile){}
  public:

    template <typename ... _arg_ts> mapped_page(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

    mapped_page& operator=(const mapped_page& src){
      _super_t::operator =(src);
      return *this;
    }
    mapped_page& operator=(mapped_page&& rhs){
      _super_t::operator =(std::move(rhs));
      return *this;
    }

    void flush(){
      xtd::windows::exception::throw_if(FlushViewOfFile(_super_t::get(), 0), [](BOOL b){return FALSE == b; });
    }

  };

  template <size_t _page_size>
  class mapped_file : _::mapped_file_base<_page_size>{
    using _super_t = _::mapped_file_base<_page_size>;
    HANDLE _hFile;
    HANDLE _hMap;
  public:
    ~mapped_file(){
      if (_hMap && INVALID_HANDLE_VALUE != _hMap) CloseHandle(_hMap);
      if (_hFile && INVALID_HANDLE_VALUE != _hFile) CloseHandle(_hFile);
    }
    explicit mapped_file(const filesystem::path& Path)
      : _hFile(xtd::windows::exception::throw_if(CreateFileA(Path.string().c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr), [](HANDLE h){ return nullptr==h || INVALID_HANDLE_VALUE==h; }))
      , _hMap(xtd::windows::exception::throw_if(CreateFileMapping(_hFile, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(_super_t::page_size()), nullptr), [](HANDLE h){ return nullptr == h || INVALID_HANDLE_VALUE == h; }))
      {}



    template <typename _ty> mapped_page<_ty> get(size_t pageNum){
      size_t newSize = (pageNum * _super_t::page_size()) + _super_t::page_size();
      LARGE_INTEGER iSize;
      xtd::windows::exception::throw_if(GetFileSizeEx(_hFile, &iSize), [](BOOL b){return FALSE == b; });
      if (static_cast<size_t>(iSize.QuadPart) < newSize){
        xtd::windows::exception::throw_if(CloseHandle(_hMap), [](BOOL b){ return FALSE == b; });
        iSize.QuadPart = newSize;
        _hMap = xtd::windows::exception::throw_if(CreateFileMapping(_hFile, nullptr, PAGE_READWRITE, iSize.HighPart, iSize.LowPart, nullptr), [](HANDLE h){ return nullptr == h || INVALID_HANDLE_VALUE == h; });
      }
      iSize.QuadPart = newSize - _super_t::page_size();
      return mapped_page<_ty>(
        xtd::windows::exception::throw_if(
        MapViewOfFile(_hMap, FILE_MAP_READ|FILE_MAP_WRITE , iSize.HighPart, iSize.LowPart, _super_t::page_size()),
        [](void*addr){ return nullptr == addr; }));
    }

    template <typename _ty> mapped_page<_ty> append(size_t& newpage){
      LARGE_INTEGER iSize;
      xtd::windows::exception::throw_if(GetFileSizeEx(_hFile, &iSize), [](BOOL b){return FALSE == b; });
      newpage = static_cast<size_t>((iSize.QuadPart + _super_t::page_size()) / _super_t::page_size());
      return get<_ty>(newpage);
    }

  };
#endif

  template<typename _other_t, typename _this_t>
  mapped_page<_other_t> static_page_cast(_this_t ptr){
    return (mapped_page<_other_t>(ptr, static_cast<_other_t*>(ptr.get())));
  }

}

