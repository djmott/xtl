/**
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

TEST(test_spin_lock, initialization){
  xtd::spin_lock oLock;

}


TEST(test_spin_lock, lock){
  xtd::spin_lock oLock;
  oLock.lock();
}

TEST(test_spin_lock, unlock){
  xtd::spin_lock oLock;
  oLock.lock();
  oLock.unlock();
}

TEST(test_spin_lock, try_lock){
  xtd::spin_lock oLock;
  EXPECT_TRUE(oLock.try_lock());
  EXPECT_FALSE(oLock.try_lock());
  oLock.unlock();
  EXPECT_TRUE(oLock.try_lock());
}
