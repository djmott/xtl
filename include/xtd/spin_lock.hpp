/** @file
simple user mode spin lock based on std::atomic
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


namespace xtd{
  namespace concurrent{
    ///A single owner spinning lock
    template <typename _WaitPolicyT = null_wait_policy>
    class spin_lock_base : private std::atomic < uint32_t >{
    public:
      using _super_t = std::atomic < uint32_t >;
      using wait_policy_type = _WaitPolicyT;
      static const uint32_t LockedValue = 0x80000000;

      ~spin_lock_base() = default;
      spin_lock_base(wait_policy_type oWait = wait_policy_type()) : _super_t(0), _WaitPolicy(oWait){};
      spin_lock_base(const spin_lock_base&) = delete;
      spin_lock_base(spin_lock_base&&) = delete;
      ///Acquires the lock
      void lock(){
        forever{
          uint32_t compare = 0;
          if (compare_exchange_strong(compare, LockedValue)){
            break;
          }
          _WaitPolicy.spin();
        }
      }
      /// Releases the lock
      void unlock(){
        static_cast<_super_t&>(*this) = 0;
      }
      /** Attempts to acquire the lock
      @return true if the lock was acquired
      */
      bool try_lock(){
        uint32_t compare = 0;
        return (compare_exchange_strong(compare, LockedValue));
      }

      ///RAII pattern to automatically acquire and release the spin lock
      class scope_lock{
        spin_lock_base& _Lock;
      public:
        ~scope_lock(){ _Lock.unlock(); }
        explicit scope_lock(spin_lock_base& oLock) : _Lock(oLock){ _Lock.lock(); }
        scope_lock(const scope_lock&) = delete;
        scope_lock& operator=(const scope_lock&) = delete;
      };
    private:
      wait_policy_type _WaitPolicy;
    };

    using spin_lock = spin_lock_base<null_wait_policy>;
  }
}