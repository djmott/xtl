/** @file
xtd::unique_id system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/unique_id.hpp>
#include <xtd/string.hpp>

TEST(test_unique_id, initialization){
  ASSERT_NO_THROW(xtd::unique_id oID);
}

TEST(test_unique_id, copy_compare){
  xtd::unique_id id1;
  xtd::unique_id id2(id1);
  ASSERT_FALSE(id1 < id2);
  ASSERT_FALSE(id2 < id1);
}

TEST(test_unique_id, assign){
  xtd::unique_id id1;
  xtd::unique_id id2 = id1;
  ASSERT_FALSE(id1 < id2);
  ASSERT_FALSE(id2 < id1);
}

TEST(test_unique_id, nullid){
  ASSERT_NO_THROW(xtd::unique_id::nullid());
}
