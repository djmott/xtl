/** @file
 * demonstrates the alternative separate-AST parser (xtd::parse_ast).
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0.
 */

#include <iostream>
#include <string>
#include <memory>
#include <xtd/parse_ast.hpp>

namespace command_line {
  using namespace xtd::parse_ast;

  AST_STRING(red, "red");
  AST_STRING(green, "green");
  AST_STRING(blue, "blue");
  AST_STRING(dash_color, "--color=");
  AST_STRING(dash_prime, "--prime=");
  AST_REGEX(number, "[[:digit:]]+");

  using rgb = or_<red, green, blue>;
  using color_param = and_<dash_color, rgb>;
  using prime_param = and_<dash_prime, number>;
  using parameter = or_<color_param, prime_param>;
}

// Recursively prints an ast_node and its children.
static void print_ast(const std::shared_ptr<xtd::parse_ast::ast_node>& node, int depth) {
  if (!node) return;
  std::cout << std::string(static_cast<size_t>(depth) * 2, ' ') << node->type().name();
  if (!node->get_text().empty()) std::cout << " = \"" << std::string(node->get_text()) << "\"";
  std::cout << "\n";
  for (auto& child : *node) print_ast(child, depth + 1);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <parameter>\n"
      << "Example: --color=red   or   --prime=7\n";
    return 1;
  }

  std::string sParam = argv[1];
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  xtd::parse_ast::parse_error<std::string::const_iterator>::vector errors;

  using parser_t = xtd::parse_ast::parser<command_line::parameter>;
  if (!parser_t::parse(sParam.cbegin(), sParam.cend(), ast, errors)) {
    std::cerr << "Parse failed for input: " << sParam << "\n";
    for (auto& err : errors) {
      auto posOffset = std::distance(sParam.cbegin(), err->position);
      std::cerr << "  Error: rule " << err->failed_rule.name()
        << " at position " << posOffset << "\n";
    }
    return 1;
  }

  std::cout << "Parse succeeded for input: " << sParam << "\nAST:\n";
  print_ast(ast, 1);
  return 0;
}
