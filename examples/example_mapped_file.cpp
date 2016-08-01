/** @file
 * demonstrates memory mapped files
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>

struct person{
  int age;
  char first_name[50];
  char last_name[50];
  int ssn;

};

int main(){
  try {
    xtd::filesystem::path oFile = xtd::filesystem::path::temp_directory() + "tmp_mapped_file.dat";

    {
      xtd::mapped_file<0x1000> oMappedFile(oFile);
      auto oPerson = oMappedFile.get<person>(0);
      oPerson->age = 123;
      oPerson->ssn = 456;
      strcpy(oPerson->first_name, "Groucho");
      strcpy(oPerson->last_name, "Marx");
    }
    {
      xtd::mapped_file<0x1000> oMappedFile(oFile);
      auto oPerson = oMappedFile.get<person>(1);
      oPerson->age = 456;
      oPerson->ssn = 789;
      strcpy(oPerson->first_name, "Harpo");
      strcpy(oPerson->last_name, "Marx");
    }

    xtd::mapped_file<0x1000> oMappedFile(oFile);
    auto oGroucho = oMappedFile.get<person>(0);
    std::cout << oGroucho->first_name << " " << oGroucho->last_name << std::endl;
    auto oHarpo = oMappedFile.get<person>(1);
    std::cout << oHarpo->first_name << " " << oHarpo->last_name << std::endl;

    return 0;
  }
  catch(const xtd::exception& ex){
    std::cout << "An exception occurred: " << ex.what() << std::endl;
    return -1;
  }
}
