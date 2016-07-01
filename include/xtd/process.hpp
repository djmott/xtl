/** @file
  represents an in-memory process
  \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

namespace xtd{

  struct process{
  #if ((XTD_OS_MINGW | XTD_OS_WINDOWS) & XTD_OS)
    using id_type = DWORD;
  #endif
  };


  namespace this_process{
#if ((XTD_OS_MINGW | XTD_OS_WINDOWS) & XTD_OS)
    process::id_type id(){ return GetCurrentProcessId(); }
#endif
  }

}
