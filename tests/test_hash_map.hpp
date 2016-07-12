/** @file
xtd::concurrent::hash_map system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


using hash_map_type = xtd::concurrent::hash_map<uint16_t, std::string>;

TEST(test_hash_map, initialization) {
  hash_map_type oMap;
}

TEST(test_hash_map, insert){
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(1, "Hello!") );
  ASSERT_TRUE(oMap.insert(2, "Hello!") );
  ASSERT_TRUE(oMap.insert(3, "Hello!") );
  ASSERT_FALSE(oMap.insert(1, "Hello!") );
  ASSERT_FALSE(oMap.insert(2, "Hello!") );
  ASSERT_FALSE(oMap.insert(3, "Hello!") );
}

TEST(test_hash_map, remove) {
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(1, "Hello!") );
  ASSERT_TRUE(oMap.insert(4, "Hello!") );
  ASSERT_TRUE(oMap.insert(7, "Hello!") );
  ASSERT_TRUE(oMap.remove(1));
  ASSERT_TRUE(oMap.remove(4) );
  ASSERT_FALSE(oMap.remove(8) );
}


TEST(test_hash_map, exists) {
  hash_map_type oMap;
  oMap.insert(0x1234, "0x1234");
  oMap.insert(0x4321, "0x4321");
  ASSERT_TRUE(oMap.exists(0x1234) );
  ASSERT_FALSE(oMap.exists(0x7890) );
  ASSERT_TRUE(oMap.exists(0x4321) );
}

TEST(test_hash_map_iterator, initialization){
  hash_map_type oMap;
  {
    auto oIt = oMap.begin();
    auto oEnd = oMap.end();
    auto oBack = oMap.back();
  }
  {
    hash_map_type::iterator_type oEnd;
  }
}

TEST(test_hash_map_iterator, comparison){
  hash_map_type oMap;
  oMap.insert(0x1234, "0x1234");
  oMap.insert(0x4321, "0x4321");
  {
    auto o1 = oMap.begin();
    auto o2 = oMap.begin();
    ASSERT_EQ(o1, o2);
    ASSERT_EQ(o1, oMap.begin());
  }
  {
    auto o1 = oMap.back();
    auto o2 = oMap.back();
    ASSERT_EQ(o1, o2);
    ASSERT_EQ(o1, oMap.back());
  }
  {
    auto o1 = oMap.end();
    auto o2 = oMap.end();
    ASSERT_EQ(o1, o2);
    ASSERT_EQ(o1, oMap.end());
  }
  {
    ASSERT_NE(oMap.begin(), oMap.end());
    ASSERT_NE(oMap.begin(), oMap.back());
    ASSERT_NE(oMap.back(), oMap.end());
  }
}

TEST(test_hash_map_iterator, range_for){
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(0x0001, "0x0001"));
  ASSERT_TRUE(oMap.insert(0x0010, "0x0010"));
  ASSERT_TRUE(oMap.insert(0x0100, "0x0100"));
  ASSERT_TRUE(oMap.insert(0x1000, "0x1000"));
  int i = 0;
  for (auto oItem = oMap.begin(); oMap.end() != oItem; ++oItem){
    ++i;
  }
  ASSERT_EQ(i, 4);
}
TEST(test_hash_map_iterator, inc_dec){
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(0x0001, "0x0001"));
  ASSERT_TRUE(oMap.insert(0x0010, "0x0010"));
  ASSERT_TRUE(oMap.insert(0x0100, "0x0100"));
  ASSERT_TRUE(oMap.insert(0x1000, "0x1000"));

  auto o1 = oMap.begin();
  auto o2 = o1;
  ++o1;
  o2++;
  ASSERT_EQ(o1, o2);
  ++o1;
  o2++;
  ASSERT_EQ(o1, o2);
  ++o1;
  o2++;
  ASSERT_EQ(o1, o2);
  ASSERT_EQ(o1, oMap.back());
  --o1;
  o2--;
  ASSERT_EQ(o1, o2);
  --o1;
  o2--;
  --o1;
  o2--;
  ASSERT_EQ(o1, o2);
}
