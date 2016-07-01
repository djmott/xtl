/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */


#if ((XTD_OS_MINGW | XTD_OS_UNIX) & XTD_OS)

TEST(test_path, Initialization){
  ASSERT_NO_THROW(xtd::path oPath);
  ASSERT_NO_THROW(xtd::path oPath = "/a");
  ASSERT_NO_THROW(xtd::path oPath = "/a/b");
}

TEST(test_path, append){
  xtd::path oPath = "/a";
  ASSERT_NO_THROW(oPath.append("b"));
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), xtd::path("/a/b").make_preferred().string().c_str());
  oPath /= "c";
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), xtd::path("/a/b/c").make_preferred().string().c_str());
  oPath /= "d";
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), xtd::path("/a/b/c/d").make_preferred().string().c_str());
}

#endif

