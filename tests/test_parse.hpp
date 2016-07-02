/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */



namespace test_grammar{
  CHARACTER_(P, 'P');
  CHARACTER_(D, 'D');
  CHARACTER_(Q, 'Q');
  STRING_(Snafoo);
  STRING_(ABC);
  STRING_(XYZ);
  REGEX(Version, "ABC\\-[[:digit:]]\\.[[:digit:]]");
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
  //the regex tests fail under certain gcc versions with buggy regex implementations
  std::string s = "abc-2.2";
  using test_parse = xtd::parser<test_grammar::Version, true>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "snafoo";
  EXPECT_FALSE(test_parse::parse(s.begin(), s.end()));
}


TEST(test_parser, regex_case){
  std::string s = "ABC-1.1";
  using test_parse = xtd::parser<test_grammar::Version>;
  EXPECT_TRUE(!!test_parse::parse(s.begin(), s.end()));
  s = "abc-1.1";
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
