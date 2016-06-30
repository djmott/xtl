/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/parse.hpp>


namespace test_grammar{
  CHARACTER_(P, 'P');
  CHARACTER_(D, 'D');
  CHARACTER_(Q, 'Q');
  STRING_(Snafoo);
  STRING_(ABC);
  STRING_(XYZ);
  REGEX(Fnord, "Fnord\\d\\dXX");
}


TEST(test_parser, character_no_case){
  std::string s = "p";
  using test_parse = xtd::parser<test_grammar::P, true>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "x";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}

TEST(test_parser, character_case){
  std::string s = "P";
  using test_parse = xtd::parser<test_grammar::P>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "p";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}

TEST(test_parser, string_no_case){
  std::string s = "Snafoo";
  using test_parse = xtd::parser<test_grammar::Snafoo, true>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "Squeegy";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}

TEST(test_parser, string_case){
  std::string s = "Snafoo";
  using test_parse = xtd::parser<test_grammar::Snafoo>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "snafoo";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}

TEST(test_parser, regex_no_case){
  std::string s = "fnord13xx";
  using test_parse = xtd::parser<test_grammar::Fnord, true>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "snafoo";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}


TEST(test_parser, regex_case){
  std::string s = "Fnord99XX";
  using test_parse = xtd::parser<test_grammar::Fnord>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "fnord99xx";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}

TEST(test_parser, rule_and){
  std::string s = "PDQ";
  using namespace test_grammar;
  using test_parse = xtd::parser<xtd::parse::and_<P,D,Q>>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "QDP";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}


TEST(test_parser, rule_or){
  std::string s = "PDQ";
  using namespace test_grammar;
  using test_parse = xtd::parser<xtd::parse::or_<P, D, Q>>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "XYZ";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}

TEST(test_parser, isa){
  std::string s = "PDQ";
  using namespace test_grammar;
  using pdq = xtd::parse::or_<P, D, Q>;
  using test_parse = xtd::parser<pdq>;
  auto oAST = test_parse::parse(s.begin(), s.end());
  EXPECT_TRUE(!!oAST);
  EXPECT_TRUE(oAST->isa(typeid(pdq)));
  ASSERT_EQ(oAST->type(), typeid(pdq));
}
