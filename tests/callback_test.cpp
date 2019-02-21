/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#include <xtd/callback.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

bool fn_invoked = false;
int fn_val = 0;
int mem_val = 0;

void void_fn() { fn_invoked = true; }

int fn_dest(int x, int y) { fn_invoked = true; return (fn_val = x * y); }

struct evt_dest{
  void void_mem(){ fn_invoked = true; }
  int mem_dest(int x, int y) { fn_invoked = true; return (mem_val = 2 * x*y); }
};

SCENARIO("callback used in code", "[callback]") {
  GIVEN("supported use") {
    REQUIRE(std::is_default_constructible<xtd::callback<void()>>::value);
    REQUIRE(std::is_default_constructible<xtd::callback<int(int)>>::value);
    REQUIRE(!std::is_copy_constructible<xtd::callback<int()>>::value);
    REQUIRE(!std::is_copy_assignable<xtd::callback<int()>>::value);
    REQUIRE(std::is_move_constructible<xtd::callback<int()>>::value);
    REQUIRE(std::is_move_assignable<xtd::callback<int()>>::value);
    WHEN("default constructed") {
      xtd::callback<void()> lambda_call;
      xtd::callback<void()> function_call;
      xtd::callback<void()> member_call;
      THEN("can be invoked") {
        REQUIRE_NOTHROW(lambda_call());
      }
      THEN("can attach a lambda with connect") {
        lambda_call.connect([]() {});
      }
      THEN("can attach a static function with connect") {
        function_call.connect(void_fn);
      }
      THEN("can attach a class member with connect") {
        evt_dest otmp;
        member_call.connect<evt_dest, &evt_dest::void_mem>(&otmp);
      }
      THEN("can attach a lambda with operator+=") {
        lambda_call += ([]() {});
      }
      THEN("can attach a static function with operator+=") {
        function_call += (void_fn);
      }

    }
    WHEN("attached to a destination") {
      THEN("invoking the callback calls the target destination") {
        xtd::callback<int(int, int)> oCall;
        int lam_val = 0;
        evt_dest oDest;
        oCall.connect(&fn_dest);
        oCall.connect([&lam_val](int x, int y) { return (lam_val = 3 * x * 4); });
        oCall.connect<evt_dest, &evt_dest::mem_dest>(&oDest);
        lam_val = fn_val = mem_val = 0;
        oCall(3, 4);
        REQUIRE(36 == lam_val);
        REQUIRE(12 == fn_val);
        REQUIRE(24 == mem_val);
      }
    }
  }
}