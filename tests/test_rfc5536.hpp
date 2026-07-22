/** @file
 * RFC 5536 Netnews article grammar unit tests (accept/reject + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC5536.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc5536_test {

  namespace rfc = xtd::Grammars::RFC5536;

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

  const char* kMinimalArticle =
    "Date: Mon, 21 Jul 2026 16:00:00 +0000\r\n"
    "From: poster@example.com\r\n"
    "Message-ID: <abc.123@example.com>\r\n"
    "Newsgroups: misc.test\r\n"
    "Path: news.example.com!not-for-mail\r\n"
    "Subject: Hello world\r\n"
    "\r\n"
    "Body text.\r\n";

}

TEST(RFC5536, Unstructured) {
  using namespace rfc5536_test;
  ExpectPass<rfc::unstructured>("hello");
  ExpectPass<rfc::unstructured>("  hello  ");
  ExpectFail<rfc::unstructured>("");
  ExpectFail<rfc::unstructured>("   ");
}

TEST(RFC5536, MsgIdRestricted) {
  using namespace rfc5536_test;
  ExpectPass<rfc::msg_id>("<abc.123@example.com>");
  ExpectPass<rfc::msg_id>("<abc@[literal]>");
  // No CFWS inside 5536 msg-id
  ExpectFail<rfc::msg_id>("<abc@example.com >");
  ExpectFail<rfc::msg_id>("<(comment)abc@example.com>");
  ExpectFail<rfc::msg_id>("abc@example.com");
}

TEST(RFC5536, NewsgroupName) {
  using namespace rfc5536_test;
  for (const char* s : {"misc.test", "comp.lang.c++", "a", "foo+bar", "x_y-z"}) {
    ExpectPass<rfc::newsgroup_name>(s);
  }
  ExpectFail<rfc::newsgroup_name>("");
  ExpectFail<rfc::newsgroup_name>("bad name");
  ExpectFail<rfc::newsgroup_name>(".leading");
}

TEST(RFC5536, Path) {
  using namespace rfc5536_test;
  ExpectPass<rfc::path>("Path: news.example.com!not-for-mail\r\n");
  ExpectPass<rfc::path>("Path: site1!site2!not-for-mail\r\n");
  ExpectPass<rfc::path>("Path:  site!!other!not-for-mail\r\n");
  ExpectFail<rfc::path>("Path:not-for-mail\r\n"); // missing SP after colon
  ExpectFail<rfc::path>("Path: \r\n");
}

TEST(RFC5536, MandatoryHeaders) {
  using namespace rfc5536_test;
  ExpectPass<rfc::orig_date>("Date: Mon, 21 Jul 2026 16:00:00 +0000\r\n");
  ExpectPass<rfc::orig_date>("Date: 21 Jul 2026 16:00:00 GMT\r\n");
  ExpectPass<rfc::from>("From: poster@example.com\r\n");
  ExpectPass<rfc::message_id>("Message-ID: <abc@example.com>\r\n");
  ExpectPass<rfc::newsgroups>("Newsgroups: misc.test,comp.lang.c\r\n");
  ExpectPass<rfc::subject>("Subject: Hello\r\n");

  ExpectFail<rfc::from>("From:poster@example.com\r\n"); // missing SP
  ExpectFail<rfc::subject>("Subject: \r\n"); // empty body (no VCHAR)
}

TEST(RFC5536, OptionalHeaders) {
  using namespace rfc5536_test;
  ExpectPass<rfc::followup_to>("Followup-To: misc.test\r\n");
  ExpectPass<rfc::followup_to>("Followup-To: poster\r\n");
  ExpectPass<rfc::distribution>("Distribution: world\r\n");
  ExpectPass<rfc::organization>("Organization: Example Org\r\n");
  ExpectPass<rfc::references>("References: <a@b.com> <c@d.com>\r\n");
  ExpectPass<rfc::supersedes>("Supersedes: <old@example.com>\r\n");
  ExpectPass<rfc::control>("Control: cancel <x@y.com>\r\n");
  ExpectPass<rfc::lines>("Lines: 42\r\n");
}

TEST(RFC5536, ArticlePass) {
  using namespace rfc5536_test;
  ExpectPass<rfc::article>(kMinimalArticle);

  std::string with_optional = std::string(
    "Date: Mon, 21 Jul 2026 16:00:00 +0000\r\n"
    "From: poster@example.com\r\n"
    "Message-ID: <abc.123@example.com>\r\n"
    "Newsgroups: misc.test\r\n"
    "Path: news.example.com!not-for-mail\r\n"
    "Subject: Hello world\r\n"
    "Organization: Test\r\n"
    "\r\n"
    "Body.\r\n");
  ExpectPass<rfc::article>(with_optional);
}

TEST(RFC5536, ArticleFail) {
  using namespace rfc5536_test;
  ExpectFail<rfc::article>("not an article");
  ExpectFail<rfc::article>(":\r\n\r\n");
  // Illegal: Message-ID without SP after colon
  ExpectFail<rfc::message_id>("Message-ID:<abc@example.com>\r\n");
  // 5322-style msg-id with CFWS rejected by 5536 msg_id
  ExpectFail<rfc::msg_id>("< abc@example.com >");
  ExpectFail<rfc::path>("Path:\r\n");
}

TEST(RFC5536, AST_Shape) {
  using namespace rfc5536_test;
  std::shared_ptr<rfc::article> ast;
  ASSERT_TRUE(parse_all(std::string(kMinimalArticle), ast));
  ASSERT_NE(ast, nullptr);
  ExpectHas<rfc::orig_date>(*ast);
  ExpectHas<rfc::from>(*ast);
  ExpectHas<rfc::message_id>(*ast);
  ExpectHas<rfc::newsgroups>(*ast);
  ExpectHas<rfc::path>(*ast);
  ExpectHas<rfc::subject>(*ast);
  ExpectHas<rfc::msg_id>(*ast);
}
