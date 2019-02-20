#include <xtd/concurrent/stack.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <thread>
#include <chrono>
#include <vector>

SCENARIO("concurrent stack used in code") {
  GIVEN("supported use") {
    REQUIRE(std::is_default_constructible<xtd::concurrent::stack<int>>::value);
    REQUIRE(std::is_default_constructible<xtd::concurrent::stack<int>>::value);
    REQUIRE(!std::is_copy_constructible<xtd::concurrent::stack<int>>::value);
    REQUIRE(!std::is_copy_assignable<xtd::concurrent::stack<int>>::value);
    REQUIRE(std::is_move_constructible<xtd::concurrent::stack<int>>::value);
    REQUIRE(std::is_move_assignable<xtd::concurrent::stack<int>>::value);
    WHEN("An empty stack is created") {
      xtd::concurrent::stack<int> s1;
      THEN("pop returns false") {
        int i;
        REQUIRE(false == s1.try_pop(i));
      }
      THEN("items can be pushed and popped in order") {
        int x;
        for (int i = 0; i < 10; ++i) { s1.push(i); }
        for (int i = 9; i >= 9; --i) {
          REQUIRE(s1.try_pop(x));
          REQUIRE(x == i);
        }
      }
    }
    WHEN("two threads each push 10000 items") {
      xtd::concurrent::stack<int> s1;
      {
        std::thread t1([&s1]() { for (int x = 0; x < 10000; ++x) { s1.push(x); } });
        std::thread t2([&s1]() { for (int x = 0; x < 10000; ++x) { s1.push(x); } });
        t1.join();
        t2.join();
      }
      THEN("stack contains 20000 items") { 
        REQUIRE(20000 == s1.unsafe_count()); 
      }
    }
    WHEN("multiple threads are pushing and popping for 5 seconds") {
      xtd::concurrent::stack<size_t> s1;
      {
        constexpr int thread_cnt = 4;
        std::vector<std::thread> threads;
        bool _quit = false;
        std::hash<std::thread::id> oHasher;
        for (int i = 0; i < thread_cnt; ++i) {
          threads.emplace_back([&]() { 
            size_t x;
            for (int t = 0; !_quit; ++t) {
              s1.push(oHasher(std::this_thread::get_id()));
              s1.try_pop(x);
            }
            s1.push(oHasher(std::this_thread::get_id()));
          });
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        _quit = true;
        for (auto & item : threads) {
          item.join();
        }
        THEN("collision doesnt occur") {
          std::map<size_t, size_t> unique_val_counts;
          size_t x;
          while (s1.try_pop(x)) {
            unique_val_counts[x]++;
          }
          REQUIRE(unique_val_counts.size() == thread_cnt);
        }
      }
    }
  }
}