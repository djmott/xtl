#pragma once

#include <xtd/concurrent/recursive_spin_lock.hpp>

#include <future>

TEST(test_recursive_spin_lock, initialize){
  EXPECT_NO_THROW(xtd::concurrent::recursive_spin_lock oLock);
}

TEST(test_recursive_spin_lock, lock_unlock){
  xtd::concurrent::recursive_spin_lock oLock;
  EXPECT_NO_THROW(oLock.try_lock());
  auto fn = std::async(std::launch::async, [&](){ return oLock.try_lock(); });
  EXPECT_FALSE(fn.get());
  EXPECT_NO_THROW(oLock.unlock());
  auto fn2 = std::async(std::launch::async, [&](){ return oLock.try_lock(); });
  EXPECT_TRUE(fn2.get());
}

TEST(test_recursive_spin_lock, nested_locking){
  using sl = xtd::concurrent::recursive_spin_lock;
  sl oLock;
  {
    sl::scope_locker locker(oLock);
    auto fn = std::async(std::launch::async, [&](){ return oLock.try_lock(); });
    EXPECT_FALSE(fn.get());
    {
      sl::scope_locker locker(oLock);
      auto fn = std::async(std::launch::async, [&](){ return oLock.try_lock(); });
      EXPECT_FALSE(fn.get());

    }
  }
  auto fn2 = std::async(std::launch::async, [&](){ return oLock.try_lock(); });
  EXPECT_TRUE(fn2.get());

}
