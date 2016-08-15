/** @file
 * demonstrates using the xtd::btree
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#if 1
int main(){}
#else
#include <xtd/btree.hpp>
#include <xtd/executable.hpp>
#include <iostream>

int main(){
  auto oFile = xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename();
  oFile += ".dat";
  try{
    xtd::btree<int, int> oTree(oFile);
    for (int i = 0; i < 10000 && oTree.insert(i, i); i++);
    return 0;
  }
  catch(const xtd::exception& ex){
    std::cout << "An xtd::exception occurred at " << ex.location().file() << "(" << ex.location().line() << ") : " << ex.what() << std::endl;
  }
  catch(const std::exception& ex){
    std::cout << "An std::exception occurred: " << ex.what() << std::endl;
  }
  catch (...){}
  return -1;
//  xtd::filesystem::remove(oFile);
}
#endif
