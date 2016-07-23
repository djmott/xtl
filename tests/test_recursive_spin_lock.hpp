#pragma once

TEST(recursive_spin_lock, initialize){
  EXPECT_NO_THROW(xtd::concurrent::recursive_spin_lock oLock);
}