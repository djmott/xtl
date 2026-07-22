/** @file
 * RFC 7230 HTTP/1.1 Message Syntax and Routing grammar unit tests
 * (accept/reject permutations + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC7230.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rfc7230_test {

  namespace rfc = xtd::Grammars::RFC7230;

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

  const char* kMinimalRequest =
    "GET /hello.txt HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "\r\n";

  const char* kCurlRequest =
    "GET /hello.txt HTTP/1.1\r\n"
    "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
    "Host: www.example.com\r\n"
    "Accept-Language: en, mi\r\n"
    "\r\n";

  const char* kMinimalResponse =
    "HTTP/1.1 200 OK\r\n"
    "\r\n";

}

TEST(RFC7230, TokenAndTchar) {
  using namespace rfc7230_test;
  for (const char* s : {"GET", "Host", "a", "a1", "!#$%&'*+-.^_`|~", "CONTENT_TYPE"}) {
    ExpectPass<rfc::token>(s);
  }
  ExpectFail<rfc::token>("");
  ExpectFail<rfc::token>("has space");
  ExpectFail<rfc::token>("a:b");
  ExpectFail<rfc::token>("\"quoted\"");
}

TEST(RFC7230, QuotedStringAndComment) {
  using namespace rfc7230_test;
  ExpectPass<rfc::quoted_string>("\"\"");
  ExpectPass<rfc::quoted_string>("\"hello\"");
  ExpectPass<rfc::quoted_string>("\"a\\\"b\"");
  ExpectFail<rfc::quoted_string>("\"unterminated");
  ExpectFail<rfc::quoted_string>("hello");

  ExpectPass<rfc::comment>("()");
  ExpectPass<rfc::comment>("(hello)");
  ExpectPass<rfc::comment>("(a (nested) b)");
  ExpectPass<rfc::comment>("(a\\)b)");
  ExpectFail<rfc::comment>("(");
  ExpectFail<rfc::comment>("(unterminated");
}

TEST(RFC7230, OWS_RWS) {
  using namespace rfc7230_test;
  ExpectPass<rfc::OWS>("");
  ExpectPass<rfc::OWS>(" ");
  ExpectPass<rfc::OWS>("\t");
  ExpectPass<rfc::OWS>(" \t ");
  ExpectFail<rfc::OWS>("x");
  ExpectFail<rfc::OWS>("\r\n");

  ExpectPass<rfc::RWS>(" ");
  ExpectPass<rfc::RWS>("\t");
  ExpectPass<rfc::RWS>("  \t");
  ExpectFail<rfc::RWS>("");
  ExpectFail<rfc::RWS>("x");
}

TEST(RFC7230, HTTP_version) {
  using namespace rfc7230_test;
  ExpectPass<rfc::HTTP_version>("HTTP/1.1");
  ExpectPass<rfc::HTTP_version>("HTTP/1.0");
  ExpectPass<rfc::HTTP_version>("HTTP/2.0");
  ExpectFail<rfc::HTTP_version>("HTTP/1");
  ExpectFail<rfc::HTTP_version>("http/1.1");
  ExpectFail<rfc::HTTP_version>("HTTP/1.1 ");
  ExpectFail<rfc::HTTP_version>("");
}

TEST(RFC7230, RequestTarget) {
  using namespace rfc7230_test;
  ExpectPass<rfc::origin_form>("/");
  ExpectPass<rfc::origin_form>("/hello.txt");
  ExpectPass<rfc::origin_form>("/a/b?x=1");
  ExpectPass<rfc::absolute_form>("http://www.example.com/index.html");
  ExpectPass<rfc::authority_form>("www.example.com");
  ExpectPass<rfc::authority_form>("www.example.com:8080");
  ExpectPass<rfc::asterisk_form>("*");

  ExpectPass<rfc::request_target>("/");
  ExpectPass<rfc::request_target>("/hello.txt");
  ExpectPass<rfc::request_target>("http://www.example.com/path");
  ExpectPass<rfc::request_target>("www.example.com:80");
  ExpectPass<rfc::request_target>("*");

  ExpectFail<rfc::origin_form>("");
  ExpectFail<rfc::asterisk_form>("**");
}

TEST(RFC7230, RequestLine) {
  using namespace rfc7230_test;
  ExpectPass<rfc::request_line>("GET / HTTP/1.1\r\n");
  ExpectPass<rfc::request_line>("GET /hello.txt HTTP/1.1\r\n");
  ExpectPass<rfc::request_line>("OPTIONS * HTTP/1.1\r\n");
  ExpectFail<rfc::request_line>("GET / HTTP/1.1");
  ExpectFail<rfc::request_line>("GET/ HTTP/1.1\r\n");
  ExpectFail<rfc::request_line>("GET  / HTTP/1.1\r\n");
}

TEST(RFC7230, StatusLine) {
  using namespace rfc7230_test;
  ExpectPass<rfc::status_line>("HTTP/1.1 200 OK\r\n");
  ExpectPass<rfc::status_line>("HTTP/1.1 404 Not Found\r\n");
  ExpectPass<rfc::status_line>("HTTP/1.1 200 \r\n");
  ExpectFail<rfc::status_line>("HTTP/1.1 20 OK\r\n");
  ExpectFail<rfc::status_line>("HTTP/1.1 2000 OK\r\n");
  ExpectFail<rfc::status_line>("HTTP/1.1 200 OK");
}

TEST(RFC7230, HeaderField) {
  using namespace rfc7230_test;
  ExpectPass<rfc::header_field>("Host: www.example.com");
  ExpectPass<rfc::header_field>("Host:www.example.com");
  ExpectPass<rfc::header_field>("Accept-Language: en, mi");
  ExpectPass<rfc::header_field>("X-Empty:");
  ExpectPass<rfc::header_field>("X-Fold: a\r\n b");
  ExpectFail<rfc::header_field>("Host www.example.com");
  ExpectFail<rfc::header_field>(": value");
}

TEST(RFC7230, http_URI) {
  using namespace rfc7230_test;
  ExpectPass<rfc::http_URI>("http://www.yahoo.com/foobar/here/now.txt");
  ExpectPass<rfc::http_URI>("http://example.com/");
  ExpectPass<rfc::http_URI>("http://example.com");
  ExpectPass<rfc::https_URI>("https://example.com/a?b=1#f");
  ExpectFail<rfc::http_URI>("https://example.com/");
  ExpectFail<rfc::http_URI>("http:/example.com");
}

TEST(RFC7230, TransferCoding) {
  using namespace rfc7230_test;
  ExpectPass<rfc::transfer_coding>("chunked");
  ExpectPass<rfc::transfer_coding>("compress");
  ExpectPass<rfc::transfer_coding>("deflate");
  ExpectPass<rfc::transfer_coding>("gzip");
  ExpectPass<rfc::transfer_coding>("custom");
  ExpectPass<rfc::transfer_coding>("custom;foo=bar");
  ExpectPass<rfc::transfer_coding>("custom ; foo = \"bar\"");

  ExpectPass<rfc::Transfer_Encoding>("chunked");
  ExpectPass<rfc::Transfer_Encoding>("gzip, chunked");
  ExpectPass<rfc::Transfer_Encoding>(", gzip");

  ExpectFail<rfc::transfer_coding>("");
  ExpectFail<rfc::transfer_coding>("chunked gzip");
}

TEST(RFC7230, ChunkedBody) {
  using namespace rfc7230_test;
  // last-chunk only — non-empty chunk + trailing CRLF needs length-aware parsing
  // (greedy chunk_data / OCTET cannot leave the terminating CRLF).
  ExpectPass<rfc::last_chunk>("0\r\n");
  ExpectPass<rfc::last_chunk>("000\r\n");
  ExpectPass<rfc::chunk_size>("a");
  ExpectPass<rfc::chunk_size>("1A3");
  ExpectPass<rfc::chunked_body>("0\r\n\r\n");
  ExpectPass<rfc::chunked_body>("0\r\nX-Trailer: v\r\n\r\n");
  ExpectFail<rfc::last_chunk>("0");
  ExpectFail<rfc::chunked_body>("0\r\n");
}

TEST(RFC7230, HTTP_message_Pass) {
  using namespace rfc7230_test;
  ExpectPass<rfc::HTTP_message>(kMinimalRequest);
  ExpectPass<rfc::HTTP_message>(kCurlRequest);
  ExpectPass<rfc::HTTP_message>(kMinimalResponse);
  ExpectPass<rfc::HTTP_message>("GET / HTTP/1.1\r\n\r\n");
  ExpectPass<rfc::HTTP_message>(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "hello"
  );
}

TEST(RFC7230, HTTP_message_Fail) {
  using namespace rfc7230_test;
  ExpectFail<rfc::HTTP_message>("not http");
  ExpectFail<rfc::HTTP_message>("GET / HTTP/1.1\r\n");
  ExpectFail<rfc::HTTP_message>("GET / HTTP/1.1");
  ExpectFail<rfc::HTTP_message>("");
}

TEST(RFC7230, AST_Shape) {
  using namespace rfc7230_test;
  std::shared_ptr<rfc::HTTP_message> ast;
  ASSERT_TRUE(parse_all(std::string(kMinimalRequest), ast));
  ASSERT_NE(ast, nullptr);
  ExpectHas<rfc::request_line>(*ast);
  ExpectHas<rfc::method>(*ast);
  ExpectHas<rfc::request_target>(*ast);
  ExpectHas<rfc::HTTP_version>(*ast);
  ExpectHas<rfc::header_field>(*ast);
  ExpectHas<rfc::field_name>(*ast);

  std::shared_ptr<rfc::HTTP_message> resp;
  ASSERT_TRUE(parse_all(std::string(kMinimalResponse), resp));
  ExpectHas<rfc::status_line>(*resp);
  ExpectHas<rfc::status_code>(*resp);
}
