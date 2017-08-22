/** @file
 * memory related methods
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */
#pragma once
#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <unistd.h>
#endif

#include <xtd/debug.hpp>

namespace xtd{

  namespace memory{
  #if (XTD_OS_UNIX & XTD_OS)
    static inline size_t page_size(){
      static auto iRet = sysconf(_SC_PAGESIZE);
      return iRet;
    }
  #elif (XTD_OS_WINDOWS & XTD_OS)
    static inline size_t page_size(){
      static bool bInit = false;
      static SYSTEM_INFO oSysInfo;
      if (!bInit){
        bInit = true;
        GetSystemInfo(&oSysInfo);
      }
      return oSysInfo.dwAllocationGranularity;
    }
  #endif
  }

  template <typename _Ty, typename _ParamT> static inline std::unique_ptr<_Ty> make_unique(_ParamT&& src){ return std::unique_ptr<_Ty>(new _Ty(std::forward<_ParamT>(src))); }
  template <typename _Ty, typename _ParamT> static inline std::unique_ptr<_Ty> make_unique(const _ParamT& src){ return std::unique_ptr<_Ty>(new _Ty(src)); }

}
