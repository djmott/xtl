/** @file
 * RFC 4647 basic language-range grammar unit tests
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC4647.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc4647_test {

  namespace rfc = xtd::Grammars::RFC4647;

  template <typename Rule>
  bool parse_all(const std::string& s, std::shared_ptr<Rule>& ast) {
    return xtd::parser<Rule, false, xtd::parse::whitespace<>>::parse(s.cbegin(), s.cend(), ast);
  }

  template <typename Rule>
  void ExpectPass(const std::string& s) {
    std::shared_ptr<Rule> ast;
    EXPECT_TRUE(parse_all(s, ast)) << "expected pass: [" << s << "]";
    ASSERT_NE(ast, nullptr);
  }

  template <typename Rule>
  void ExpectFail(const std::string& s) {
    std::shared_ptr<Rule> ast;
    EXPECT_FALSE(parse_all(s, ast)) << "expected fail: [" << s << "]";
  }

  bool ast_has(const xtd::parse::rule_base& n, const std::type_info& t) {
    if (n.isa(t)) return true;
    for (const auto& child : n) {
      if (child && ast_has(*child, t)) return true;
    }
    return false;
  }

  template <typename Child>
  void ExpectHas(const xtd::parse::rule_base& n) {
    EXPECT_TRUE(ast_has(n, typeid(Child))) << "missing AST node: " << typeid(Child).name();
  }

}

TEST(RFC4647, LanguageRange_Pass) {
  using namespace rfc4647_test;
  ExpectPass<rfc::language_range>("*");
  ExpectPass<rfc::language_range>("en");
  ExpectPass<rfc::language_range>("en-US");
  ExpectPass<rfc::language_range>("x-pig-latin");
  ExpectPass<rfc::language_range>("abcdefgh");
  ExpectPass<rfc::language_range>("zh-Hans-CN");
}

TEST(RFC4647, LanguageRange_Fail) {
  using namespace rfc4647_test;
  ExpectFail<rfc::language_range>("");
  ExpectFail<rfc::language_range>("toolongprimary");
  ExpectFail<rfc::language_range>("-en");
  ExpectFail<rfc::language_range>("en-");
  ExpectFail<rfc::language_range>("en--US");
  ExpectFail<rfc::language_range>("**");
}

TEST(RFC4647, AST_Shape) {
  using namespace rfc4647_test;
  std::shared_ptr<rfc::language_range> ast;
  ASSERT_TRUE(parse_all(std::string("en-US"), ast));
  ASSERT_NE(ast, nullptr);
  ExpectHas<rfc::language_range>(*ast);
}
