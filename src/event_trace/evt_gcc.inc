/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

xtd::concurrent::hash_map<void*,std::string> _FunctionNames;

extern "C" {


  void __xtd_EventEnter(const char * fn);
  void __xtd_EventLeave();


  // cppcheck-suppress unusedFunction
  void __cyg_profile_func_enter(void *this_fn, void * /*call_site*/){
    Dl_info oInfo;
    if (!_FunctionNames.find(this_fn)){
      dladdr(this_fn, &oInfo);
    }
    __xtd_EventEnter(this_fn);
  }

  // cppcheck-suppress unusedFunction
  void __cyg_profile_func_exit(void *this_fn, void */*call_site*/){
    __xtd_EventLeave(this_fn);
  }
}
