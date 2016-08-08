/** @file
xtd::path system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/filesystem.hpp>

TEST(test_path, Initialization){
  using namespace xtd::filesystem;
  ASSERT_NO_THROW(path oPath);
  ASSERT_NO_THROW(path oPath = "/a");
  ASSERT_NO_THROW(path oPath = "/a/b");
}

TEST(test_path, append){
  using namespace xtd::filesystem;
  path oPath = "/a";
  ASSERT_NO_THROW(oPath.append("b"));
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), path("/a/b").make_preferred().string().c_str());
  oPath /= "c";
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), path("/a/b/c").make_preferred().string().c_str());
  oPath /= "d";
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), path("/a/b/c/d").make_preferred().string().c_str());
  auto oPath2 = path("/a/b/c") + path("d");
  ASSERT_STREQ(oPath.make_preferred().string().c_str(), oPath2.make_preferred().string().c_str());
}

TEST(test_path, DISABLED_append_fail){
  using namespace xtd::filesystem;
  path p1 = "/a/";
  ASSERT_NO_THROW(p1 += "/b/");
  ASSERT_STREQ(path("/a/b/").make_preferred().string().c_str(), p1.make_preferred().string().c_str());

}

TEST(test_path, remove_filename){
  using namespace xtd::filesystem;
  path oPath="/a/b/c";
  ASSERT_STREQ(oPath.remove_filename().make_preferred().string().c_str(),  path("/a/b/").make_preferred().string().c_str());
}

TEST(test_path, filename){
  using namespace xtd::filesystem;
  auto s = xtd::xstring<path::value_type>::format("fnord");
  auto p1 = temp_directory_path() + s;
  ASSERT_STREQ(p1.filename().c_str(), s.c_str());

}
