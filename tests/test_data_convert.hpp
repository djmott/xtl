/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */



TEST(test_data_convert, lonibble){
  ASSERT_EQ(0x1, xtd::lonibble(0x21));
}
TEST(test_data_convert, hinibble){
  ASSERT_EQ(0x2, xtd::hinibble(0x21));
}
TEST(test_data_convert, lobyte){
  ASSERT_EQ(0x10, xtd::lobyte(0x2010));
}
TEST(test_data_convert, hibyte){
  ASSERT_EQ(0x20, xtd::hibyte(0x2010));
}
TEST(test_data_convert, loword){
  ASSERT_EQ(0x2010, xtd::loword(0x80402010));
}
TEST(test_data_convert, hiword){
  ASSERT_EQ(0x8040, xtd::hiword(0x80402010));
}

TEST(test_data_convert, lodword){
  ASSERT_EQ(0x80402010, xtd::lodword(0xd0c0b0a080402010));
}
TEST(test_data_convert, hidword){
  ASSERT_EQ(0xd0c0b0a0, xtd::hidword(0xd0c0b0a080402010));
}
