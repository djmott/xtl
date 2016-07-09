/** @file
xtd::source_location system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


TEST(test_source_location, initialization){
  auto a = here();
  ASSERT_EQ(__LINE__-1, a.line());
  ASSERT_STREQ(__FILE__, a.file());
  auto b = here();
  ASSERT_STREQ(a.file(), b.file());
  xtd::source_location c(a);
}

TEST(test_source_location, assignment) {
  auto a = here();
  auto b = here();
  b = a;
  ASSERT_EQ(a.line(), b.line());
  ASSERT_STREQ(a.file(), b.file());
}

TEST(test_source_location, comparison) {
  auto a = here();
  auto b = here();
  ASSERT_FALSE(a == b);
  ASSERT_TRUE( a < b);
  b = a;
  ASSERT_TRUE(a == b);
}
