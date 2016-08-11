/** @file
load and invoke methods in a dynamic library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

 */

#pragma once

#include <xtd/xtd.hpp>

#if ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
  #include <dlfcn.h>
#endif

#include <memory>
#include <map>

#include <xtd/exception.hpp>
#include <xtd/filesystem.hpp>

namespace xtd{

  class dynamic_library_exception
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    : public xtd::windows::exception{
      using _super_t = xtd::windows::exception;
#elif ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
    : public xtd::exception{
      using _super_t = xtd::exception;
#endif
  public:

    template <typename _ReturnT, typename _ExpressionT>
    inline static _ReturnT _throw_if(const xtd::source_location& source, _ReturnT ret, _ExpressionT exp, const char* expstr){
      if (exp(ret)){
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
          throw dynamic_library_exception(source, expstr);
#elif ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
          throw dynamic_library_exception(source, std::string(dlerror()) + " " + expstr);
#endif
      }
      return ret;
    }

    dynamic_library_exception(const source_location& Source, const std::string& What) : _super_t(Source, What){}
    dynamic_library_exception(const dynamic_library_exception& src) : _super_t(src){}
    dynamic_library_exception(dynamic_library_exception&& src) : _super_t(std::move(src)){}

  };

  class dynamic_library : public std::enable_shared_from_this<dynamic_library>{
  public:
  #if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    using native_handle_type = HMODULE;
  #elif ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
    using native_handle_type = void *;
  #endif

    using pointer = std::shared_ptr<dynamic_library>;
    using map = std::map<xtd::filesystem::path, pointer>;

    static inline pointer make(const char * spath){ return pointer(new dynamic_library(spath)); }
    static inline pointer make(const xtd::filesystem::path& spath){ return pointer(new dynamic_library(spath.string().c_str())); }

    native_handle_type handle() const{ return _Handle; }

    template <typename _ReturnT, typename ... _ArgsT>
    class function{
    public:
      using function_pointer_type = _ReturnT(*)(_ArgsT...);

      inline _ReturnT operator()(_ArgsT...oArgs) const{
        return _function_pointer(std::forward<_ArgsT>(oArgs)...);
      }

      function() = delete;
      ~function() = default;
      function(const function& src) : _function_pointer(src._function_pointer), _library(src._library){}
      function& operator=(const function& src){
        if (this==&src){
          return *this;
        }
        _function_pointer = src._function_pointer;
        _library = src._library;
        return *this;
      }
    private:
      friend class dynamic_library;
      function(function_pointer_type fnptr, dynamic_library::pointer oLib) : _function_pointer(fnptr), _library(oLib){}
      function_pointer_type _function_pointer = nullptr;
      dynamic_library::pointer _library;
    };

    dynamic_library() = delete;
    dynamic_library(const dynamic_library&) = delete;
    dynamic_library& operator=(const dynamic_library&) = delete;
    dynamic_library(dynamic_library&& src) : _Handle(src._Handle){
      src._Handle = nullptr;
    }
    dynamic_library& operator=(dynamic_library&& src){
      if (this == &src) {
        return *this;
      }
      _Handle = src._Handle;
      src._Handle = nullptr;
      return *this;
    }
  #if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    ~dynamic_library(){
      if (_Handle){
        FreeLibrary(_Handle);
      }
    }
  #elif ((XTD_OS_LINUX | XTD_OS_CYGWIN) & XTD_OS)
    ~dynamic_library(){
      if (_Handle){
        dlclose(_Handle);
      }
    }
  #endif

    template <typename _ReturnT, typename ... _ArgsT> function <_ReturnT, _ArgsT...> get(const char * name){
      using return_type = function <_ReturnT, _ArgsT...>;
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
      auto fnptr = reinterpret_cast<typename return_type::function_pointer_type>(xtd::dynamic_library_exception::throw_if(GetProcAddress(_Handle, name), [](FARPROC p){ return nullptr == p; }));
#elif ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
      auto fnptr = reinterpret_cast<typename return_type::function_pointer_type>(xtd::dynamic_library_exception::throw_if(dlsym(_Handle, name), [](void * p){ return nullptr == p; }));
#endif
      return return_type(fnptr, shared_from_this());
    }

  private:

    friend class process;

#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    explicit dynamic_library(const tchar * sPath) : _Handle(xtd::exception::throw_if(LoadLibrary(sPath), [](HMODULE h){ return (INVALID_HANDLE_VALUE == h || nullptr == h); })){}
#elif ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)
    explicit dynamic_library(const char * sPath) : _Handle(xtd::dynamic_library_exception::throw_if(dlopen(sPath, RTLD_LAZY), [](native_handle_type h){ return nullptr == h; })){}
#endif

    explicit dynamic_library(native_handle_type hHandle) : _Handle(hHandle){}

    native_handle_type _Handle;
  };
}
