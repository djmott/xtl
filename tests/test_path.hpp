/** @file
xtd::path system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


TEST(test_path, Initialization){
  ASSERT_NO_THROW(xtd::filesystem::path oPath);
  ASSERT_NO_THROW(xtd::filesystem::path oPath = "/a");
  ASSERT_NO_THROW(xtd::filesystem::path oPath = "/a/b");
}

TEST(test_path, append){
  xtd::filesystem::path oPath = "/a";
  ASSERT_NO_THROW(oPath.append("b"));
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), xtd::filesystem::path("/a/b").make_preferred().string().c_str());
  oPath /= "c";
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), xtd::filesystem::path("/a/b/c").make_preferred().string().c_str());
  oPath /= "d";
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), xtd::filesystem::path("/a/b/c/d").make_preferred().string().c_str());

  xtd::filesystem::path p1 = "/a/";
  ASSERT_NO_THROW(p1 += "/b/");
  ASSERT_STREQ(p1.make_preferred().string().c_str(), xtd::filesystem::path("/a/b/").make_preferred().string().c_str());

}



