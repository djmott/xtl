/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

TEST(test_exception, Initialization){
  EXPECT_THROW(xtd::exception::throw_if(false, [](bool){ return true; }), xtd::exception);
}

TEST(test_exception, copy_construct){
  xtd::exception ex1(here(), "This is a test");
  xtd::exception ex2(ex1);
  ASSERT_EQ(ex1.location(), ex2.location());
  ASSERT_STREQ(ex1.what(), ex2.what());
}

