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

namespace xtd {

  /** Represents an executable binary
  Provides access to the executable binary file structure
   */
  class executable{
    xtd::filesystem::path _path;

    explicit executable(const xtd::filesystem::path& oPath) : _path(oPath){}

#if (XTD_OS_WINDOWS & XTD_OS)
    static inline xtd::filesystem::path get_path(){
      static xtd::tstring sPath;
      if (!sPath.empty()) return xtd::filesystem::path(sPath);
      sPath.resize(MAX_PATH, 0);
      DWORD iLen;
      for (;;) {
        iLen = xtd::crt_exception::throw_if(::GetModuleFileName(nullptr, &sPath[0], static_cast<DWORD>(sPath.size())), [](DWORD d) { return !d; });
        if (iLen < sPath.size()) break;
        sPath.resize(2 * sPath.size());
      }
      sPath.resize(iLen);
      return xtd::filesystem::path(sPath);
    }

#elif (XTD_OS_UNIX & XTD_OS)

    static inline xtd::filesystem::path get_path() {
      static std::string sRet(PATH_MAX, 0);
      if (!sRet[0]) {
        sRet.resize(xtd::crt_exception::throw_if(::readlink("/proc/self/exe", &sRet[0], sRet.size()), [](int i) { return (-1 == i); }));
      }
      return xtd::filesystem::path(sRet);
    }

#endif

  public:

    ///
    const xtd::filesystem::path& path() const { return _path; }

    static executable& this_executable(){
      static executable _executable(get_path());
      return _executable;
    }

  };

}

