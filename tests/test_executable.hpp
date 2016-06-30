/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/executable.hpp>

TEST(test_executable, get_path){
  xtd::path oPath;
  ASSERT_NO_THROW(oPath = xtd::this_executable::get_path());
  ASSERT_STRNE("", oPath.string().c_str());
}



