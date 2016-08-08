/** @file
xtd::executable and xtd::this_executable system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/executable.hpp>

TEST(test_executable, initialization) {
  ASSERT_NO_THROW(auto & oExec = xtd::executable::this_executable());
}

TEST(test_executable, path) {
  auto & oExec = xtd::executable::this_executable();
  ASSERT_STRNE("", oExec.path().string().c_str());
}



