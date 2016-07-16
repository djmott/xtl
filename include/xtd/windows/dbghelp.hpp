/** @file
object oriented access to the dbghelp library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace windows{
    class debug_help{
      debug_help(){
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

        xtd::windows::exception::throw_if(SymInitialize(GetCurrentProcess(), NULL, TRUE), [](BOOL b){return FALSE == b; });
      }
    private:
      std::mutex _library_lock;
    };
  }
}