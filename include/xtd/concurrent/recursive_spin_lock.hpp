/** @file
Recursive spin lock
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once


#include <xtd/concurrent/concurrent.hpp>

namespace xtd{
  namespace concurrent {
    namespace _ {
      template<typename _wait_policy_t = null_wait_policy>
      class recursive_spin_lock_base{
        using wait_policy_type = _wait_policy_t;
        using hash_type = std::hash<std::thread::id>;
        std::atomic<size_t> _lock;
        hash_type _hash;
        uint32_t _lock_count;
        wait_policy_type _wait_policy;
      public:
        using scope_locker = xtd::concurrent::scope_locker<recursive_spin_lock_base<_wait_policy_t>>;

        ~recursive_spin_lock_base() = default;

        recursive_spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _lock(-1), _hash(), _lock_count(0), _wait_policy(oWait) {};

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
            _wait_policy();
          }
        }

        void unlock() {
          if (0 == --_lock_count) {
            _lock.store(-1);
          }
        }
      };
    }
    using recursive_spin_lock = _::recursive_spin_lock_base<null_wait_policy>;
  }
}
