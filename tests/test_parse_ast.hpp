/** @file
xtd::parse_ast system and unit tests (separate ast_node implementation)
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/parse_ast.hpp>
#include <string>

namespace ast_test_grammar {
  AST_CHARACTER_(P, 'P');
  AST_CHARACTER_(D, 'D');
  AST_CHARACTER_(Q, 'Q');
  AST_STRING_(Snafoo);
}

namespace ast_basic_grammar {
  using namespace xtd::parse_ast;
  AST_STRING_(DIM);
  AST_CHARACTER_(EOL, '\n');
  AST_CHARACTER_(ASSIGNMENT, '=');
  AST_STRING_(INPUT);
  AST_REGEX(IDENTIFIER, "[[:alpha:]]+[[:alnum:]]*");
  AST_REGEX(numeric_literal, "\\d+(\\.\\d+)?");

  struct dim_statement : rule<dim_statement, and_<DIM, IDENTIFIER, EOL>> {};
  struct input_statement : rule<input_statement, and_<IDENTIFIER, ASSIGNMENT, INPUT>> {};
  struct statement : rule<statement, or_<dim_statement, input_statement, EOL>> {};
  struct statements : rule<statements, zero_or_more_<statement>> {};

  using WS = whitespace<' ', '\r', '\t'>;
  using start_rule = statements;
  using parser = xtd::parse_ast::parser<start_rule, true, WS>;
}

template <typename _element_t> bool ast_can_parse(std::string src) {
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  return xtd::parse_ast::parser<_element_t, true, xtd::parse_ast::whitespace<' ', '\r', '\t'>>::parse(src.cbegin(), src.cend(), ast);
}

TEST(test_parser_ast, terminals) {
  EXPECT_TRUE(ast_can_parse<ast_test_grammar::P>("P"));
  EXPECT_FALSE(ast_can_parse<ast_test_grammar::P>("x"));
  EXPECT_TRUE(ast_can_parse<ast_test_grammar::Snafoo>("Snafoo"));
  EXPECT_FALSE(ast_can_parse<ast_test_grammar::Snafoo>("Squeegy"));
  EXPECT_TRUE(ast_can_parse<ast_basic_grammar::IDENTIFIER>("abc123"));
  EXPECT_FALSE(ast_can_parse<ast_basic_grammar::IDENTIFIER>("123abc"));
  EXPECT_TRUE(ast_can_parse<ast_basic_grammar::numeric_literal>("123.45"));
}

TEST(test_parser_ast, empty_input) {
  EXPECT_TRUE(ast_can_parse<ast_basic_grammar::statements>(""));
}

TEST(test_parser_ast, ast_and_children) {
  std::string s = "PDQ";
  using rule_t = xtd::parse_ast::and_<ast_test_grammar::P, ast_test_grammar::D, ast_test_grammar::Q>;
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE(xtd::parse_ast::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  ASSERT_EQ(ast->size(), 3u);
  EXPECT_EQ((*ast)[0]->get_text(), "P");
  EXPECT_EQ((*ast)[1]->get_text(), "D");
  EXPECT_EQ((*ast)[2]->get_text(), "Q");
  EXPECT_EQ(ast->get_text(), "PDQ");
  std::string concat;
  for (auto& child : *ast) concat += std::string(child->get_text());
  EXPECT_EQ(concat, "PDQ");
}

TEST(test_parser_ast, ast_or_single_child) {
  std::string s = "D";
  using rule_t = xtd::parse_ast::or_<ast_test_grammar::P, ast_test_grammar::D, ast_test_grammar::Q>;
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE(xtd::parse_ast::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  ASSERT_EQ(ast->size(), 1u);
  EXPECT_EQ((*ast)[0]->get_text(), "D");
  EXPECT_TRUE((*ast)[0]->isa(typeid(ast_test_grammar::D)));
  EXPECT_EQ(ast->type(), typeid(rule_t));
}

TEST(test_parser_ast, ast_zero_or_more_children) {
  std::string s = "PPP";
  using rule_t = xtd::parse_ast::zero_or_more_<ast_test_grammar::P>;
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE(xtd::parse_ast::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  ASSERT_EQ(ast->size(), 3u);
  for (auto& child : *ast) EXPECT_EQ(child->get_text(), "P");
}

TEST(test_parser_ast, ast_one_or_more_nested) {
  std::string s = "PDQ";
  using namespace ast_test_grammar;
  using rule_t = xtd::parse_ast::one_or_more_<xtd::parse_ast::or_<P, D, Q>>;
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE(xtd::parse_ast::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  ASSERT_EQ(ast->size(), 3u);
  const char* expected[] = { "P", "D", "Q" };
  size_t i = 0;
  for (auto& branch : *ast) {
    ASSERT_EQ(branch->size(), 1u);
    EXPECT_EQ((*branch)[0]->get_text(), expected[i]);
    ++i;
  }
}

TEST(test_parser_ast, ast_named_rule_structure) {
  std::string s = "dim a\n";
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE((xtd::parse_ast::parser<ast_basic_grammar::dim_statement, true, ast_basic_grammar::WS>::parse(s.cbegin(), s.cend(), ast)));
  ASSERT_TRUE(ast);
  EXPECT_TRUE(ast->isa(typeid(ast_basic_grammar::dim_statement)));
  // dim_statement -> and_<DIM, IDENTIFIER, EOL>
  ASSERT_EQ(ast->size(), 1u);
  auto& seq = (*ast)[0];
  ASSERT_EQ(seq->size(), 3u);
  EXPECT_EQ((*seq)[0]->get_text(), "dim");
  EXPECT_EQ((*seq)[1]->get_text(), "a");
}

TEST(test_parser_ast, ast_parent_link) {
  std::string s = "PD";
  using rule_t = xtd::parse_ast::and_<ast_test_grammar::P, ast_test_grammar::D>;
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE(xtd::parse_ast::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_EQ(ast->size(), 2u);
  auto child = (*ast)[0];
  auto parent = child->parent.lock();
  ASSERT_TRUE(parent);
  EXPECT_EQ(parent.get(), ast.get());
}

TEST(test_parser_ast, statements_program) {
  std::string s = "dim a\ndim b\n";
  std::shared_ptr<xtd::parse_ast::ast_node> ast;
  ASSERT_TRUE(ast_basic_grammar::parser::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  // statements -> zero_or_more_<statement>; two matched statements
  ASSERT_EQ(ast->size(), 1u);
  auto& rep = (*ast)[0];
  EXPECT_EQ(rep->size(), 2u);
}
