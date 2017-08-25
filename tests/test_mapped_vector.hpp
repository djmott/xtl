/** @file
xtd::mapped_vector system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/mapped_vector.hpp>
#include <xtd/unique_id.hpp>

TEST(test_mapped_vector, initialization){
  auto oPath = xtd::filesystem::temp_directory_path() /= xtd::string::format(xtd::unique_id()).c_str();
  {
    ASSERT_NO_THROW(xtd::mapped_vector<double> oDoubles(oPath));
  }
  xtd::filesystem::remove(oPath);
}

TEST(test_mapped_vector, insert){
  auto oPath = xtd::filesystem::temp_directory_path() /= xtd::string::format(xtd::unique_id()).c_str();
  {
    xtd::mapped_vector<double> oDoubles(oPath);
    for (size_t i = 1; i < 0x10000; i++){
      oDoubles.push_back(i);
      ASSERT_EQ(oDoubles.size(), i);
    }
  }
  xtd::filesystem::remove(oPath);
}

TEST(test_mapped_vector, iterate){
  auto oPath = xtd::filesystem::temp_directory_path() /= xtd::string::format(xtd::unique_id()).c_str();
  D_(DUMP(oPath.string()));
  {
    using vector_t = xtd::mapped_vector<uint64_t>;
    vector_t oLongs(oPath);
    for (uint64_t i = 1; i < 20; i++){
      oLongs.push_back(i);
    }
    size_t x = 0;
    for (vector_t::iterator oItem = oLongs.begin(); oItem != oLongs.end(); ++oItem){
      x++;
      auto t = *oItem;
      ASSERT_GT(t, static_cast<uint64_t>(0));
    }
    ASSERT_EQ(x, oLongs.size());
  }
  xtd::filesystem::remove(oPath);
}
