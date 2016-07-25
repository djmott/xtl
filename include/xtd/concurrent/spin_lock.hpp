/** @file
simple user mode spin lock based on std::atomic
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


namespace xtd{
  namespace concurrent{
    ///A single owner spinning lock
    template <typename _WaitPolicyT = null_wait_policy>
    class spin_lock_base {
      std::atomic < uint32_t > _lock;
    public:
      using wait_policy_type = _WaitPolicyT;
      static const uint32_t LockedValue = 0x80000000;
      using scope_locker = xtd::concurrent::scope_locker<spin_lock_base<_WaitPolicyT>>;

      ~spin_lock_base() = default;
      spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _lock(0), _WaitPolicy(oWait){};
      spin_lock_base(const spin_lock_base&) = delete;
      spin_lock_base(spin_lock_base&&) = delete;
      ///Acquires the lock
      void lock(){
        forever{
          uint32_t compare = 0;
          if (_lock.compare_exchange_strong(compare, LockedValue)){
            break;
          }
          _WaitPolicy();
        }
      }
      /// Releases the lock
      void unlock(){
        _lock.store(0);
      }
      /** Attempts to acquire the lock
      @return true if the lock was acquired
      */
      bool try_lock(){
        uint32_t compare = 0;
        return (_lock.compare_exchange_strong(compare, LockedValue));
      }

    private:
      wait_policy_type _WaitPolicy;
    };

    using spin_lock = spin_lock_base<null_wait_policy>;
  }
}
