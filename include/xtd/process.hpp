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
      static process _this_process;
      return _this_process(getpid());
    }

  private:
    process(pid_type hPid) : _pid(hPid){}
    pid_type _pid;
  };



}
