/** \file example_callback.hpp
    demonstrates using the xtd::callback mechanism
    \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd.hpp>
/*
#include <xtd/callback.hpp>
#include <iostream>
*/

using callback_type = xtd::callback<void(int, int)>;
callback_type oCallback2;



void Add(int x, int y){ std::cout << x + y << std::endl; }
void Subtract(int x, int y){ std::cout << x - y << std::endl; }
void Multiply(int x, int y){ std::cout << x * y << std::endl; }

int main(){

  oCallback2.connect(&Add);
  oCallback2.connect(&Subtract);
  oCallback2.connect(&Multiply);

  oCallback2(5, 7);


  return 0;
}
