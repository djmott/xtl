/** @file
 * memory related methods
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */
#pragma once
#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <unistd.h>
#elif (XTD_OS_WINDOWS & XTD_OS)
  #include <windows.h>
#endif


#include <memory>

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

  template <typename _ty, typename ... _arg_ts> constexpr std::unique_ptr<_ty> make_unique(_arg_ts&&...args){ return std::unique_ptr<_ty>(new _ty(std::forward<_arg_ts>(args)...)); }

}
