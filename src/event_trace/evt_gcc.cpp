/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

extern "C" {

  void __xtd_EventEnter(void * addr);
  void __xtd_EventLeave(void * addr);

  // cppcheck-suppress unusedFunction
  void __cyg_profile_func_enter(void *this_fn, void * /*call_site*/){
    __xtd_EventEnter(this_fn);
  }

  // cppcheck-suppress unusedFunction
  void __cyg_profile_func_exit(void *this_fn, void */*call_site*/){
    __xtd_EventLeave(this_fn);
  }
}
