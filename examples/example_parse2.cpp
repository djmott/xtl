/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 \example
 */

#include <xtd/parse.hpp>
#include <iostream>

namespace command_line {
  using namespace xtd::parse;
  STRING(number, "--number:");
  STRING(_1, "1");
  STRING(_3, "3");
  STRING(_5, "5");
  STRING(_7, "7");
  using numbers = or_<_1, _3, _5, _7>;
  using number_param = and_<number, numbers>;

  STRING(color, "--color:");
  STRING_(red);
  STRING_(blue)
  STRING_(green);
  using colors = or_<red, blue, green>;
  using color_param = and_<color, colors >;

  using parameter = or_<color_param , number_param>;

}


int main(int argc, char * argv[]){
  for (int i=1 ; i<argc ; ++i){
    std::string sArg = argv[i];
    auto oCommandLine = xtd::parser<command_line::parameter>::parse(sArg.begin(), sArg.end());
    if (!oCommandLine) continue;
  }

  return argc;
}
