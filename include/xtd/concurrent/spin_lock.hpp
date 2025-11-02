/** @file
simple user mode spin lock based on std::atomic
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/concurrent/concurrent.hpp>

#include <atomic>

namespace xtd{
  namespace concurrent{
    ///A single owner spinning lock
    template <typename _WaitPolicyT = null_wait_policy>
    class spin_lock_base {
      std::atomic < uint32_t > _lock;
    public:
      using wait_policy_type = _WaitPolicyT;
      static constexpr uint32_t LockedValue = 0x80000000;
      using scope_locker = xtd::concurrent::scope_locker<spin_lock_base<_WaitPolicyT>>;

      ~spin_lock_base() = default;
      spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _lock(0), _WaitPolicy(oWait){};
      spin_lock_base(const spin_lock_base&) = delete;
      spin_lock_base(spin_lock_base&&) = delete;
      /** @brief Acquires the lock (blocks until available)
       * 
       * Spins in a loop until the lock is successfully acquired.
       * Uses the wait policy between spin attempts.
       */
      void lock() {
        forever{
          uint32_t compare = 0;
          if (_lock.compare_exchange_strong(compare, LockedValue, std::memory_order_acquire)){
            break;
          }
          _WaitPolicy();
        }
      }
      
      /** @brief Releases the lock
       * 
       * Releases the lock, allowing other threads to acquire it.
       */
      void unlock() {
        _lock.store(0, std::memory_order_release);
      }
      
      /** @brief Attempts to acquire the lock without blocking
       * @return true if the lock was acquired, false otherwise
       */
      [[nodiscard]] bool try_lock() {
        uint32_t compare = 0;
        return (_lock.compare_exchange_strong(compare, LockedValue, std::memory_order_acquire));
      }

    private:
      wait_policy_type _WaitPolicy;
    };

    using spin_lock = spin_lock_base<null_wait_policy>;
  }
}
