/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>

#if (XTD_COMPILER_GCC & XTD_COMPILER)
  #include "evt_gcc.inc"
#endif

namespace{
  thread_local std::stack<const char *> _ThreadStack;
  thread_local bool _InTrace = false;
}

extern "C"{
  void __xtd_EventEnter(const char * fn){
    if (_InTrace){
      return;
    }
/*    Dl_info oInfo;
    if (!_FunctionNames.exists(pAddr) && dladdr(pAddr, &oInfo)){
      _FunctionNames.insert(pAddr, oInfo.dli_sname);
    }*/
    _InTrace = true;
    _ThreadStack.push("");
    _InTrace = false;
  }

  void __xtd_EventLeave(){
    if (_InTrace){
      return;
    }
    _InTrace = true;
    _ThreadStack.pop();
    _InTrace = false;
  }
}
