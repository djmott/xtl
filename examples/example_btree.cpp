/** @file
 * demonstrates using the xtd::btree
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/btree.hpp>
#include <xtd/executable.hpp>

int main(){
  auto oFile = xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename();
  oFile += ".dat";
  try{
    xtd::btree<int, int> oTree(oFile);
    for (int i = 0; i < 10000 && oTree.insert(i, i); i++);
  }
  catch (...){}

  xtd::filesystem::remove(oFile);
  return 0;
}
