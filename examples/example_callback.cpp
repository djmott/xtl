/** \file example_callback.hpp
    demonstrates using the xtd::callback mechanism
    @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>


// Create a few callback objects with the invocation signature of int(int, int) 
xtd::callback<int(int, int)> oAdd;
xtd::callback<int(int, int)> oSubtract;
xtd::callback<int(int, int)> oMultiply;


//receiver static function
int Adder(int x, int y){ return x + y; }

//class with receiver member function
struct Multiplier{
  int Multiply(int x, int y){ return x*y; }
};


int main(){

  //attach Adder static function to oAdd object using += notation
  oAdd += &Adder;

  std::cout << "Calling Adder(5, 7) through oAdd callback returns : " << oAdd(5, 7) << std::endl;
  
  //attach a lambda to oSubtract using .connect() method
  oSubtract.connect([](int x, int y){ return x - y; });

  std::cout << "Calling lambda(15, 4) through oSubtract callback returns : " << oSubtract(15, 4) << std::endl;

  //create an instance of multipler
  Multiplier oMultipler;
  
  //attach multipler instance to oMultiply using .connect() method. This requires the class member to invoke and the instance to invoke it on
  oMultiply.connect<Multiplier, &Multiplier::Multiply>(&oMultipler);

  std::cout << "Calling oMultipler.Multiply(6,3) through oMultiply callback returns : " << oMultiply(6, 3) << std::endl;
  return 0;
}
