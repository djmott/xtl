/** @file
 * demonstrates sqlite use
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/sqlite.hpp>

#include <iostream>

int main(){
  using namespace xtd::sqlite;
  try{
    auto oDB = database::open_database(xtd::filesystem::temp_directory_path() + "sqlite_test.db");
    return 0;
  }
  catch (const xtd::sqlite::exception& ex){
    std::cout << "An xtd::sqlite::exception occurred: " << ex.what() << std::endl;
  }
  catch (const xtd::exception& ex){
    std::cout << "An xtd::exception occurred: " << ex.what() << std::endl;
  }
  catch (const std::exception& ex){
    std::cout << "An std::exception occurred: " << ex.what() << std::endl;
  }
  catch (...){
    std::cout << "An unexpected exception occurred." << std::endl;
  }
  return -1;
}
