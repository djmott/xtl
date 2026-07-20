/** @file
lru cache system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/lru_cache.hpp>

TEST(test_lru_cache, loads_on_miss) {
  int loads = 0;
  auto loader = [&](const int& k) { ++loads; return k * 10; };
  xtd::lru_cache<int, int, 2, decltype(loader)> cache(loader);
  ASSERT_EQ(20, cache[2]);
  ASSERT_EQ(1, loads);
}

TEST(test_lru_cache, hit_does_not_reload) {
  int loads = 0;
  auto loader = [&](const int& k) { ++loads; return k * 10; };
  xtd::lru_cache<int, int, 2, decltype(loader)> cache(loader);
  ASSERT_EQ(20, cache[2]);
  ASSERT_EQ(1, loads);
  ASSERT_EQ(20, cache[2]);
  ASSERT_EQ(1, loads);
}

TEST(test_lru_cache, evicts_oldest) {
  int loads = 0;
  auto loader = [&](const int& k) { ++loads; return k * 10; };
  xtd::lru_cache<int, int, 2, decltype(loader)> cache(loader);
  ASSERT_EQ(10, cache[1]);
  ASSERT_EQ(20, cache[2]);
  ASSERT_EQ(2, loads);
  ASSERT_EQ(30, cache[3]);
  ASSERT_EQ(3, loads);
  ASSERT_EQ(10, cache[1]);
  ASSERT_EQ(4, loads);
}
