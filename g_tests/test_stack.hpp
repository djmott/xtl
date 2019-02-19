/** @file
xtd::concurrent::stack system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <future>

#include <xtd/concurrent/stack.hpp>

TEST(test_stack, initialization){
  EXPECT_NO_THROW(xtd::concurrent::stack<int> oStack);
}

TEST(test_stack, push){
  xtd::concurrent::stack<int> oStack;
  for (int i = 0; i < 1000; i++){
    EXPECT_NO_THROW(oStack.push(i));
  }
}


TEST(test_stack, try_pop){
  xtd::concurrent::stack<int> oStack;
  for (int i = 0; i < 1000; i++){
    EXPECT_NO_THROW(oStack.push(i));
  }
  int x=0;
  for (int i = 0; i < 1000; i++){
    EXPECT_TRUE(oStack.try_pop(x));
  }
}

TEST(test_stack, concurrent_push_pop){
  xtd::concurrent::stack<int> oStack;
  auto pushfn = [&]() -> bool{
    for (int i = 0; i < 10000; i++){
      EXPECT_NO_THROW(oStack.push(i));
    }
    return true;
  };
  auto t1 = std::async(std::launch::async, pushfn);
  auto t2 = std::async(std::launch::async, pushfn);
  auto t3 = std::async(std::launch::async, pushfn);
  auto t4 = std::async(std::launch::async, pushfn);

  EXPECT_TRUE(t1.get() && t2.get() && t3.get() && t4.get());

  auto popfn = [&]()->bool{
    for (int i = 0; i < 10000; i++){
      EXPECT_NO_THROW(oStack.pop());
    }
    return true;
  };
  auto t5 = std::async(std::launch::async, popfn);
  auto t6 = std::async(std::launch::async, popfn);
  auto t7 = std::async(std::launch::async, popfn);
  auto t8 = std::async(std::launch::async, popfn);
  EXPECT_TRUE(t5.get() && t6.get() && t7.get() && t8.get());
}
