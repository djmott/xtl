/** @file
 * demonstrates a simple command line parser
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0.
 */

#include <iostream>
#include <xtd/parse.hpp>

namespace command_line {
  using namespace xtd::parse;

  // Terminals
  STRING(red, "red");
  STRING(green, "green");
  STRING(blue, "blue");
  STRING(one, "1");
  STRING(three, "3");
  STRING(five, "5");
  STRING(dash_color, "--color=");
  STRING(dash_prime, "--prime=");

  // Rules
  using rgb = or_<red, green, blue>;
  using prime_num = or_<one, three, five>;
  using color_param = and_<dash_color, rgb>;
  using prime_param = and_<dash_prime, prime_num>;
  using parameter = or_<color_param, prime_param>;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <parameter>\n"
      << "Example: --color=red   or   --prime=3\n";
    return 1;
  }

  std::string sParam = argv[1];

  // AST pointer and error list
  command_line::parameter::pointer_type oAST;
  xtd::parse::parse_error<std::string::const_iterator>::vector errors;

  // The parser type alias
  using parser_t = xtd::parser<command_line::parameter>;

  bool success = parser_t::parse(sParam.cbegin(), sParam.cend(), oAST, errors);

  if (!success) {
    std::cerr << "Parse failed for input: " << sParam << "\n";

    // Display errors
    for (auto& err : errors) {
      auto posOffset = std::distance(sParam.cbegin(), err->position);
      std::cerr << "  Error: rule " << err->failed_rule.name()
        << " at position " << posOffset << "\n";
    }
    return 1;
  }

  std::cout << "Parse succeeded for input: " << sParam << "\n";

  // Example: show the top-level rule type
  if (oAST) {
    std::cout << "AST root type: " << oAST->type().name() << "\n";
  }

  return 0;
}
