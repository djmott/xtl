/** @file
Simple multi reader/single writer spin lock
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  class read_write_lock : std::atomic<uint32_t>{
    using _super_t = std::atomic<uint32_t>;
    static const uint32_t write_lock_bit = 0x80000000;
    static const uint32_t read_bits_mask = ~write_lock_bit;
  public:

    read_write_lock() : _super_t(0){}

    uint32_t readers() const {
      return _super_t::load() & read_bits_mask;
    }

    void unlock(){
      while (true){
        auto iOriginal = _super_t::load();
        if (write_lock_bit == iOriginal){
          _super_t::store(0);
          break;
        }else{
          if (_super_t::compare_exchange_strong(iOriginal, iOriginal-1)){
            break;
          }
        }
      }
    }
    void lock_read(){
      while (true){
        auto iOriginal = _super_t::load() & read_bits_mask;
        if (_super_t::compare_exchange_strong(iOriginal, 1+iOriginal)){
          break;
        }
      }
    }
    bool try_lock_read(){
      auto iOriginal = _super_t::load() & read_bits_mask;
      return _super_t::compare_exchange_strong(iOriginal, 1+iOriginal);
    }
    void lock_write(){
      uint32_t iOriginal = 0;
      while (!_super_t::compare_exchange_strong(iOriginal, write_lock_bit)){
        std::this_thread::yield();
      }
    }
    bool try_lock_write(){
      uint32_t iOriginal = 0;
      return _super_t::compare_exchange_strong(iOriginal, write_lock_bit);
    }

    class scope_read{
      read_write_lock& _Lock;
    public:
      scope_read(read_write_lock& oLock) : _Lock(oLock){
        _Lock.lock_read();
      }
      ~scope_read(){
        _Lock.unlock();
      }
    };

    class scope_write{
      read_write_lock& _Lock;
    public:
      scope_write(read_write_lock& oLock) : _Lock(oLock){
        _Lock.lock_write();
      }
      ~scope_write(){
        _Lock.unlock();
      }
    };

  };
}
