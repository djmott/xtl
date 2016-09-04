/** @file
xtd::concurrent::read_write_lock system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/concurrent/rw_lock.hpp>

TEST(test_rw_lock, initialization){
  xtd::concurrent::rw_lock oLock;
  oLock.lock_read();
  oLock.unlock();
  oLock.lock_write();
  oLock.unlock();
}


TEST(test_rw_lock, readers){
  xtd::concurrent::rw_lock oLock;
  oLock.lock_read();
  ASSERT_EQ(oLock.readers(), 1);
  oLock.lock_read();
  ASSERT_EQ(oLock.readers(), 2);
  oLock.lock_read();
  ASSERT_EQ(oLock.readers(), 3);
  oLock.unlock();
  ASSERT_EQ(oLock.readers(), 2);
  oLock.unlock();
  ASSERT_EQ(oLock.readers(), 1);
  oLock.unlock();
  ASSERT_EQ(oLock.readers(), 0);
}

TEST(test_rw_lock, try_read){
  xtd::concurrent::rw_lock oLock;
  oLock.try_lock_read();
  ASSERT_EQ(oLock.readers(), 1);
  oLock.try_lock_read();
  ASSERT_EQ(oLock.readers(), 2);
  oLock.try_lock_read();
  ASSERT_EQ(oLock.readers(), 3);
  oLock.unlock();
  ASSERT_EQ(oLock.readers(), 2);
  oLock.unlock();
  ASSERT_EQ(oLock.readers(), 1);
  oLock.unlock();
  ASSERT_EQ(oLock.readers(), 0);
}

TEST(test_rw_lock, scope_read){
  xtd::concurrent::rw_lock rw;
  {
    xtd::concurrent::rw_lock::scope_read oLock(rw);
    {
      xtd::concurrent::rw_lock::scope_read oLock(rw);
      {
        xtd::concurrent::rw_lock::scope_read oLock(rw);
        ASSERT_EQ(rw.readers(), 3);
      }
      ASSERT_EQ(rw.readers(), 2);
    }
    ASSERT_EQ(rw.readers(), 1);
  }
  ASSERT_EQ(rw.readers(), 0);
}

TEST(test_rw_lock, writer){
  xtd::concurrent::rw_lock rw;
  rw.lock_write();
  ASSERT_FALSE(rw.try_lock_read());
  ASSERT_FALSE(rw.try_lock_write());
}

TEST(test_rw_lock, try_lock_write){
  xtd::concurrent::rw_lock rw;
  ASSERT_TRUE(rw.try_lock_write());
  ASSERT_FALSE(rw.try_lock_write());
  ASSERT_FALSE(rw.try_lock_read());
  rw.unlock();
  ASSERT_TRUE(rw.try_lock_write());
  ASSERT_FALSE(rw.try_lock_write());
  ASSERT_FALSE(rw.try_lock_read());
}

TEST(test_rw_lock, scope_write) {
  xtd::concurrent::rw_lock rw;
  {
    xtd::concurrent::rw_lock::scope_write oLock(rw);
    ASSERT_FALSE(rw.try_lock_read());
    ASSERT_FALSE(rw.try_lock_write());
  }
  ASSERT_TRUE(rw.try_lock_write());
}
