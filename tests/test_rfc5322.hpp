/** @file
 * RFC 5322 Internet Message Format grammar unit tests (accept/reject permutations + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC5322.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc5322_test {

  namespace rfc = xtd::Grammars::RFC5322;

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

TEST(RFC5322, QuotedPairAndFWS) {
  using namespace rfc5322_test;
  ExpectPass<rfc::quoted_pair>("\\a");
  ExpectPass<rfc::quoted_pair>("\\ ");
  ExpectPass<rfc::quoted_pair>("\\\\");

  ExpectFail<rfc::quoted_pair>("\\");
  ExpectFail<rfc::quoted_pair>("");
  ExpectFail<rfc::quoted_pair>("a");

  ExpectPass<rfc::FWS>(" ");
  ExpectPass<rfc::FWS>("\t");
  ExpectPass<rfc::FWS>("  ");
  ExpectPass<rfc::FWS>("\r\n ");
  ExpectPass<rfc::FWS>(" \r\n ");

  ExpectFail<rfc::FWS>("");
  ExpectFail<rfc::FWS>("\r\n");
}

TEST(RFC5322, AtomDotAtom) {
  using namespace rfc5322_test;
  for (const char* s : {"hello", "a", "a1b2", "a!#$%&'*+-/=?^_`{|}~"}) {
    ExpectPass<rfc::atom>(s);
  }
  ExpectPass<rfc::atom>(" hello ");
  ExpectPass<rfc::atom>("(comment)hello(comment)");

  ExpectFail<rfc::atom>("a b");
  ExpectFail<rfc::atom>("a.b");
  ExpectFail<rfc::atom>("");
  ExpectFail<rfc::atom>("\"quoted\"");

  for (const char* s : {"a.b.c", "hello.world", "a1.b2.c3", "a-b.c_d"}) {
    ExpectPass<rfc::dot_atom>(s);
  }
  ExpectFail<rfc::dot_atom>("a. b");
  ExpectFail<rfc::dot_atom>(".a");
  ExpectFail<rfc::dot_atom>("a.");
  ExpectFail<rfc::dot_atom>("");
}

TEST(RFC5322, QuotedString) {
  using namespace rfc5322_test;
  ExpectPass<rfc::quoted_string>("\"hello\"");
  ExpectPass<rfc::quoted_string>("\"a\\b\"");
  ExpectPass<rfc::quoted_string>("\"\"");
  ExpectPass<rfc::quoted_string>("\"hello world\"");
  ExpectPass<rfc::quoted_string>(" \"hello\" ");

  ExpectFail<rfc::quoted_string>("\"hello");
  ExpectFail<rfc::quoted_string>("hello\"");
  ExpectFail<rfc::quoted_string>("hello");
  ExpectFail<rfc::quoted_string>("");
}

TEST(RFC5322, DateTime) {
  using namespace rfc5322_test;
  ExpectPass<rfc::date_time>("Mon, 21 Jul 2026 16:00:00 +0000");
  ExpectPass<rfc::date_time>("Tue, 1 Jan 2000 00:00:00 -0500");
  ExpectPass<rfc::date_time>("Mon,  21  Jul  2026  16:00:00  +0000");
  ExpectPass<rfc::date_time>("Mon, 21 Jul 2026 16:00:00 +0000 (UTC)");
  // obsolete named zone (obs-zone), reached via the date_time -> time -> zone chain
  ExpectPass<rfc::date_time>("21 Jul 2026 16:00:00GMT");
  ExpectPass<rfc::date_time>("21 Jul 2026 16:00:00 GMT");
  ExpectPass<rfc::date_time>("Tue, 21 Jul 2026 16:00GMT");
  ExpectPass<rfc::date_time>("Tue, 21 Jul 2026 16:00:00 GMT");

  ExpectFail<rfc::date_time>("not a date");
  ExpectFail<rfc::date_time>("");
  ExpectFail<rfc::date_time>("Mon, 21 2026 16:00:00 +0000");
  ExpectFail<rfc::date_time>("Mon, 21 Jul 16:00:00 +0000");
}

TEST(RFC5322, MailboxAddrSpec) {
  using namespace rfc5322_test;
  ExpectPass<rfc::addr_spec>("user@example.com");
  ExpectPass<rfc::addr_spec>("first.last@example.com");
  ExpectPass<rfc::addr_spec>("\"john q\"@example.com");

  ExpectFail<rfc::addr_spec>("user@");
  ExpectFail<rfc::addr_spec>("@example.com");
  ExpectFail<rfc::addr_spec>("user");
  ExpectFail<rfc::addr_spec>("");

  ExpectPass<rfc::mailbox>("user@example.com");
  ExpectPass<rfc::mailbox>("Name <user@example.com>");
  ExpectPass<rfc::mailbox>("\"John Q. Public\" <user@example.com>");
  ExpectPass<rfc::mailbox>("<user@example.com>");

  ExpectFail<rfc::mailbox>("user@");
  ExpectFail<rfc::mailbox>("Name <user@>");
  ExpectFail<rfc::mailbox>("");
}

TEST(RFC5322, MsgId) {
  using namespace rfc5322_test;
  ExpectPass<rfc::msg_id>("<id@example.com>");
  ExpectPass<rfc::msg_id>("<a.b.c@sub.example.com>");
  ExpectPass<rfc::msg_id>(" <id@example.com> ");

  ExpectFail<rfc::msg_id>("id@example.com");
  ExpectFail<rfc::msg_id>("<id@example.com");
  ExpectFail<rfc::msg_id>("id@example.com>");
  ExpectFail<rfc::msg_id>("");
}

TEST(RFC5322, HeaderFields) {
  using namespace rfc5322_test;
  ExpectPass<rfc::from>("From: a@b.com\r\n");
  ExpectPass<rfc::from>("From: a@b.com, c@d.com\r\n");
  ExpectPass<rfc::subject>("Subject: hi\r\n");
  ExpectPass<rfc::subject>("Subject: \r\n");
  ExpectPass<rfc::orig_date>("Date: Mon, 21 Jul 2026 16:00:00 +0000\r\n");
  ExpectPass<rfc::to>("To: user@example.com\r\n");
  ExpectPass<rfc::message_id>("Message-ID: <id@example.com>\r\n");

  ExpectFail<rfc::from>("From: a@b.com");
  ExpectFail<rfc::subject>("Subject hi\r\n");
  ExpectFail<rfc::orig_date>("Date: garbage\r\n");
}

TEST(RFC5322, MessagePass) {
  using namespace rfc5322_test;
  ExpectPass<rfc::message>(
    "From: a@b.com\r\n"
    "To: c@d.com\r\n"
    "Subject: hi\r\n"
    "\r\n"
    "Hello world\r\n"
  );

  ExpectPass<rfc::message>(
    "From: a@b.com\r\n"
    "Subject: hi\r\n"
  );

  ExpectPass<rfc::message>(
    "Date: Mon, 21 Jul 2026 16:00:00 +0000\r\n"
    "From: Sender Name <sender@example.com>\r\n"
    "To: Recipient <recipient@example.com>\r\n"
    "Subject: Test Message\r\n"
    "Message-ID: <1234@example.com>\r\n"
    "\r\n"
    "This is the body.\r\n"
    "It has multiple lines.\r\n"
  );

  ExpectPass<rfc::message>("");
}

TEST(RFC5322, MessageFail) {
  using namespace rfc5322_test;
  ExpectFail<rfc::message>("From: a@b.com\r\nSubject");
  ExpectFail<rfc::message>("!!!not a valid message!!!");
  ExpectFail<rfc::message>("From a@b.com\r\n");
}

TEST(RFC5322, ObsForms) {
  using namespace rfc5322_test;
  ExpectPass<rfc::obs_subject>("Subject \t: hi\r\n");
  ExpectPass<rfc::obs_from>("From : a@b.com\r\n");
  ExpectPass<rfc::obs_phrase>("hello.world");
  ExpectPass<rfc::obs_phrase>("hello . world");

  ExpectPass<rfc::message>("Subject \t: hi\r\n");
  ExpectPass<rfc::message>("From : a@b.com\r\n");
  ExpectPass<rfc::message>(
    "From: a@b.com\r\n"
    "Subject \t: hi\r\n"
  );
}

TEST(RFC5322, AST_Shape) {
  using namespace rfc5322_test;
  std::shared_ptr<rfc::message> ast;
  ASSERT_TRUE(parse_all(std::string(
    "From: a@b.com\r\n"
    "Subject: hi\r\n"
    "\r\n"
    "Hello world\r\n"
  ), ast));
  ExpectHas<rfc::from>(*ast);
  ExpectHas<rfc::subject>(*ast);
  ExpectHas<rfc::body>(*ast);
  ExpectHas<rfc::addr_spec>(*ast);
  ExpectHas<rfc::mailbox_list>(*ast);
}
