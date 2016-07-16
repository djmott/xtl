/** @file
  represents an in-memory process
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

namespace xtd{

  class process{
  public:

#if (XTD_OS_LINUX & XTD_OS)
    using pid_type = pid_t;
#elif (XTD_OS_WINDOWS & XTD_OS)
    using pid_type = HINSTANCE;
#else
  #error "Unsupported system for xtd::process"
#endif

    static process& this_process(){
      static process _this_process(getpid());
      return _this_process;
    }

  private:
    process(pid_type hPid) : _pid(hPid){

      for (auto pMap = reinterpret_cast<const link_map*>(dlopen(0, RTLD_LAZY)); pMap ; pMap = pMap->l_next){
        if (pMap->l_name){
          std::cout << pMap->l_name << std::endl;
        }
      }

//      dl_iterate_phdr(enum_libraries, this);

    }
/*    static int enum_libraries(dl_phdr_info * pInfo, size_t size, void * data){
      auto pThis = reinterpret_cast<process*>(data);
      std::cout << pInfo->dlpi_name << std::endl;
    }*/
    pid_type _pid;
  };



}
