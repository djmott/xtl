/**
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


using hash_map_type = xtd::concurrent_hash_map<int, std::string>;

TEST(test_hash_map, initialization){
  hash_map_type oMap;
  ASSERT_TRUE(oMap.insert(123, "Hello!") );
}
