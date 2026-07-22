/** @file
 * RFC 5646 Language-Tag grammar unit tests
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC5646.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc5646_test {

  namespace rfc = xtd::Grammars::RFC5646;

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

TEST(RFC5646, LanguageTag_Pass) {
  using namespace rfc5646_test;
  ExpectPass<rfc::Language_Tag>("en");
  ExpectPass<rfc::Language_Tag>("en-US");
  ExpectPass<rfc::Language_Tag>("zh-Hant-CN");
  ExpectPass<rfc::Language_Tag>("x-private");
  ExpectPass<rfc::Language_Tag>("i-klingon");
  ExpectPass<rfc::Language_Tag>("zh-min-nan");
  ExpectPass<rfc::Language_Tag>("de-CH-x-phonebk");
  ExpectPass<rfc::Language_Tag>("sl-rozaj-biske");
}

TEST(RFC5646, LanguageTag_Fail) {
  using namespace rfc5646_test;
  ExpectFail<rfc::Language_Tag>("");
  ExpectFail<rfc::Language_Tag>("-en");
  ExpectFail<rfc::Language_Tag>("en-");
  ExpectFail<rfc::Language_Tag>("toolongprimarysubtag");
}

TEST(RFC5646, AST_Shape) {
  using namespace rfc5646_test;
  {
    std::shared_ptr<rfc::Language_Tag> ast;
    ASSERT_TRUE(parse_all(std::string("en-US"), ast));
    ExpectHas<rfc::langtag>(*ast);
  }
  {
    std::shared_ptr<rfc::Language_Tag> ast;
    ASSERT_TRUE(parse_all(std::string("x-private"), ast));
    ExpectHas<rfc::privateuse>(*ast);
  }
  {
    std::shared_ptr<rfc::Language_Tag> ast;
    ASSERT_TRUE(parse_all(std::string("i-klingon"), ast));
    ExpectHas<rfc::grandfathered>(*ast);
  }
}
