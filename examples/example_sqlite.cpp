/** @file
 * demonstrates sqlite use
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

struct IUnknown;

#include <xtd/sqlite.hpp>

#include <iostream>

#include <xtd/debug.hpp>

int main(){
  using namespace xtd::sqlite;
  try{
    auto oDBFile = xtd::filesystem::temp_directory_path() + "sqlite_test.db";
    INFO("Using database file ", oDBFile.string());
    auto oDB = database::open_database(xtd::filesystem::temp_directory_path() + "sqlite_test.db", database::OpenCreate|database::ReadWrite|database::MemoryDB);

    {
      auto oTransaction = oDB->begin_transaction();
      oDB->execute("Create Table Fnord (Age Int primary key, Name String);");

      oTransaction.commit();
    }
    {
      auto oTransaction = oDB->begin_transaction();
      auto oCmd = oDB->prepare<int, xtd::string>("Insert Into Fnord (Age, Name) Values (?, ?);");
      (*oCmd)(123, "Charlie Brown");
      (*oCmd)(234, "Homer Simpson");
      (*oCmd)(345, "George Bush");
      (*oCmd)(456, "Donald Trump");
      oTransaction.commit();
    }
    {
      auto oRS = oDB->execute_reader<int, xtd::string>("Select Age, Name From Fnord;");
      while (oRS->next()){
        INFO(oRS->get<0>(), " ", oRS->get<1>());
      }
    }
    return 0;
  }
  catch (const xtd::sqlite::exception& ex){
    ERR("An xtd::sqlite::exception occurred: ", ex.what());
  }
  catch (const xtd::exception& ex){
    ERR("An xtd::exception occurred: ", ex.what());
  }
  catch (const std::exception& ex){
    ERR("An std::exception occurred: ", ex.what());
  }
  catch (...){
    ERR("An unexpected exception occurred.");
  }
  return -1;
}
