#pragma once

#include <xtd/concurrent/recursive_spin_lock.hpp>

TEST(recursive_spin_lock, initialize){
  EXPECT_NO_THROW(xtd::concurrent::recursive_spin_lock oLock);
}

TEST(recursive_spin_lock, lock_unlock){
  xtd::concurrent::recursive_spin_lock oLock;
  EXPECT_NO_THROW(oLock.try_lock());
  std::async(std::launch::async, [&](){ EXPECT_FALSE(oLock.try_lock()); });

}
