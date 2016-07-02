/**
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
\file demonstrates a simple command line parser
 */

#include <xtd.hpp>
/*

#include <xtd/parse.hpp>
#include <iostream>
*/

namespace command_line {
  using namespace xtd::parse;

//termnals
  STRING(red, "red");
  STRING(green, "green");
  STRING(blue, "blue");
  STRING(one, "1");
  STRING(three, "3");
  STRING(five, "5");
  STRING(dash_color, "--color=");
  STRING(dash_prime, "--prime=");
//rules
  using rgb = or_<red, green, blue>;
  using prime_num = or_<one, three, five>;
  using color_param = and_<dash_color, rgb>;
  using prime_param = and_<dash_prime, prime_num>;
  using parameter = or_<color_param, prime_param>;
}

int main(int argc, char * argv[]){
  std::string sParam = argv[1];
  auto oAST = xtd::parser<command_line::parameter>::parse(sParam.cbegin(), sParam.cend());
  if (!oAST){
    //parse failed, show usage or error
    return 1;
  }else{
    //work with parsed parameters
    return 0;
  }
}
