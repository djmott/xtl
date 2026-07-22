/** @file
 * RFC 7233 HTTP/1.1 Range Requests grammar unit tests
 * (accept/reject permutations + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC7233.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc7233_test {

  namespace rfc = xtd::Grammars::RFC7233;

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

TEST(RFC7233, Range_Smoke) {
  using namespace rfc7233_test;
  ExpectPass<rfc::Range>("bytes=0-499");
}

TEST(RFC7233, ByteRangeSet) {
  using namespace rfc7233_test;
  ExpectPass<rfc::byte_range_set>("0-499");
  ExpectPass<rfc::byte_range_set>("500-");
  ExpectPass<rfc::byte_range_set>("-500");
  ExpectPass<rfc::byte_range_set>("0-0");
  ExpectPass<rfc::byte_range_set>("0-499,500-999");
  ExpectPass<rfc::byte_range_set>(",0-1");
  ExpectPass<rfc::byte_range_set>("0-1,");
  ExpectFail<rfc::byte_range_set>("");
  ExpectFail<rfc::byte_range_set>("-");
  ExpectFail<rfc::byte_range_set>("0");
  ExpectFail<rfc::byte_range_set>("abc");
}

TEST(RFC7233, Range) {
  using namespace rfc7233_test;
  ExpectPass<rfc::Range>("bytes=0-499");
  ExpectPass<rfc::Range>("bytes=-500");
  ExpectPass<rfc::Range>("bytes=500-");
  ExpectPass<rfc::Range>("bytes=0-0,-1");
  ExpectPass<rfc::Range>("items=1-2");
  ExpectFail<rfc::Range>("bytes=");
  ExpectFail<rfc::Range>("=0-1");
  ExpectFail<rfc::Range>("BYTES=0-1");
}

TEST(RFC7233, Accept_Ranges) {
  using namespace rfc7233_test;
  ExpectPass<rfc::Accept_Ranges>("bytes");
  ExpectPass<rfc::Accept_Ranges>("none");
  ExpectPass<rfc::Accept_Ranges>("bytes, pages");
  ExpectFail<rfc::Accept_Ranges>("");
  ExpectFail<rfc::Accept_Ranges>(",");
}

TEST(RFC7233, Content_Range) {
  using namespace rfc7233_test;
  ExpectPass<rfc::Content_Range>("bytes 0-499/1234");
  ExpectPass<rfc::Content_Range>("bytes 42-42/*");
  ExpectPass<rfc::Content_Range>("bytes */1234");
  ExpectPass<rfc::Content_Range>("items 1-2/3");
  ExpectFail<rfc::Content_Range>("bytes0-1/2");
  ExpectFail<rfc::Content_Range>("bytes */");
  ExpectFail<rfc::Content_Range>("bytes 0-/1");
}

TEST(RFC7233, If_Range) {
  using namespace rfc7233_test;
  ExpectPass<rfc::If_Range>("W/\"xyzzy\"");
  ExpectPass<rfc::If_Range>("\"abc\"");
  ExpectPass<rfc::If_Range>("Sat, 29 Oct 1994 19:43:31 GMT");
  ExpectFail<rfc::If_Range>("");
  ExpectFail<rfc::If_Range>("not-a-tag-or-date");
}

TEST(RFC7233, AST_Shape) {
  using namespace rfc7233_test;
  {
    std::shared_ptr<rfc::Range> ast;
    ASSERT_TRUE(parse_all(std::string("bytes=0-499"), ast));
    ExpectHas<rfc::bytes_unit>(*ast);
    ExpectHas<rfc::byte_range_spec>(*ast);
  }
  {
    std::shared_ptr<rfc::Accept_Ranges> ast;
    ASSERT_TRUE(parse_all(std::string("none"), ast));
    ExpectHas<rfc::none>(*ast);
  }
  {
    std::shared_ptr<rfc::If_Range> ast;
    ASSERT_TRUE(parse_all(std::string("W/\"xyzzy\""), ast));
    ExpectHas<rfc::entity_tag>(*ast);
  }
  {
    std::shared_ptr<rfc::If_Range> ast;
    ASSERT_TRUE(parse_all(std::string("Sat, 29 Oct 1994 19:43:31 GMT"), ast));
    ExpectHas<rfc::HTTP_date>(*ast);
  }
}
