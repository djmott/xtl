/** @file
Recursive spin lock
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{
  namespace concurrent{
    template <typename _WaitPolicyT = null_wait_policy>
    class recursive_spin_lock_base : private std::atomic< std::thread::native_handle_type>{
      using _super_t = std::atomic < std::thread::native_handle_type >;
      using wait_policy_type = _WaitPolicyT;
      uint32_t _lock_count;
      wait_policy_type _WaitPolicy;
    public:
      using scope_locker = xtd::concurrent::scope_locker<recursive_spin_lock_base<_WaitPolicyT>>;
      ~recursive_spin_lock_base() = default;
      recursive_spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _super_t(), _WaitPolicy(oWait){};
      recursive_spin_lock_base(const recursive_spin_lock_base&) = delete;
      recursive_spin_lock_base(recursive_spin_lock_base&&) = delete;
      bool try_lock(){
        auto nullid = reinterpret_cast<std::thread::native_handle_type>(nullptr);
        auto thisid = std::this_thread::get_id().native_handle();
        if (!(compare_exchange_strong(&nullid, thisid)) && !(compare_exchange_strong(&thisid, thisid))){
          return false;
        }
        ++_lock_count;
        return true;
      }
      void lock(){
        while (!try_lock()){
          _WaitPolicy();
        }
      }
      void unlock(){
        if (0==--_lock_count){
          _super_t::operator=(nullptr);
        }
      }
    };
    using recursive_spin_lock = recursive_spin_lock_base<null_wait_policy>;
  }
}