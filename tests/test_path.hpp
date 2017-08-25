/** @file
xtd::path system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <fstream>
#include <xtd/filesystem.hpp>

#define ASSERT_PATH_EQ(p1, p2) ASSERT_STREQ(p1.make_preferred().string().c_str(), p2.make_preferred().string().c_str());
#define ASSERT_PATH_NE(p1, p2)  ASSERT_STRNE(p1.make_preferred().string().c_str(), p2.make_preferred().string().c_str());

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
  ASSERT_PATH_EQ(oPath, path("/a/b"));
  oPath /= "c";
  ASSERT_PATH_EQ(oPath, path("/a/b/c"));
  oPath /= "d";
  ASSERT_PATH_EQ(oPath, path("/a/b/c/d"));
  auto oPath2 = path("/a/b/c") /= path("d");
  ASSERT_PATH_EQ(oPath, oPath2);
}

TEST(test_path, append_fail){
  using namespace xtd::filesystem;
  path p1 = "/a/";
  ASSERT_NO_THROW(p1 += "/b/");
  ASSERT_PATH_NE(p1, path("/a/b"));

}

TEST(test_path, remove_filename){
  using namespace xtd::filesystem;
  path oPath="/a/b/cdef";
  ASSERT_PATH_EQ(oPath.remove_filename(),  path("/a/b"));
}

/*
TEST(test_path, filename){
  using namespace xtd::filesystem;
  auto s = xtd::xstring<path::value_type>::format("fnord");
  auto p1 = temp_directory_path() /= s;
  ASSERT_PATH_EQ(p1, path(s));
}
*/

TEST(test_path, replace_filename){
  using namespace xtd::filesystem;
  path p1("/a/b/c/fnord");
  p1.replace_filename("snafoo");
  ASSERT_PATH_EQ(p1, path("/a/b/c/snafoo"));
}

TEST(test_path, operator_plus){
  using namespace xtd::filesystem;
  path p1("/a/b");
  path p2("/c/d");
  auto p3 = p1 /= p2;
  ASSERT_PATH_EQ(p3, path("/a/b/c/d"));
}

TEST(test_path, operator_diveq){
  using namespace xtd::filesystem;
  path p1("/a/b");  
  p1 /= path("/c/d");
  ASSERT_PATH_EQ(p1, path("/a/b/c/d"));
}


TEST(test_path, remove){
  using namespace xtd::filesystem;
  auto oTmpFile = temp_directory_path() /= "test_path_remove";
  {
    std::ofstream of;
    of.open(oTmpFile.string());
  }
  remove(oTmpFile);
}
