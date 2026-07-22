/** @file
 * RFC 7231 HTTP/1.1 Semantics and Content grammar unit tests
 * (accept/reject permutations + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC7231.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc7231_test {

  namespace rfc = xtd::Grammars::RFC7231;

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

TEST(RFC7231, HTTP_date) {
  using namespace rfc7231_test;
  ExpectPass<rfc::IMF_fixdate>("Sun, 06 Nov 1994 08:49:37 GMT");
  ExpectPass<rfc::HTTP_date>("Sun, 06 Nov 1994 08:49:37 GMT");
  ExpectPass<rfc::rfc850_date>("Sunday, 06-Nov-94 08:49:37 GMT");
  ExpectPass<rfc::asctime_date>("Sun Nov  6 08:49:37 1994");
  ExpectPass<rfc::HTTP_date>("Sunday, 06-Nov-94 08:49:37 GMT");
  ExpectPass<rfc::HTTP_date>("Sun Nov  6 08:49:37 1994");
  ExpectPass<rfc::Date>("Sun, 06 Nov 1994 08:49:37 GMT");

  ExpectFail<rfc::IMF_fixdate>("Sun, 6 Nov 1994 08:49:37 GMT");
  ExpectFail<rfc::IMF_fixdate>("Sun, 06 Nov 1994 08:49:37");
  ExpectFail<rfc::HTTP_date>("");
}

TEST(RFC7231, QvalueAndWeight) {
  using namespace rfc7231_test;
  ExpectPass<rfc::qvalue>("0");
  ExpectPass<rfc::qvalue>("0.5");
  ExpectPass<rfc::qvalue>("0.123");
  ExpectPass<rfc::qvalue>("1");
  ExpectPass<rfc::qvalue>("1.0");
  ExpectPass<rfc::qvalue>("1.000");
  ExpectFail<rfc::qvalue>("1.001");
  ExpectFail<rfc::qvalue>("2");
  ExpectFail<rfc::qvalue>("0.1234");
  ExpectFail<rfc::qvalue>("");

  ExpectPass<rfc::weight>(";q=0.5");
  ExpectPass<rfc::weight>(" ; q=1");
  ExpectPass<rfc::weight>(";q=0");
  ExpectFail<rfc::weight>(";q=2");
}

TEST(RFC7231, MediaTypeAndRange) {
  using namespace rfc7231_test;
  ExpectPass<rfc::media_type>("text/html");
  ExpectPass<rfc::media_type>("text/plain; charset=utf-8");
  ExpectPass<rfc::media_type>("application/json; charset=\"utf-8\"");
  ExpectFail<rfc::media_type>("text");
  ExpectFail<rfc::media_type>("/html");

  ExpectPass<rfc::media_range>("*/*");
  ExpectPass<rfc::media_range>("image/*");
  ExpectPass<rfc::media_range>("text/html");
  ExpectPass<rfc::media_range>("text/html; level=1");

  ExpectPass<rfc::Content_Type>("text/html; charset=UTF-8");
}

TEST(RFC7231, Accept) {
  using namespace rfc7231_test;
  ExpectPass<rfc::accept>("");
  ExpectPass<rfc::accept>("text/html");
  ExpectPass<rfc::accept>("text/html, application/xhtml+xml, application/xml;q=0.9, */*;q=0.8");
  ExpectPass<rfc::accept>(",");
  ExpectPass<rfc::accept>("text/html;q=0.8; foo=bar");
}

TEST(RFC7231, Accept_Charset_Encoding_Language) {
  using namespace rfc7231_test;
  ExpectPass<rfc::Accept_Charset>("utf-8");
  ExpectPass<rfc::Accept_Charset>("utf-8, *;q=0.5");
  ExpectPass<rfc::Accept_Charset>(", utf-8");
  ExpectFail<rfc::Accept_Charset>("");

  ExpectPass<rfc::Accept_Encoding>("");
  ExpectPass<rfc::Accept_Encoding>("gzip");
  ExpectPass<rfc::Accept_Encoding>("gzip;q=1.0, identity; q=0.5, *;q=0");
  ExpectPass<rfc::Accept_Encoding>("identity");
  ExpectPass<rfc::Accept_Encoding>("*");

  ExpectPass<rfc::Accept_Language>("en");
  ExpectPass<rfc::Accept_Language>("en, mi");
  ExpectPass<rfc::Accept_Language>("da, en-gb;q=0.8, en;q=0.7");
  ExpectPass<rfc::Accept_Language>("*");
  ExpectFail<rfc::Accept_Language>("");
}

TEST(RFC7231, Content_Headers) {
  using namespace rfc7231_test;
  ExpectPass<rfc::Content_Encoding>("gzip");
  ExpectPass<rfc::Content_Encoding>("gzip, deflate");
  ExpectFail<rfc::Content_Encoding>("");

  ExpectPass<rfc::Content_Language>("da");
  ExpectPass<rfc::Content_Language>("mi, en");
  ExpectPass<rfc::Content_Language>("zh-Hant-CN");
  ExpectFail<rfc::Content_Language>("");

  ExpectPass<rfc::Content_Location>("http://example.com/a");
  ExpectPass<rfc::Content_Location>("/relative/path");
  ExpectPass<rfc::Referer>("http://www.example.org/index.html");
  ExpectPass<rfc::Referer>("/path?q=1");
  ExpectPass<rfc::Location>("http://example.com/new");
  ExpectPass<rfc::Location>("/new");
}

TEST(RFC7231, Allow_Expect_From_MaxForwards) {
  using namespace rfc7231_test;
  ExpectPass<rfc::Allow>("");
  ExpectPass<rfc::Allow>("GET");
  ExpectPass<rfc::Allow>("GET, HEAD, OPTIONS");
  ExpectPass<rfc::Allow>(",");

  ExpectPass<rfc::Expect>("100-continue");
  ExpectFail<rfc::Expect>("100-Continue");
  ExpectFail<rfc::Expect>("");

  ExpectPass<rfc::From>("user@example.com");
  ExpectPass<rfc::From>("Boss <boss@example.com>");

  ExpectPass<rfc::Max_Forwards>("0");
  ExpectPass<rfc::Max_Forwards>("10");
  ExpectFail<rfc::Max_Forwards>("");
  ExpectFail<rfc::Max_Forwards>("a");
}

TEST(RFC7231, ProductHeaders) {
  using namespace rfc7231_test;
  ExpectPass<rfc::product>("curl");
  ExpectPass<rfc::product>("curl/7.16.3");
  ExpectPass<rfc::User_Agent>("curl/7.16.3");
  ExpectPass<rfc::User_Agent>("curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3");
  ExpectPass<rfc::User_Agent>("CERN-LineMode/2.15 libwww/2.17b3");
  ExpectPass<rfc::Server>("Apache");
  ExpectPass<rfc::Server>("CERN/3.0 libwww/2.17");
  ExpectFail<rfc::User_Agent>("");
  ExpectFail<rfc::Server>("");
}

TEST(RFC7231, Vary_RetryAfter) {
  using namespace rfc7231_test;
  ExpectPass<rfc::Vary>("*");
  ExpectPass<rfc::Vary>("Accept-Encoding");
  ExpectPass<rfc::Vary>("Accept-Encoding, Accept-Language");
  ExpectFail<rfc::Vary>("");

  ExpectPass<rfc::Retry_After>("120");
  ExpectPass<rfc::Retry_After>("Fri, 31 Dec 1999 23:59:59 GMT");
  ExpectFail<rfc::Retry_After>("");
}

TEST(RFC7231, AST_Shape) {
  using namespace rfc7231_test;
  {
    std::shared_ptr<rfc::HTTP_date> ast;
    ASSERT_TRUE(parse_all(std::string("Sun, 06 Nov 1994 08:49:37 GMT"), ast));
    ExpectHas<rfc::IMF_fixdate>(*ast);
  }
  {
    std::shared_ptr<rfc::accept> ast;
    ASSERT_TRUE(parse_all(std::string("text/html;q=0.8"), ast));
    ExpectHas<rfc::media_range>(*ast);
    // PEG: media_range *(OWS ";" OWS parameter) absorbs ";q=…" as parameter
    ExpectHas<rfc::parameter>(*ast);
  }
  {
    std::shared_ptr<rfc::Accept_Language> ast;
    ASSERT_TRUE(parse_all(std::string("en;q=0.8"), ast));
    ExpectHas<rfc::weight>(*ast);
  }
  {
    std::shared_ptr<rfc::Content_Type> ast;
    ASSERT_TRUE(parse_all(std::string("text/plain; charset=utf-8"), ast));
    ExpectHas<rfc::media_type>(*ast);
    ExpectHas<rfc::parameter>(*ast);
  }
  {
    std::shared_ptr<rfc::User_Agent> ast;
    ASSERT_TRUE(parse_all(std::string("curl/7.16.3"), ast));
    ExpectHas<rfc::product>(*ast);
  }
}
