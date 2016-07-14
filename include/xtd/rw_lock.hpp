/** @file
Simple multi reader/single writer spin lock
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace concurrent{
    /** A multiple reader/single writer spin lock
    supports 2^31 simultaneous readers
    @tparam _WaitPolicyT behavior when spinning
    */
    template <typename _WaitPolicyT = null_wait_policy>
    class rw_lock_base : std::atomic<uint32_t>{
    public:
      using wait_policy_type = _WaitPolicyT;
      rw_lock_base(wait_policy_type oWait = wait_policy_type()) : _super_t(0), _WaitPolicy(oWait){}
      ///Returns the number of active read locks
      uint32_t readers() const{
        return _super_t::load() & read_bits_mask;
      }

      ///Frees the write lock or decrements the reader count
      void unlock(){
        forever{
          auto iOriginal = _super_t::load();
          if (write_lock_bit == iOriginal){
            _super_t::store(0);
            break;
          } else{
            if (_super_t::compare_exchange_strong(iOriginal, iOriginal - 1)){
              break;
            }
          }
          _WaitPolicy.spin();
        }
      }
      /// Acquires a shared read lock
      void lock_read(){
        forever{
          auto iOriginal = _super_t::load() & read_bits_mask;
          if (_super_t::compare_exchange_strong(iOriginal, 1 + iOriginal)){
            break;
          }
          _WaitPolicy.spin();
        }
      }
      /** tries to acquire a shared read lock
      @return true if the lock was acquired
      */
      bool try_lock_read(){
        auto iOriginal = _super_t::load() & read_bits_mask;
        return _super_t::compare_exchange_strong(iOriginal, 1 + iOriginal);
      }
      ///acquires a write lock for exclusive access
      void lock_write(){
        uint32_t iOriginal = 0;
        while (!_super_t::compare_exchange_strong(iOriginal, write_lock_bit)){
          _WaitPolicy.spin();
        }
      }
      /** attempts to acquire a write lock for exclusive access
      @returns true if the lock was acquired
      */
      bool try_lock_write(){
        uint32_t iOriginal = 0;
        return _super_t::compare_exchange_strong(iOriginal, write_lock_bit);
      }
      /// RAII pattern to acquire and release a read lock
      class scope_read{
        rw_lock_base& _Lock;
      public:
        explicit scope_read(rw_lock_base& oLock) : _Lock(oLock){
          _Lock.lock_read();
        }
        ~scope_read(){
          _Lock.unlock();
        }
      };
      /// RAII pattern to acquire and release a write lock
      class scope_write{
        rw_lock_base& _Lock;
      public:
        explicit scope_write(rw_lock_base& oLock) : _Lock(oLock){
          _Lock.lock_write();
        }
        ~scope_write(){
          _Lock.unlock();
        }
      };
    private:
      using _super_t = std::atomic<uint32_t>;
      static const uint32_t write_lock_bit = 0x80000000;
      static const uint32_t read_bits_mask = ~write_lock_bit;
      wait_policy_type _WaitPolicy;
    };

    using rw_lock = rw_lock_base<null_wait_policy>;
  }
}
