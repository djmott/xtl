/** @file
Recursive spin lock
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{
  namespace concurrent {
    namespace _ {
      template<typename _WaitPolicyT = null_wait_policy>
      class recursive_spin_lock_base : private std::atomic<size_t> {
        using _super_t = std::atomic<size_t>;
        using wait_policy_type = _WaitPolicyT;
        using hash_type = std::hash<std::thread::id>;
        hash_type _hash;
        uint32_t _lock_count;
        wait_policy_type _WaitPolicy;
      public:
        using scope_locker = xtd::concurrent::scope_locker<recursive_spin_lock_base<_WaitPolicyT>>;

        ~recursive_spin_lock_base() = default;

        recursive_spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _super_t(-1), _WaitPolicy(oWait) {};

        recursive_spin_lock_base(const recursive_spin_lock_base &) = delete;

        recursive_spin_lock_base(recursive_spin_lock_base &&) = delete;

        bool try_lock() {
          size_t bad_id = -1;
          auto ThisID = _hash(std::this_thread::get_id());
          if (!compare_exchange_strong(bad_id, ThisID) && !(compare_exchange_strong(ThisID, ThisID))) {
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
          assert(load() == _hash(std::this_thread::get_id()));
          if (0 == --_lock_count) {
            _super_t::store(-1);
          }
        }
      };
    }
    using recursive_spin_lock = _::recursive_spin_lock_base<null_wait_policy>;
  }
}
