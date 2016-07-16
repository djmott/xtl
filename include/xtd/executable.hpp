/** @file
  represents an executable binary on disk or memory
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/


#pragma once


namespace xtd {

  namespace this_executable {
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    static inline xtd::path get_path(){
      static xtd::path sRet="";
      if (0 != sRet.string().size()){
        return sRet;
      }
      std::string sTemp(MAX_PATH, 0);
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
#elif ((XTD_OS_LINUX | XTD_OS_CYGWIN | XTD_OS_MSYS) & XTD_OS)

    static inline xtd::path get_path() {
      static xtd::path sRet = "";
      if (0 != sRet.size()){
        return sRet;
      }
      sRet.resize(PATH_MAX);
      sRet.resize(xtd::crt_exception::throw_if(::readlink("/proc/self/exe", &sRet[0], sRet.size()), [](int i) { return (-1 == i); }));
      return sRet;
    }

#endif
  }
}

