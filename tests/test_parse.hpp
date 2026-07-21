/** @file
xtd::parse system and unit tests (in-place AST implementation)
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/parse.hpp>
#include <string>

namespace test_grammar{
  CHARACTER_(P, 'P');
  CHARACTER_(D, 'D');
  CHARACTER_(Q, 'Q');
  STRING_(Snafoo);
  STRING_(ABC);
  STRING_(XYZ);
  REGEX(Alphabet, "ABC[[:digit:]]+");
}


namespace basic_grammar {
  using namespace xtd::parse;
  STRING_(IF);
  STRING_(THEN);
  STRING_(ELSE);
  STRING_(ENDIF);
  STRING_(INPUT);
  STRING_(PRINT);
  STRING_(DO);
  STRING_(UNTIL);
  STRING_(WHILE);
  STRING_(WEND);
  STRING_(DIM);
  CHARACTER_(EOL, '\n');
  CHARACTER_(ASSIGNMENT, '=');
  CHARACTER_(PLUS, '+');
  CHARACTER_(MINUS, '-');
  CHARACTER_(BIT_AND, '&');
  CHARACTER_(BIT_OR, '|');
  CHARACTER_(BIT_NOT, '~');
  CHARACTER_(LT, '<');
  CHARACTER_(GT, '>');
  CHARACTER_(MUL, '*');
  CHARACTER_(DIV, '/');
  CHARACTER_(EXP, '^');
  CHARACTER_(OPENPARAN, '(');
  CHARACTER_(CLOSEPARAN, ')');
  STRING(EQ, "==");
  STRING(GE, ">=");
  STRING(LE, "<=");
  STRING(NE, "<>");
  REGEX(IDENTIFIER, "[[:alpha:]]+[[:alnum:]]*");
  REGEX(string_literal, "\\\"[[:print:]]*\\\"");
  REGEX(numeric_literal, "\\d+(\\.\\d+)?");

  struct statements;
  struct expression;
  struct add_expression;
  struct mul_expression;
  struct neg_expression;
  struct value_expression;

  struct if_statement : rule<if_statement,
    or_<
      and_< IF, expression, THEN, EOL, statements, ENDIF, EOL >,
      and_< IF, expression, THEN, EOL, statements, ELSE, if_statement >,
      and_< IF, expression, THEN, EOL, statements, ELSE, EOL, statements, ENDIF, EOL>
    > > { template <typename ... _arg_ts> if_statement(_arg_ts&&...oArgs) : rule(oArgs...){} };

  struct do_statement : rule<do_statement,
    or_<
    and_< DO, UNTIL, expression, EOL, statements, WEND, EOL>,
    and_< DO, WHILE, expression, EOL, statements, WEND, EOL>,
    and_< DO, EOL, statements, UNTIL, expression, EOL>,
    and_< DO, EOL, statements, WHILE, expression, EOL>
  > >{ template <typename ... _arg_ts> do_statement(_arg_ts&&...oArgs) : rule(oArgs...) {} };

  struct input_statement : rule<input_statement, and_<IDENTIFIER, ASSIGNMENT, INPUT> > {
    template <typename ... _arg_ts> input_statement(_arg_ts&&...oArgs) : rule(oArgs...) {}
  };

  struct print_statement : rule<print_statement, and_<PRINT, expression, EOL> > {
    template <typename ... _arg_ts> print_statement(_arg_ts&&...oArgs) : rule(oArgs...) {}
  };

  struct dim_statement : rule<dim_statement, and_<DIM, IDENTIFIER, EOL> > {
    template <typename ... _arg_ts> dim_statement(_arg_ts&&...oArgs) : rule(oArgs...) {}
  };

  struct statement : rule<statement, or_<dim_statement, if_statement, do_statement, input_statement, print_statement, EOL> > {
    template <typename ... _arg_ts> statement(_arg_ts&&...oArgs) : rule(oArgs...) {}
  };

  struct statements : rule<statements, zero_or_more_<statement> > {
    template <typename ... _arg_ts> statements(_arg_ts&&...oArgs) : rule(oArgs...) {}
  };

  struct expression : rule<expression, or_< 
    and_<expression, GT, add_expression>,
    and_<expression, LT, add_expression>,
    and_<expression, GE, add_expression>,
    and_<expression, LE, add_expression>,
    and_<expression, EQ, add_expression>,
    and_<expression, NE, add_expression>,
    add_expression
  > >{ template <typename ... _arg_ts> expression(_arg_ts&&...oArgs) : rule(oArgs...) {} };

  struct add_expression : rule<add_expression, or_<
    and_<add_expression, PLUS, mul_expression>,
    and_<add_expression, MINUS, mul_expression>,
    and_<add_expression, BIT_AND, mul_expression>,
    and_<add_expression, BIT_OR, mul_expression>,
    and_<add_expression, BIT_NOT, mul_expression>,
    mul_expression
  > > { template <typename ... _arg_ts> add_expression(_arg_ts&&...oArgs) : rule(oArgs...) {} };

  struct mul_expression : rule<mul_expression, or_<
    and_<mul_expression, MUL, neg_expression>,
    and_<mul_expression, DIV, neg_expression>,
    and_<mul_expression, EXP, neg_expression>,
    neg_expression
  > > { template <typename ... _arg_ts> mul_expression(_arg_ts&&...oArgs) : rule(oArgs...) {} };

  struct neg_expression : rule<neg_expression, or_<
    and_<MINUS, value_expression>,
    value_expression
  > > { template <typename ... _arg_ts> neg_expression(_arg_ts&&...oArgs) : rule(oArgs...) {} };

  struct value_expression : rule<value_expression, or_<
    IDENTIFIER,
    string_literal,
    numeric_literal,
    and_< OPENPARAN, expression, CLOSEPARAN >    
  > > { template <typename ... _arg_ts> value_expression(_arg_ts&&...oArgs) : rule(oArgs...) {} };

  using WS = whitespace<' ', '\r', '\t'>;
  using start_rule = statements;
  using parser = xtd::parser<start_rule, true, WS>;

}

template <typename _element_t> bool can_parse(std::string src) {
  std::shared_ptr<_element_t> ast;
  return xtd::parser<_element_t, true, xtd::parse::whitespace<' ', '\r', '\t'>>::parse(src.cbegin(), src.cend(), ast);
}

TEST(test_parser, basic_identifier){
  EXPECT_TRUE(can_parse<basic_grammar::IDENTIFIER >("x"));
}

TEST(test_parser, basic_empty_input) {
  EXPECT_TRUE(can_parse<basic_grammar::statements>(""));
}

TEST(test_parser, basic_language_elements) {
  EXPECT_FALSE(can_parse<basic_grammar::IF>("fnord"));
  EXPECT_TRUE(can_parse<basic_grammar::IF>("IF"));
  EXPECT_TRUE(can_parse<basic_grammar::THEN>("THEN"));
  EXPECT_TRUE(can_parse<basic_grammar::ELSE>("ELSE"));
  EXPECT_TRUE(can_parse<basic_grammar::ENDIF>("ENDIF"));
  EXPECT_TRUE(can_parse<basic_grammar::INPUT>("INPUT"));
  EXPECT_TRUE(can_parse<basic_grammar::PRINT>("PRINT"));
  EXPECT_TRUE(can_parse<basic_grammar::DO>("DO"));
  EXPECT_TRUE(can_parse<basic_grammar::UNTIL>("UNTIL"));
  EXPECT_TRUE(can_parse<basic_grammar::WHILE>("WHILE"));
  EXPECT_TRUE(can_parse<basic_grammar::WEND>("WEND"));
  EXPECT_TRUE(can_parse<basic_grammar::DIM>("DIM"));
  EXPECT_TRUE(can_parse<basic_grammar::EOL>("\n"));
  EXPECT_TRUE(can_parse<basic_grammar::ASSIGNMENT>("="));
  EXPECT_TRUE(can_parse<basic_grammar::PLUS>("+"));
  EXPECT_TRUE(can_parse<basic_grammar::MINUS>("-"));
  EXPECT_TRUE(can_parse<basic_grammar::BIT_AND>("&"));
  EXPECT_TRUE(can_parse<basic_grammar::BIT_OR>("|"));
  EXPECT_TRUE(can_parse<basic_grammar::BIT_NOT>("~"));
  EXPECT_TRUE(can_parse<basic_grammar::LT>("<"));
  EXPECT_TRUE(can_parse<basic_grammar::GT>(">"));
  EXPECT_TRUE(can_parse<basic_grammar::MUL>("*"));
  EXPECT_TRUE(can_parse<basic_grammar::DIV>("/"));
  EXPECT_TRUE(can_parse<basic_grammar::EXP>("^"));
  EXPECT_TRUE(can_parse<basic_grammar::OPENPARAN>("("));
  EXPECT_TRUE(can_parse<basic_grammar::CLOSEPARAN>(")"));
  EXPECT_TRUE(can_parse<basic_grammar::EQ>("=="));
  EXPECT_TRUE(can_parse<basic_grammar::GE>(">="));
  EXPECT_TRUE(can_parse<basic_grammar::LE>("<="));
  EXPECT_TRUE(can_parse<basic_grammar::NE>("<>"));
  EXPECT_TRUE(can_parse<basic_grammar::IDENTIFIER>("abc"));
  EXPECT_FALSE(can_parse<basic_grammar::IDENTIFIER>("123abc"));
  EXPECT_TRUE(can_parse<basic_grammar::IDENTIFIER>("abc123"));
  EXPECT_TRUE(can_parse<basic_grammar::string_literal>("\"\""));
  EXPECT_TRUE(can_parse<basic_grammar::string_literal>("\"asdf\""));
  EXPECT_TRUE(can_parse<basic_grammar::string_literal>("\"asdf 1234 jghjh\""));
  EXPECT_FALSE(can_parse<basic_grammar::string_literal>("123abc"));
  EXPECT_TRUE(can_parse<basic_grammar::numeric_literal>("123"));
  EXPECT_TRUE(can_parse<basic_grammar::numeric_literal>("123.345"));
  EXPECT_FALSE(can_parse<basic_grammar::numeric_literal>("abc"));
}


TEST(test_parser, basic_dim_statement) {
  EXPECT_TRUE(can_parse<basic_grammar::dim_statement>("dim a\n"));
}

TEST(test_parser, basic_dim_statements) {
  std::string s = "dim a\ndim b\ndim c\n";
  std::shared_ptr<basic_grammar::start_rule> ast;
  EXPECT_TRUE(basic_grammar::parser::parse(s.cbegin(), s.cend(), ast));
}

TEST(test_parser, basic_input_statement) {
  std::string s = "xx123=input\n";
  std::shared_ptr<basic_grammar::start_rule> ast;
  EXPECT_TRUE(basic_grammar::parser::parse(s.cbegin(), s.cend(), ast));
  s = "xx123=input()\n";
  EXPECT_FALSE(basic_grammar::parser::parse(s.cbegin(), s.cend(), ast));
}

TEST(test_parser, character_case){
  std::string s = "P";
  using test_parse = xtd::parser<test_grammar::P>;
  std::shared_ptr<test_grammar::P> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
  s = "x";
  EXPECT_FALSE(test_parse::parse(s.cbegin(), s.cend(), ast));
}

TEST(test_parser, string_case){
  std::string s = "Snafoo";
  using test_parse = xtd::parser<test_grammar::Snafoo>;
  std::shared_ptr<test_grammar::Snafoo> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
  s = "Snafoooo";
  EXPECT_FALSE(test_parse::parse(s.cbegin(), s.cend(), ast));
  s = "snafoo";
  EXPECT_FALSE(test_parse::parse(s.cbegin(), s.cend(), ast));
}

TEST(test_parser, rule_and){
  std::string s = "PDQ";
  using rule_t = xtd::parse::and_<test_grammar::P, test_grammar::D, test_grammar::Q>;
  using test_parse = xtd::parser<rule_t>;
  std::shared_ptr<rule_t> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
  s = "QDP";
  EXPECT_FALSE(test_parse::parse(s.cbegin(), s.cend(), ast));
}


TEST(test_parser, rule_or){
  std::string s = "P";
  using rule_t = xtd::parse::or_<test_grammar::P, test_grammar::D, test_grammar::Q>;
  using test_parse = xtd::parser<rule_t>;
  std::shared_ptr<rule_t> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
  s = "XYZ";
  EXPECT_FALSE(test_parse::parse(s.cbegin(), s.cend(), ast));
}

TEST(test_parser, rule_zero_or_more){
  std::string s= "PPP";
  using rule_t = xtd::parse::zero_or_more_<test_grammar::P>;
  using test_parse = xtd::parser<rule_t>;
  std::shared_ptr<rule_t> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
  s = "XYZ";
  EXPECT_FALSE(test_parse::parse(s.cbegin(), s.cend(), ast));
}



TEST(test_parser, rule_zero_or_one){
  std::string s= "PD";
  using namespace xtd::parse;
  using namespace test_grammar;
  std::shared_ptr<zero_or_one_<P>> ast1;
  EXPECT_FALSE(xtd::parser<zero_or_one_<P>>::parse(s.cbegin(), s.cend(), ast1));
  using rule_t = and_<zero_or_one_<P>, zero_or_one_<P>, zero_or_one_<P>>;
  using grammar = xtd::parser<rule_t>;
  std::shared_ptr<rule_t> ast2;
  EXPECT_FALSE(grammar::parse(s.cbegin(), s.cend(), ast2));
  using rule2_t = and_<zero_or_one_<P>, zero_or_one_<D>>;
  using grammar2 = xtd::parser<rule2_t>;
  std::shared_ptr<rule2_t> ast3;
  EXPECT_TRUE(grammar2::parse(s.cbegin(), s.cend(), ast3));
}

TEST(test_parser, isa){
  std::string s = "PDQ";
  using namespace test_grammar;
  using pdq = xtd::parse::one_or_more_<xtd::parse::or_<P, D, Q>>;
  using test_parse = xtd::parser<pdq>;
  std::shared_ptr<pdq> oAST;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), oAST));
  EXPECT_TRUE(oAST);
  EXPECT_TRUE(oAST->isa(typeid(pdq)));
  ASSERT_EQ(oAST->type(), typeid(pdq));
}

// --- AST construction / iteration ---------------------------------------------

TEST(test_parser, ast_and_children) {
  std::string s = "PDQ";
  using rule_t = xtd::parse::and_<test_grammar::P, test_grammar::D, test_grammar::Q>;
  std::shared_ptr<rule_t> ast;
  ASSERT_TRUE(xtd::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  // a matched and_ has one child per sequence element, in order
  ASSERT_EQ(ast->size(), 3u);
  EXPECT_EQ((*ast)[0]->get_text(), "P");
  EXPECT_EQ((*ast)[1]->get_text(), "D");
  EXPECT_EQ((*ast)[2]->get_text(), "Q");
  EXPECT_EQ(ast->get_text(), "PDQ");
  // range-for iteration over children
  std::string concat;
  for (auto& child : *ast) concat += std::string(child->get_text());
  EXPECT_EQ(concat, "PDQ");
}

TEST(test_parser, ast_or_single_child) {
  std::string s = "D";
  using rule_t = xtd::parse::or_<test_grammar::P, test_grammar::D, test_grammar::Q>;
  std::shared_ptr<rule_t> ast;
  ASSERT_TRUE(xtd::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  // a matched or_ holds exactly one child: the alternative that matched
  ASSERT_EQ(ast->size(), 1u);
  EXPECT_EQ((*ast)[0]->get_text(), "D");
  EXPECT_TRUE((*ast)[0]->isa(typeid(test_grammar::D)));
}

TEST(test_parser, ast_zero_or_more_children) {
  std::string s = "PPP";
  using rule_t = xtd::parse::zero_or_more_<test_grammar::P>;
  std::shared_ptr<rule_t> ast;
  ASSERT_TRUE(xtd::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  ASSERT_EQ(ast->size(), 3u);
  for (auto& child : *ast) EXPECT_EQ(child->get_text(), "P");

  std::string empty;
  std::shared_ptr<rule_t> ast_empty;
  ASSERT_TRUE(xtd::parser<rule_t>::parse(empty.cbegin(), empty.cend(), ast_empty));
  EXPECT_EQ(ast_empty->size(), 0u);
}

TEST(test_parser, ast_one_or_more_nested) {
  std::string s = "PDQ";
  using namespace test_grammar;
  using rule_t = xtd::parse::one_or_more_<xtd::parse::or_<P, D, Q>>;
  std::shared_ptr<rule_t> ast;
  ASSERT_TRUE(xtd::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_TRUE(ast);
  ASSERT_EQ(ast->size(), 3u);
  // each repetition child is an or_ node with one grandchild terminal
  const char* expected[] = { "P", "D", "Q" };
  size_t i = 0;
  for (auto& branch : *ast) {
    ASSERT_EQ(branch->size(), 1u);
    EXPECT_EQ((*branch)[0]->get_text(), expected[i]);
    ++i;
  }
}

TEST(test_parser, ast_named_rule_structure) {
  std::string s = "dim a\n";
  std::shared_ptr<basic_grammar::dim_statement> ast;
  ASSERT_TRUE((xtd::parser<basic_grammar::dim_statement, true, basic_grammar::WS>::parse(s.cbegin(), s.cend(), ast)));
  ASSERT_TRUE(ast);
  EXPECT_TRUE(ast->isa(typeid(basic_grammar::dim_statement)));
  // dim_statement -> and_<DIM, IDENTIFIER, EOL>
  ASSERT_EQ(ast->size(), 1u);
  auto& seq = (*ast)[0];
  ASSERT_EQ(seq->size(), 3u);
  EXPECT_EQ((*seq)[0]->get_text(), "dim");
  EXPECT_EQ((*seq)[1]->get_text(), "a");
}

TEST(test_parser, ast_parent_link) {
  std::string s = "PD";
  using rule_t = xtd::parse::and_<test_grammar::P, test_grammar::D>;
  std::shared_ptr<rule_t> ast;
  ASSERT_TRUE(xtd::parser<rule_t>::parse(s.cbegin(), s.cend(), ast));
  ASSERT_EQ(ast->size(), 2u);
  auto child = (*ast)[0];
  auto parent = child->parent();
  ASSERT_TRUE(parent);
  EXPECT_EQ(parent.get(), static_cast<xtd::parse::rule_base*>(ast.get()));
}

// Case-insensitive parsing is not implemented; kept disabled.
TEST(test_parser, DISABLED_character_no_case){
  std::string s = "p";
  using test_parse = xtd::parser<test_grammar::P, true>;
  std::shared_ptr<test_grammar::P> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
}

TEST(test_parser, DISABLED_regex_case){
  using test_parse = xtd::parser<test_grammar::Alphabet, false>;
  std::string s = "ABC1";
  std::shared_ptr<test_grammar::Alphabet> ast;
  EXPECT_TRUE(test_parse::parse(s.cbegin(), s.cend(), ast));
}
