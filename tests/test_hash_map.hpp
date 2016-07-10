/** @file
xtd::concurrent::hash_map system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


using hash_map_type = xtd::concurrent::hash_map<int, std::string>;

TEST(test_hash_map, initialization) {
  hash_map_type oMap;
}

TEST(test_hash_map, insert){
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(123, "Hello!") );
  ASSERT_TRUE(oMap.insert(456, "Hello!") );
  ASSERT_TRUE(oMap.insert(789, "Hello!") );
  ASSERT_FALSE(oMap.insert(123, "Hello!") );
  ASSERT_FALSE(oMap.insert(456, "Hello!") );
  ASSERT_FALSE(oMap.insert(789, "Hello!") );
}

TEST(test_hash_map, remove) {
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(123, "Hello!") );
  ASSERT_TRUE(oMap.insert(456, "Hello!") );
  ASSERT_TRUE(oMap.insert(789, "Hello!") );
  ASSERT_TRUE(oMap.remove(123));
  ASSERT_TRUE(oMap.remove(456) );
  ASSERT_FALSE(oMap.remove(456) );
}

TEST(test_hash_map, DISABLED_iterator){
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(123, "Hello!"));
  ASSERT_TRUE(oMap.insert(456, "Hello!"));
  ASSERT_TRUE(oMap.insert(789, "Hello!"));
  int i = 0;
  for (auto oItem : oMap){
    ++i;
  }
  ASSERT_EQ(i, 3);
}