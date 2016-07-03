/** @file
  Simple user mode spin lock based on std::atomic
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


namespace xtd {

  class spin_lock : private std::atomic < uint32_t > {
  public:
    using _super_t = std::atomic < uint32_t > ;
    static const uint32_t LockedValue = 0x80000000;

    ~spin_lock() = default;
    spin_lock() : _super_t(0){};
    spin_lock(const spin_lock&) = delete;
    spin_lock(spin_lock&&) = delete;

    void lock() {
      for (;;) {
        uint32_t compare = 0;
        if (compare_exchange_strong(compare, LockedValue)) break;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
    }
    void unlock() {
      static_cast<_super_t&>(*this) = 0;
    }
    bool try_lock() {
      uint32_t compare = 0;
      return (compare_exchange_strong(compare, LockedValue));
    }

    class scope_lock {
      spin_lock& _Lock;
    public:
      ~scope_lock() { _Lock.unlock(); }
      explicit scope_lock(spin_lock& oLock) : _Lock(oLock) { _Lock.lock(); }
      scope_lock(const scope_lock&) = delete;
      scope_lock& operator=(const scope_lock&) = delete;
    };
  };

}

