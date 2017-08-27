/** @file
* demonstrates vector on a memory mapped file
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/mapped_vector.hpp>

struct person{
  int age;
  char first_name[50];
  char last_name[50];
  int ssn;

};

int main(){
  try{
    xtd::filesystem::path oFile = xtd::filesystem::temp_directory_path() /= "tmp_mapped_vector.dat";

    xtd::mapped_vector<int> oInts(oFile);

    for (int i=0 ; i<10000000 ; i++){
      oInts.push_back(i);
    }

    return 0;
  }
  catch (const xtd::crt_exception& ex){
    ERR("A CRT exception ", ex.errnum(), " occurred: ", ex.what());
    return -1;
  }
  catch (const xtd::exception& ex){
    ERR("An exception occurred: ", ex.what());
    return -1;
  }
}
