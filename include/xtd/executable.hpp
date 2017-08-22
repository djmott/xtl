/** @file
  represents an executable binary on disk or memory
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/


#pragma once

#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <limits.h>
  #include <unistd.h>
#endif

#include <xtd/filesystem.hpp>
#include <xtd/exception.hpp>

namespace xtd {

  /** Represents an executable binary
  Provides access to the executable binary file structure
   */
  class executable{
    xtd::filesystem::path _Path;

    explicit executable(const xtd::filesystem::path& oPath) : _Path(oPath){}

#if (XTD_OS_WINDOWS & XTD_OS)
    static inline xtd::filesystem::path get_path(){
      static xtd::filesystem::path sRet="";
      if (0 != sRet.string().size()){
        return sRet;
      }
      xtd::tstring sTemp(MAX_PATH, 0);
      forever {
        auto iLen = xtd::crt_exception::throw_if(GetModuleFileName(nullptr, &sTemp[0], static_cast<DWORD>(sTemp.size())), [](DWORD ret){ return (0==ret); });
        if (iLen >= sTemp.size()){
          sTemp.resize(sTemp.size() * 2);
        }else{
          sTemp.resize(iLen);
          break;
        }
      }
      sRet = sTemp;
      return sRet;
    }
#elif (XTD_OS_UNIX & XTD_OS)

    static inline xtd::filesystem::path get_path() {
      static std::string sRet(PATH_MAX, 0);
      if (!sRet[0]) {
        sRet.resize(xtd::crt_exception::throw_if(::readlink("/proc/self/exe", &sRet[0], sRet.size()), [](int i) { return (-1 == i); }));
      }
      return sRet;
    }

#endif

  public:

    ///
    const xtd::filesystem::path& path() const { return _Path; }

    static executable& this_executable(){
      static executable _executable(get_path());
      return _executable;
    }

  };

}

