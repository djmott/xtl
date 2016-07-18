/** @file
base exception system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


TEST(test_exception, Initialization){
  xtd::exception ex1(here(), "");
  xtd::crt_exception(here(), "");
}

TEST(test_exception, copy_construct){
  xtd::exception ex1(here(), "This is a test");
  xtd::exception ex2(ex1);
  ASSERT_EQ(ex1.location(), ex2.location());
  ASSERT_STREQ(ex1.what(), ex2.what());
  xtd::crt_exception crtex1(here(), "");
  xtd::crt_exception crtex2(crtex1);
  ASSERT_EQ(crtex1.location(), crtex2.location());
  ASSERT_STREQ(crtex1.what(), crtex2.what());

}

TEST(test_exception, move_construct){
  xtd::exception ex1(here(), "");
  xtd::exception ex2(std::move(ex1));
  xtd::crt_exception crtex1(here(), "");
  xtd::crt_exception crtex2(std::move(crtex1));
}



TEST(test_exception, fail_throw_if){
  EXPECT_ANY_THROW(xtd::exception::throw_if(false, [](bool){ return true;}) );
  EXPECT_ANY_THROW(xtd::crt_exception::throw_if(false, [](bool){ return true;}) );
}


TEST(test_exception, no_throw_if){
  ASSERT_TRUE(xtd::exception::throw_if(true, [](bool){ return false;}) );
  ASSERT_TRUE(xtd::crt_exception::throw_if(true, [](bool){ return false;}) );
  ASSERT_FALSE(xtd::exception::throw_if(false, [](bool){ return false;}) );
  ASSERT_FALSE(xtd::crt_exception::throw_if(false, [](bool){ return false;}) );
}
