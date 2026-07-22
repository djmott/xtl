/** @file
 * RFC 7232 HTTP/1.1 Conditional Requests grammar unit tests
 * (accept/reject permutations + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC7232.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc7232_test {

  namespace rfc = xtd::Grammars::RFC7232;

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

TEST(RFC7232, EntityTag) {
  using namespace rfc7232_test;
  ExpectPass<rfc::opaque_tag>("\"\"");
  ExpectPass<rfc::opaque_tag>("\"xyzzy\"");
  ExpectPass<rfc::entity_tag>("\"xyzzy\"");
  ExpectPass<rfc::entity_tag>("W/\"xyzzy\"");
  ExpectPass<rfc::ETag>("\"xyzzy\"");
  ExpectPass<rfc::ETag>("W/\"xyzzy\"");
  ExpectPass<rfc::opaque_tag>("\"!#~\x80\"");
  ExpectFail<rfc::entity_tag>("w/\"xyzzy\"");
  ExpectFail<rfc::entity_tag>("xyzzy");
  ExpectFail<rfc::entity_tag>("\"xy\"zzy\"");
  ExpectFail<rfc::ETag>("");
}

TEST(RFC7232, If_Match_If_None_Match) {
  using namespace rfc7232_test;
  ExpectPass<rfc::If_Match>("*");
  ExpectPass<rfc::If_Match>("\"xyzzy\"");
  ExpectPass<rfc::If_Match>("\"xyzzy\", \"r2d2xxxx\", \"c3piozzzz\"");
  ExpectPass<rfc::If_Match>("W/\"xyzzy\", \"abc\"");
  ExpectPass<rfc::If_Match>(", \"xyzzy\"");
  ExpectPass<rfc::If_Match>("\"xyzzy\",");
  ExpectFail<rfc::If_Match>("");
  ExpectFail<rfc::If_Match>(",");
  ExpectFail<rfc::If_Match>("xyzzy");

  ExpectPass<rfc::If_None_Match>("*");
  ExpectPass<rfc::If_None_Match>("\"xyzzy\"");
  ExpectPass<rfc::If_None_Match>("W/\"xyzzy\", \"abc\"");
  ExpectFail<rfc::If_None_Match>("");
}

TEST(RFC7232, DateHeaders) {
  using namespace rfc7232_test;
  const char* imf = "Sat, 29 Oct 1994 19:43:31 GMT";
  ExpectPass<rfc::If_Modified_Since>(imf);
  ExpectPass<rfc::If_Unmodified_Since>(imf);
  ExpectPass<rfc::Last_Modified>(imf);
  ExpectPass<rfc::Last_Modified>("Sunday, 06-Nov-94 08:49:37 GMT");
  ExpectFail<rfc::Last_Modified>("");
  ExpectFail<rfc::If_Modified_Since>("not-a-date");
}

TEST(RFC7232, AST_Shape) {
  using namespace rfc7232_test;
  {
    std::shared_ptr<rfc::ETag> ast;
    ASSERT_TRUE(parse_all(std::string("W/\"xyzzy\""), ast));
    ExpectHas<rfc::weak>(*ast);
    ExpectHas<rfc::opaque_tag>(*ast);
  }
  {
    std::shared_ptr<rfc::If_None_Match> ast;
    ASSERT_TRUE(parse_all(std::string("\"a\", W/\"b\""), ast));
    ExpectHas<rfc::entity_tag>(*ast);
  }
  {
    std::shared_ptr<rfc::Last_Modified> ast;
    ASSERT_TRUE(parse_all(std::string("Sat, 29 Oct 1994 19:43:31 GMT"), ast));
    ExpectHas<rfc::HTTP_date>(*ast);
  }
}
