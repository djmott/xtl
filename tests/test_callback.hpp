/** @file
xtd::callback system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/callback.hpp>

/// callback object initialization test
TEST(test_callback, initialization){
  ASSERT_NO_THROW(xtd::callback<void()> c1);
  ASSERT_NO_THROW(xtd::callback<void(int)> c2);
  ASSERT_NO_THROW(xtd::callback<int(int)> c3);
}

/// callback invokation test with no receivers
TEST(test_callback, invoke_with_no_receiver){

  xtd::callback<void()> c1;
  ASSERT_NO_THROW(c1());

  xtd::callback<void(int)> c2;
  ASSERT_NO_THROW(c2(0));

  xtd::callback<void(int, int, int)> c3;
  ASSERT_NO_THROW(c3(4, 5, 6));
}

/// callback test invokation to lambda test
TEST(test_callback, invoke_lambda){
  bool bSuccess = false;
  xtd::callback<void()> c1;
  c1.connect([&bSuccess]() { bSuccess = true; });
  c1();
  ASSERT_TRUE(bSuccess);
}

/// callback test invokation to lambda with parameter
TEST(test_callback, pass_parameter_to_lambda){
  bool bSuccess = false;
  xtd::callback<void(bool)> c1;
  c1.connect([&bSuccess](bool newval) { bSuccess = newval; });
  c1(true);
  ASSERT_TRUE(bSuccess);
}

int g_var;

void StaticCallback(int x){
  g_var = x;
}



/// callback test invoke static method
 TEST(test_callback, static_method_callback){
  xtd::callback<void(int)> c1;
  c1.connect<&StaticCallback>();
  for (int i = 0; i<10; i++){
    c1(i);
    ASSERT_EQ(i, g_var);
  }
}

/// callback test invoke class member
TEST(test_callback, class_member_callback){
  struct source{
    source() : s1(), s2(){}
    xtd::callback<void()> s1;
    xtd::callback<void(int, int)> s2;
  };
  struct dest{
    dest() : _d1called(false), _total(0){}
    // cppcheck-suppress unusedFunction
    void d1(){ _d1called = true; }
    // cppcheck-suppress unusedFunction
    void d2(int x, int y){ _total = x + y; }
    bool _d1called;
    int _total;
  };
  source oSource;
  dest oDest;
  oSource.s1.connect<dest, &dest::d1>(&oDest);
  oSource.s2.connect<dest, &dest::d2>(&oDest);
  oSource.s1();
  ASSERT_EQ(true, oDest._d1called);
  oSource.s2(5, 10);
  ASSERT_EQ(15, oDest._total);
}

/// callback test invoke multiple attached lambdas
TEST(test_callback, multiple_labda_destinations){
  int x = 0;
  xtd::callback<void()> c1;
  c1.connect([&x]() { x++; });
  c1.connect([&x]() { x++; });
  c1.connect([&x]() { x++; });
  c1.connect([&x]() { x++; });
  c1.connect([&x]() { x++; });
  c1();
  ASSERT_EQ(x, 5);
}

/// callback test empty invoker with return type throws exception
TEST(test_callback, empty_invokers_with_return_type_throws){
  xtd::callback<int()> c1;
  ASSERT_THROW(c1(), std::runtime_error);
}

/// callback test result policy return first
TEST(test_callback, result_policy_return_first){
  xtd::callback<int()> c;
  c.connect([](){ return 1; });
  c.connect([](){ return 2; });
  c.connect([](){ return 3; });
  ASSERT_EQ(c(xtd::callback<int()>::result_policy::return_first), 1);
}

/// callback test result policy return last
TEST(test_callback, result_policy_return_last){
  xtd::callback<int()> c;
  c.connect([](){ return 1; });
  c.connect([](){ return 2; });
  c.connect([](){ return 3; });
  ASSERT_EQ(c(xtd::callback<int()>::result_policy::return_last), 3);
}

/// callback test result policy with parameters
TEST(test_callback, result_policy_with_parameters){
  xtd::callback<int(int, int)> c;
  c.connect([](int x, int y){ return x + y; });
  c.connect([](int x, int y){ return x * y; });
  c.connect([](int x, int y){ return x - y; });
  ASSERT_EQ(c(xtd::callback<int(int, int)>::result_policy::return_first, 5, 3), 8);
  ASSERT_EQ(c(xtd::callback<int(int, int)>::result_policy::return_last, 5, 3), 2);
}

/// callback test result policy with empty callback throws
TEST(test_callback, result_policy_empty_throws){
  xtd::callback<int()> c;
  ASSERT_THROW(c(xtd::callback<int()>::result_policy::return_first), std::runtime_error);
  ASSERT_THROW(c(xtd::callback<int()>::result_policy::return_last), std::runtime_error);
}
