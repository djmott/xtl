/** @file
xtd::btree system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/btree.hpp>

#include <cstdint>
#include <fstream>
#include <map>
#include <random>
#include <vector>

namespace {

  inline xtd::filesystem::path btree_temp_path(){
    static int32_t n = 0;
    return xtd::filesystem::temp_directory_path() /= xtd::string::format("btree_test_", ++n, ".dat").c_str();
  }

  /// scoped temp file that is removed on destruction so each test starts clean
  struct btree_scoped_file{
    xtd::filesystem::path _path;
    btree_scoped_file() : _path(btree_temp_path()){ xtd::filesystem::remove(_path); }
    ~btree_scoped_file(){ xtd::filesystem::remove(_path); }
    operator const xtd::filesystem::path&() const { return _path; }
  };

  inline std::uintmax_t btree_file_size(const xtd::filesystem::path& oPath){
    std::ifstream oFile(oPath.string().c_str(), std::ios::binary | std::ios::ate);
    if (!oFile) return 0;
    return static_cast<std::uintmax_t>(oFile.tellg());
  }

  struct btree_point{
    int _x;
    int _y;
    bool operator==(const btree_point& rhs) const { return _x == rhs._x && _y == rhs._y; }
  };

  // small order forces frequent node splits with only a handful of insertions
  using small_tree = xtd::btree<int, int, 4>;

}

TEST(test_btree, empty_container){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  ASSERT_EQ(oTree.size(), 0u);
  ASSERT_TRUE(oTree.empty());
  ASSERT_FALSE(oTree.contains(42));
  ASSERT_EQ(oTree.count(42), 0u);
  ASSERT_TRUE(oTree.find(42) == oTree.end());
  ASSERT_TRUE(oTree.begin() == oTree.end());
  ASSERT_THROW(oTree.at(42), std::out_of_range);
}

TEST(test_btree, single_insert_and_lookup){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  ASSERT_TRUE(oTree.insert(7, 700));
  ASSERT_EQ(oTree.size(), 1u);
  ASSERT_FALSE(oTree.empty());
  ASSERT_TRUE(oTree.contains(7));
  ASSERT_EQ(oTree.count(7), 1u);
  ASSERT_EQ(oTree.at(7), 700);
  auto it = oTree.find(7);
  ASSERT_TRUE(it != oTree.end());
  ASSERT_EQ((*it).first, 7);
  ASSERT_EQ((*it).second, 700);
  ASSERT_EQ(it->first, 7);
  ASSERT_FALSE(oTree.contains(8));
}

TEST(test_btree, duplicate_insert_rejected_assign_overwrites){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  ASSERT_TRUE(oTree.insert(1, 10));
  ASSERT_FALSE(oTree.insert(1, 999)); // duplicate key not inserted
  ASSERT_EQ(oTree.at(1), 10);
  ASSERT_EQ(oTree.size(), 1u);

  oTree.insert_or_assign(1, 111);
  ASSERT_EQ(oTree.at(1), 111);
  ASSERT_EQ(oTree.size(), 1u);

  oTree.insert_or_assign(2, 222); // acts as insert when absent
  ASSERT_EQ(oTree.at(2), 222);
  ASSERT_EQ(oTree.size(), 2u);
}

TEST(test_btree, subscript_operator){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  oTree[5] = 50;
  ASSERT_EQ(oTree.size(), 1u);
  ASSERT_EQ(static_cast<int>(oTree[5]), 50);
  oTree[5] = 55;
  ASSERT_EQ(static_cast<int>(oTree[5]), 55);
  ASSERT_EQ(oTree.size(), 1u);
  // reading an absent key inserts a default-constructed value
  ASSERT_EQ(static_cast<int>(oTree[6]), 0);
  ASSERT_TRUE(oTree.contains(6));
  ASSERT_EQ(oTree.size(), 2u);
}

TEST(test_btree, leaf_splits){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  const int N = 50;
  for (int i = 0; i < N; ++i){
    ASSERT_TRUE(oTree.insert(i, i * 3));
  }
  ASSERT_EQ(oTree.size(), static_cast<size_t>(N));
  for (int i = 0; i < N; ++i){
    ASSERT_TRUE(oTree.contains(i));
    ASSERT_EQ(oTree.at(i), i * 3);
  }
}

TEST(test_btree, multilevel_splits_reverse_insert){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  const int N = 300;
  for (int i = N - 1; i >= 0; --i){ // reverse order stresses branch splitting
    ASSERT_TRUE(oTree.insert(i, i + 1000));
  }
  ASSERT_EQ(oTree.size(), static_cast<size_t>(N));
  for (int i = 0; i < N; ++i){
    ASSERT_EQ(oTree.at(i), i + 1000);
  }
  // iteration must be in ascending key order
  int expect = 0;
  for (auto it = oTree.begin(); it != oTree.end(); ++it){
    ASSERT_EQ((*it).first, expect);
    ASSERT_EQ((*it).second, expect + 1000);
    ++expect;
  }
  ASSERT_EQ(expect, N);
}

TEST(test_btree, ordered_iteration_after_shuffled_insert){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  std::vector<int> keys;
  for (int i = 0; i < 200; ++i) keys.push_back(i);
  std::mt19937 rng(1234);
  std::shuffle(keys.begin(), keys.end(), rng);
  for (int k : keys) ASSERT_TRUE(oTree.insert(k, k * 2));

  size_t seen = 0;
  int prev = -1;
  for (auto it = oTree.begin(); it != oTree.end(); ++it){
    ASSERT_LT(prev, (*it).first);
    prev = (*it).first;
    ASSERT_EQ((*it).second, (*it).first * 2);
    ++seen;
  }
  ASSERT_EQ(seen, oTree.size());
}

TEST(test_btree, erase_basic){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  const int N = 200;
  for (int i = 0; i < N; ++i) ASSERT_TRUE(oTree.insert(i, i));

  ASSERT_EQ(oTree.erase(1000), 0u); // absent
  ASSERT_EQ(oTree.size(), static_cast<size_t>(N));

  // erase every third key
  size_t removed = 0;
  for (int i = 0; i < N; i += 3){
    ASSERT_EQ(oTree.erase(i), 1u);
    ++removed;
  }
  ASSERT_EQ(oTree.size(), static_cast<size_t>(N) - removed);

  for (int i = 0; i < N; ++i){
    if (i % 3 == 0) ASSERT_FALSE(oTree.contains(i));
    else{ ASSERT_TRUE(oTree.contains(i)); ASSERT_EQ(oTree.at(i), i); }
  }

  // iteration still ascending and consistent with size
  size_t seen = 0; int prev = -1;
  for (auto it = oTree.begin(); it != oTree.end(); ++it){ ASSERT_LT(prev, (*it).first); prev = (*it).first; ++seen; }
  ASSERT_EQ(seen, oTree.size());
}

TEST(test_btree, erase_all_then_reinsert){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  const int N = 120;
  for (int i = 0; i < N; ++i) ASSERT_TRUE(oTree.insert(i, i));
  for (int i = 0; i < N; ++i) ASSERT_EQ(oTree.erase(i), 1u);
  ASSERT_EQ(oTree.size(), 0u);
  ASSERT_TRUE(oTree.empty());
  ASSERT_TRUE(oTree.begin() == oTree.end());

  for (int i = 0; i < N; ++i) ASSERT_TRUE(oTree.insert(i, i * 10));
  ASSERT_EQ(oTree.size(), static_cast<size_t>(N));
  for (int i = 0; i < N; ++i) ASSERT_EQ(oTree.at(i), i * 10);
}

TEST(test_btree, free_list_reuse_bounds_file_growth){
  btree_scoped_file oFile;
  const int N = 300;
  std::uintmax_t sizeAfterFirstFill = 0;
  {
    small_tree oTree(oFile);
    for (int i = 0; i < N; ++i) ASSERT_TRUE(oTree.insert(i, i));
  }
  sizeAfterFirstFill = btree_file_size(oFile);
  ASSERT_GT(sizeAfterFirstFill, 0u);

  {
    small_tree oTree(oFile);
    for (int i = 0; i < N; ++i) ASSERT_EQ(oTree.erase(i), 1u);
    ASSERT_EQ(oTree.size(), 0u);
  }
  // refill: freed pages must be recycled from the embedded free list, so the
  // file must not grow beyond its previous extent
  {
    small_tree oTree(oFile);
    for (int i = 0; i < N; ++i) ASSERT_TRUE(oTree.insert(i, i));
    ASSERT_EQ(oTree.size(), static_cast<size_t>(N));
  }
  ASSERT_LE(btree_file_size(oFile), sizeAfterFirstFill);
}

TEST(test_btree, persistence_across_reopen){
  btree_scoped_file oFile;
  const int N = 300;
  {
    small_tree oTree(oFile);
    for (int i = 0; i < N; ++i) ASSERT_TRUE(oTree.insert(i, i + 5));
    ASSERT_EQ(oTree.size(), static_cast<size_t>(N));
  }
  {
    small_tree oTree(oFile); // reopen the same file
    ASSERT_EQ(oTree.size(), static_cast<size_t>(N));
    for (int i = 0; i < N; ++i){
      ASSERT_TRUE(oTree.contains(i));
      ASSERT_EQ(oTree.at(i), i + 5);
    }
    int expect = 0;
    for (auto it = oTree.begin(); it != oTree.end(); ++it){ ASSERT_EQ((*it).first, expect); ++expect; }
    ASSERT_EQ(expect, N);
  }
}

TEST(test_btree, clear_empties_container){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  for (int i = 0; i < 100; ++i) ASSERT_TRUE(oTree.insert(i, i));
  oTree.clear();
  ASSERT_EQ(oTree.size(), 0u);
  ASSERT_TRUE(oTree.empty());
  ASSERT_TRUE(oTree.begin() == oTree.end());
  ASSERT_FALSE(oTree.contains(0));
  // container remains usable after clear
  ASSERT_TRUE(oTree.insert(1, 1));
  ASSERT_EQ(oTree.size(), 1u);
}

TEST(test_btree, pod_value_type){
  btree_scoped_file oFile;
  xtd::btree<int, btree_point, 4> oTree(oFile);
  for (int i = 0; i < 40; ++i) ASSERT_TRUE(oTree.insert(i, btree_point{ i, i * i }));
  for (int i = 0; i < 40; ++i){
    btree_point p = oTree.at(i);
    ASSERT_EQ(p._x, i);
    ASSERT_EQ(p._y, i * i);
  }
}

TEST(test_btree, randomized_against_std_map){
  btree_scoped_file oFile;
  small_tree oTree(oFile);
  std::map<int, int> oOracle;
  std::mt19937 rng(987654321);
  const int KEY_RANGE = 250;
  const int OPS = 4000;

  for (int op = 0; op < OPS; ++op){
    int key = static_cast<int>(rng() % KEY_RANGE);
    if (rng() & 1){
      int val = static_cast<int>(rng());
      bool bTree = oTree.insert(key, val);
      bool bOracle = oOracle.insert(std::make_pair(key, val)).second;
      ASSERT_EQ(bTree, bOracle);
    } else {
      size_t rTree = oTree.erase(key);
      size_t rOracle = oOracle.erase(key);
      ASSERT_EQ(rTree, rOracle);
    }
    ASSERT_EQ(oTree.size(), oOracle.size());
  }

  // every key in range must agree with the oracle
  for (int k = 0; k < KEY_RANGE; ++k){
    auto oIt = oOracle.find(k);
    if (oIt == oOracle.end()){
      ASSERT_FALSE(oTree.contains(k));
    } else {
      ASSERT_TRUE(oTree.contains(k));
      ASSERT_EQ(oTree.at(k), oIt->second);
    }
  }

  // full ordered traversal must match the oracle exactly
  auto oTreeIt = oTree.begin();
  for (auto& kv : oOracle){
    ASSERT_TRUE(oTreeIt != oTree.end());
    ASSERT_EQ((*oTreeIt).first, kv.first);
    ASSERT_EQ((*oTreeIt).second, kv.second);
    ++oTreeIt;
  }
  ASSERT_TRUE(oTreeIt == oTree.end());
}
