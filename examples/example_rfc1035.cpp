/** @file
 * Demonstrates RFC 1035 domain-name parsing https://tools.ietf.org/html/rfc1035
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
*/

#include <iostream>
#include <string>
#include <xtd/xtd.hpp>
#include <xtd/grammars/RFC1035.hpp>
#include <xtd/debug.hpp>

static void print_ast(const xtd::parse::rule_base::pointer_type& node, int depth) {
  if (!node) return;
  std::cout << std::string(static_cast<size_t>(depth) * 2, ' ') << node->type().name();
  auto text = node->get_text();
  if (!text.empty()) std::cout << " = \"" << std::string(text) << "\"";
  std::cout << "\n";
  for (auto& child : *node) print_ast(child, depth + 1);
}

int main(int argc, char* argv[]) {
  try {
    std::string input = (argc > 1) ? argv[1] : "www.example.com";
    std::shared_ptr<xtd::Grammars::RFC1035::domain> ast;
    using parser_t = xtd::parser<xtd::Grammars::RFC1035::domain, false, xtd::parse::whitespace<>>;
    if (!parser_t::parse(input.cbegin(), input.cend(), ast)) {
      ERR("Failed to parse domain: ", input);
      return 1;
    }
    std::cout << "Parsed domain \"" << input << "\":\n";
    print_ast(ast, 1);
    return 0;
  }
  catch (const xtd::exception& ex) {
    ERR("An xtd::exception occurred: ", ex.what());
  }
  return -1;
}
