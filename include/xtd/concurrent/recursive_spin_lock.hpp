/** @file
Recursive spin lock
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/concurrent/concurrent.hpp>
#include <xtd/debug.hpp>

namespace xtd{
  namespace concurrent {
    namespace _ {
      template<typename _WaitPolicyT = null_wait_policy>
      class recursive_spin_lock_base{
        using wait_policy_type = _WaitPolicyT;
        using hash_type = std::hash<std::thread::id>;
        std::atomic<size_t> _lock;
        hash_type _hash;
        uint32_t _lock_count;
        wait_policy_type _WaitPolicy;
      public:
        using scope_locker = xtd::concurrent::scope_locker<recursive_spin_lock_base<_WaitPolicyT>>;

        ~recursive_spin_lock_base() = default;

        recursive_spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _lock(-1), _lock_count(0), _WaitPolicy(oWait) {};

        recursive_spin_lock_base(const recursive_spin_lock_base &) = delete;

        recursive_spin_lock_base(recursive_spin_lock_base &&) = delete;

        bool try_lock() {
          size_t bad_id = -1;
          auto ThisID = _hash(std::this_thread::get_id());
          if (!_lock.compare_exchange_strong(bad_id, ThisID) && !(_lock.compare_exchange_strong(ThisID, ThisID))) {
            return false;
          }
          ++_lock_count;
          return true;
        }

        void lock() {
          while (!try_lock()) {
            _WaitPolicy();
          }
        }

        void unlock() {
          XTD_ASSERT(_lock.load() == _hash(std::this_thread::get_id()));
          if (0 == --_lock_count) {
            _lock.store(-1);
          }
        }
      };
    }
    using recursive_spin_lock = _::recursive_spin_lock_base<null_wait_policy>;
  }
}
