/** @file
 * demonstrates using the xtd::btree on-disk key-value map
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/btree.hpp>
#include <iostream>

int main(){
  auto oFile = xtd::filesystem::temp_directory_path();
  oFile /= "example_btree.dat";
  xtd::filesystem::remove(oFile);
  try{
    {
      xtd::btree<int, int> oTree(oFile);
      for (int i = 0; i < 10000; ++i){
        oTree.insert(i, 10 + i);
      }
      std::cout << "inserted " << oTree.size() << " elements" << std::endl;
      std::cout << "value at key 1234 is " << oTree.at(1234) << std::endl;

      oTree.insert_or_assign(1234, 4321);
      std::cout << "value at key 1234 is now " << oTree.at(1234) << std::endl;

      oTree.erase(0);
      std::cout << "after erase, size is " << oTree.size() << std::endl;
    }
    {
      // reopen to demonstrate persistence
      xtd::btree<int, int> oTree(oFile);
      std::cout << "reopened file, size is " << oTree.size() << std::endl;
    }
    xtd::filesystem::remove(oFile);
    return 0;
  }
  catch (const xtd::exception& ex){
    std::cout << "An xtd::exception occurred at " << ex.location().file() << "(" << ex.location().line() << ") : " << ex.what() << std::endl;
  }
  catch (const std::exception& ex){
    std::cout << "An std::exception occurred: " << ex.what() << std::endl;
  }
  catch (...){}
  xtd::filesystem::remove(oFile);
  return -1;
}
