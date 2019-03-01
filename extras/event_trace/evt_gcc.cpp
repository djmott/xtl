/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */


extern "C" {


  void __xtd_EventEnter(const char * fn);
  void __xtd_EventLeave();


  // cppcheck-suppress unusedFunction
  void __cyg_profile_func_enter(void *, void * /*call_site*/){
    //__xtd_EventEnter("");
  }

  // cppcheck-suppress unusedFunction
  void __cyg_profile_func_exit(void *, void * /*call_site*/){
   //__xtd_EventLeave();
  }
}
