/** @file
xtd::cstring system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/string.hpp>

TEST(test_string, initialization){
  xtd::cstring s("abcdefghijklmnopqrstuvwxyz");
  ASSERT_EQ(s.size(), static_cast<size_t>(26));
}

TEST(test_string, format){
  auto s = xtd::cstring::format("abcdefghijklmnopqrstuvwxyz");
  ASSERT_EQ(s.size(), static_cast<size_t>(26));
}

TEST(test_string, to_lower){
  xtd::cstring s = "ABCD";
  s.to_lower(std::locale());
  ASSERT_STREQ(s.c_str(), "abcd");
}

TEST(test_string, to_upper){
  xtd::cstring s = "abcd";
  s.to_upper(std::locale());
  ASSERT_STREQ(s.c_str(), "ABCD");
}


TEST(test_string, ltrim){
  xtd::cstring s = "        a";
  s.ltrim();
  ASSERT_EQ(s.size(), static_cast<size_t>(1));
  s = ".   .";
  ASSERT_EQ(static_cast<size_t>(5), s.ltrim().size());
}

TEST(test_string, rtrim){
  xtd::cstring s = "a     ";
  s.rtrim();
  ASSERT_EQ(s.size(), static_cast<size_t>(1));
  s = ".   .";
  ASSERT_EQ(static_cast<size_t>(5), s.rtrim().size());
}

TEST(test_string, trim){
  xtd::cstring s = "   a     ";
  s.trim();
  ASSERT_EQ(s.size(), static_cast<size_t>(1));
  s = "    .   .    ";
  ASSERT_EQ(static_cast<size_t>(5), s.trim().size());
}

TEST(test_string, find_first_in_initializer_list){
  xtd::cstring s = "This is a test X";
  ASSERT_EQ(static_cast<size_t>(0), s.find_first_of({ 'T' }));
  ASSERT_EQ(static_cast<size_t>(2), s.find_first_of({ 'i' }));
  ASSERT_EQ(static_cast<size_t>(4), s.find_first_of({ 'a', 'e', ' ' }));
  ASSERT_EQ(static_cast<size_t>(3), s.find_first_of({'s'}));
  ASSERT_EQ(s.size() - 1, s.find_first_of({ 'X', 'Y', 'Z' }));
}

TEST(test_string, find_last_in_initializer_list){
  xtd::cstring s = "This is a test X";
  ASSERT_EQ(static_cast<size_t>(0), s.find_last_of({'T'}));
  ASSERT_EQ(static_cast<size_t>(13), s.find_last_of({'t'}));
  ASSERT_EQ(static_cast<size_t>(5), s.find_last_of({'i'}));
  ASSERT_EQ(static_cast<size_t>(12), s.find_last_of({'s'}));
  ASSERT_EQ(s.size() - 1, s.find_last_of({ 'X', 'Y', 'Z' }));
}

TEST(test_string, replace){
  xtd::cstring s = "aaaaaaa";
  ASSERT_TRUE(s.size());
  s.replace({ 'a' }, ' ').trim();
  ASSERT_FALSE(s.size());
  s = "AAAaaaAAA";
  s.replace({ 'A' }, 'a');
  ASSERT_EQ(static_cast<size_t>(9), s.size());
  s.replace({ 'a' }, ' ');
  ASSERT_EQ(static_cast<size_t>(9), s.size());
  s.trim();
  ASSERT_EQ(static_cast<size_t>(0), s.size());
}

TEST(test_string, split){
  xtd::cstring s = "this is a test";
  ASSERT_EQ(static_cast<size_t>(4), s.split({ ' ' }).size());
  s = "1 2 3 4 5 6 7 8 9";
  ASSERT_EQ(static_cast<size_t>(9), s.split({ ' ' }).size());
}

TEST(test_string, string_from_wchar_ptr){
  auto s = xtd::cstring::format(L"123456789");
  ASSERT_EQ(static_cast<size_t>(9), s.size());
  ::testing::StaticAssertTypeEq<decltype(s)::value_type, char>();
}

TEST(test_string, wstring_from_char_ptr){
  xtd::cstring s = "12345";
  auto ws = xtd::wstring::format(s.c_str());
  ASSERT_EQ(ws.size(), s.size());
  ::testing::StaticAssertTypeEq<decltype(ws)::value_type, wchar_t>();
}

TEST(test_string, wstring_from_string){
  xtd::cstring s = "12345";
  auto ws = xtd::wstring::format(s);
  ASSERT_EQ(ws.size(), s.size());
}

TEST(test_string, string_from_wstring){
  xtd::wstring ws(L"123456789");
  auto s = xtd::cstring::format(ws);
  ASSERT_EQ(ws.size(), s.size());
}

