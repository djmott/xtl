/** @file
 * Demonstrates the dict protocol https://tools.ietf.org/html/rfc2229
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
*/

#include <xtd/xtd.hpp>
#include <xtd/grammars/RFC1035.hpp>

int main() {
  try {
    auto oAST = xtd::parser<command_line::parameter>::parse(sParam.cbegin(), sParam.cend());
    return 0;
  }

  catch (const xtd::exception& ex) {
    ERR("An xtd::exception occurred: ", ex.what());
  }
  return -1;
}
