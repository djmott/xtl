/** @file
  @brief Memory management utilities
  
  Provides utilities for memory operations including page size detection
  and smart pointer helpers.
  
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */
#pragma once
#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <unistd.h>
#endif

#include <xtd/debug.hpp>

namespace xtd{

  /** @brief Memory-related utilities
   */
  namespace memory{
  /** @brief Gets the system page size
   * 
   * Returns the memory page size used by the operating system.
   * On UNIX systems, this uses sysconf(_SC_PAGESIZE).
   * On Windows, this uses GetSystemInfo().
   * 
   * @return System page size in bytes
   * @note Result is cached after first call for performance
   */
  #if (XTD_OS_UNIX & XTD_OS)
    [[nodiscard]] static inline size_t page_size() noexcept {
      static const auto iRet = sysconf(_SC_PAGESIZE);
      return iRet;
    }
  #elif (XTD_OS_WINDOWS & XTD_OS)
    [[nodiscard]] static inline size_t page_size() noexcept {
      static SYSTEM_INFO oSysInfo = [](){
        SYSTEM_INFO info{};
        GetSystemInfo(&info);
        return info;
      }();
      return oSysInfo.dwAllocationGranularity;
    }
  #endif
  }

  /** @brief Creates a std::unique_ptr with perfect forwarding
   * 
   * Helper function to create unique_ptr instances. Similar to std::make_unique
   * but available even when std::make_unique is not (pre-C++14 compatibility).
   * 
   * @tparam _ty Type of object to create
   * @tparam _param_t Parameter type (perfect forwarding)
   * @param src Arguments to forward to constructor
   * @return unique_ptr to newly created object
   */
  /** @brief Creates a std::unique_ptr with perfect forwarding
   * 
   * Helper function to create unique_ptr instances. With C++14+, prefers
   * std::make_unique, but available for compatibility.
   * 
   * @tparam _ty Type of object to create
   * @tparam _param_t Parameter type (perfect forwarding)
   * @param src Arguments to forward to constructor
   * @return unique_ptr to newly created object
   */
#if __cpp_lib_make_unique >= 201304L
  template <typename _ty, typename ... _param_t> 
  [[nodiscard]] constexpr auto make_unique(_param_t&&... src) -> decltype(std::make_unique<_ty>(std::forward<_param_t>(src)...)) {
    return std::make_unique<_ty>(std::forward<_param_t>(src)...);
  }
#else
  template <typename _ty, typename _param_t> 
  [[nodiscard]] static inline std::unique_ptr<_ty> make_unique(_param_t&& src){ 
    return std::unique_ptr<_ty>(new _ty(std::forward<_param_t>(src))); 
  }
  
  /** @brief Creates a std::unique_ptr with const reference
   * @tparam _ty Type of object to create
   * @tparam _param_t Parameter type
   * @param src Const reference argument for constructor
   * @return unique_ptr to newly created object
   */
  template <typename _ty, typename _param_t> 
  [[nodiscard]] static inline std::unique_ptr<_ty> make_unique(const _param_t& src){ 
    return std::unique_ptr<_ty>(new _ty(src)); 
  }
#endif

}
