/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#include <xtd/concurrent/hash_map.hpp>
#define CATCH_CONFIG_MAIN

#include <thread>
#include <future>

#include <catch2/catch.hpp>


TEST_CASE("size returns number of items", "[concurrent::hashmap]") {
  xtd::concurrent::hash_map<uint64_t, uint64_t> oMap;
  for (uint64_t i = 1; i < 1000; ++i) {
    REQUIRE(oMap.insert(i, i));
    REQUIRE(i == oMap.unsafe_count());
  }
}

TEST_CASE("Multiple threads can insert without collision", "[concurrent::hashmap]") {
  xtd::concurrent::hash_map<uint64_t, uint64_t> oMap;
  auto t1 = std::async(std::launch::async, [&]() {
    for (int i = 0; i < 0xfff; ++i) {
      REQUIRE(oMap.insert(i, i));
    }
  });
  auto t2 = std::async(std::launch::async, [&]() {
    for (int i = 0x1000; i < 0xfff000; ++i) {
      REQUIRE(oMap.insert(i, i));
    }
  });
  auto t3 = std::async(std::launch::async, [&]() {
    for (int i = 0x1000000; i < 0xfff000000; ++i) {
      REQUIRE(oMap.insert(i, i));
    }
  });
  t3.get();
  t2.get();
  t1.get();
}