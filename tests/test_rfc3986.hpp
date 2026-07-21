/** @file
 * RFC 3986 grammar unit tests (accept/reject permutations + AST shape)
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/grammars/RFC3986.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace {

  namespace rfc = xtd::Grammars::RFC3986;

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

} // namespace

TEST(RFC3986, PercentEncoding) {
  for (const char* s : {"%20", "%ff", "%00", "%AB", "%ab", "%9F"}) {
    ExpectPass<rfc::pct_encoded>(s);
  }
  for (const char* s : {"%", "%g1", "%1", "%fff", "ff", "%zz", ""}) {
    ExpectFail<rfc::pct_encoded>(s);
  }
}

TEST(RFC3986, Unreserved) {
  for (const char* s : {"a", "Z", "0", "9", "-", ".", "_", "~"}) {
    ExpectPass<rfc::unreserved>(s);
  }
  for (const char* s : {"!", "$", ":", "/", " ", "%"}) {
    ExpectFail<rfc::unreserved>(s);
  }
}

TEST(RFC3986, GenDelims) {
  for (const char* s : {":", "/", "?", "#", "[", "]", "@"}) {
    ExpectPass<rfc::gen_delims>(s);
  }
  for (const char* s : {"!", "a", "%", ""}) {
    ExpectFail<rfc::gen_delims>(s);
  }
}

TEST(RFC3986, SubDelims) {
  for (const char* s : {"!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="}) {
    ExpectPass<rfc::sub_delims>(s);
  }
  for (const char* s : {":", "/", "?", "%", "a"}) {
    ExpectFail<rfc::sub_delims>(s);
  }
}

TEST(RFC3986, Scheme) {
  for (const char* s : {"http", "HTTP", "a", "a+b.-", "ftp", "https", "file"}) {
    ExpectPass<rfc::scheme>(s);
  }
  for (const char* s : {"", "1http", "-http", "ht tp", "http:", "+http"}) {
    ExpectFail<rfc::scheme>(s);
  }
}

TEST(RFC3986, UserinfoPortAuthority) {
  for (const char* s : {"", "user", "user:pass", "%20%ff", "-_~", "!$&'", "..."}) {
    ExpectPass<rfc::userinfo>(s);
  }
  ExpectFail<rfc::userinfo>("user name");
  ExpectFail<rfc::userinfo>("%zz");

  for (const char* s : {"", "80", "65535", "0"}) {
    ExpectPass<rfc::port>(s);
  }
  ExpectFail<rfc::port>("80a");

  for (const char* s : {"example.com", "localhost", "", "%20host", "foo-bar"}) {
    ExpectPass<rfc::reg_name>(s);
  }
  ExpectFail<rfc::reg_name>("ex ample");

  for (const char* s : {"yahoo.com", "squeegy:80", "fnord@foo:80", "user:pass@host",
                        "192.168.0.1", "192.168.0.1:8080", "[::1]", "[::1]:443"}) {
    ExpectPass<rfc::authority>(s);
  }
  ExpectFail<rfc::authority>("ex ample.com");
  ExpectFail<rfc::authority>("[::1");
}

TEST(RFC3986, DecOctetAndIPv4) {
  for (const char* s : {"0", "9", "10", "99", "100", "199", "200", "249", "250", "255"}) {
    ExpectPass<rfc::dec_octet>(s);
  }
  for (const char* s : {"256", "300", "01", "001", "00", ""}) {
    ExpectFail<rfc::dec_octet>(s);
  }

  for (const char* s : {"0.0.0.0", "255.255.255.255", "192.168.1.1", "10.0.0.1", "127.0.0.1"}) {
    ExpectPass<rfc::IPv4address>(s);
  }
  for (const char* s : {"256.0.0.1", "1.2.3", "1.2.3.4.5", "1.2.3.", ".1.2.3", "1.2.256.4", ""}) {
    ExpectFail<rfc::IPv4address>(s);
  }
}

TEST(RFC3986, IPv6AndLiteral) {
  const char* pass_ipv6[] = {
    "2001:db8:85a3:0:0:8a2e:370:7334",
    "0:0:0:0:0:FFFF:129.144.52.38",
    "1:2:3:4:5:6:7:8",
    "::ffff:0:0:0:0:0:0",
    "2001:db8::1",
    "::1",
    "::",
    "fe80::1",
    "2001:db8:85a3::8a2e:370:7334",
    "::ffff:192.0.2.1",
    "2001:db8::192.0.2.1",
  };
  for (const char* s : pass_ipv6) {
    ExpectPass<rfc::IPv6address>(s);
  }

  for (const char* s : {"::1::2", "gggg::", "1:2:3", "1:2:3:4:5:6:7:8:9", "[::1]", ""}) {
    ExpectFail<rfc::IPv6address>(s);
  }

  ExpectPass<rfc::IPvFuture>("v1.x");
  ExpectPass<rfc::IPvFuture>("vF.foo:bar");
  ExpectFail<rfc::IPvFuture>("v.x");
  ExpectFail<rfc::IPvFuture>("1.x");

  ExpectPass<rfc::IP_literal>("[::1]");
  ExpectPass<rfc::IP_literal>("[2001:db8::1]");
  ExpectPass<rfc::IP_literal>("[v1.x]");
  ExpectFail<rfc::IP_literal>("[::1");
  ExpectFail<rfc::IP_literal>("::1");
  ExpectFail<rfc::IP_literal>("[gggg::]");
}

TEST(RFC3986, Host) {
  ExpectPass<rfc::host>("example.com");
  ExpectPass<rfc::host>("192.168.1.1");
  ExpectPass<rfc::host>("[::1]");
  ExpectPass<rfc::host>("[v1.xyz]");
  ExpectFail<rfc::host>("[::1");
  ExpectFail<rfc::host>("ex ample");
}

TEST(RFC3986, Paths) {
  ExpectPass<rfc::path_abempty>("");
  ExpectPass<rfc::path_abempty>("/");
  ExpectPass<rfc::path_abempty>("/a");
  ExpectPass<rfc::path_abempty>("/a/b");
  ExpectPass<rfc::path_abempty>("/a/b/");

  ExpectPass<rfc::path_absolute>("/");
  ExpectPass<rfc::path_absolute>("/a");
  ExpectPass<rfc::path_absolute>("/a/b");
  ExpectFail<rfc::path_absolute>("");
  ExpectFail<rfc::path_absolute>("a/b");
  ExpectFail<rfc::path_absolute>("//");

  ExpectPass<rfc::path_noscheme>("a");
  ExpectPass<rfc::path_noscheme>("a/b");
  ExpectFail<rfc::path_noscheme>("");
  ExpectFail<rfc::path_noscheme>("/a");
  ExpectFail<rfc::path_noscheme>(":a");

  ExpectPass<rfc::path_rootless>("a");
  ExpectPass<rfc::path_rootless>("a:b");
  ExpectPass<rfc::path_rootless>("a/b");
  ExpectFail<rfc::path_rootless>("");
  ExpectFail<rfc::path_rootless>("/a");

  ExpectPass<rfc::path_empty>("");
  ExpectFail<rfc::path_empty>("x");
}

TEST(RFC3986, QueryFragment) {
  ExpectPass<rfc::query>("");
  ExpectPass<rfc::query>("a=b");
  ExpectPass<rfc::query>("a=b&c=d");
  ExpectPass<rfc::query>("a?/b");
  ExpectFail<rfc::query>("a b");

  ExpectPass<rfc::fragment>("");
  ExpectPass<rfc::fragment>("section");
  ExpectPass<rfc::fragment>("a?/b");
  ExpectFail<rfc::fragment>("a b");
}

TEST(RFC3986, URI_PassPermutations) {
  const char* pass[] = {
    "foo://example.com:8042/over/there?name=ferret#nose",
    "urn:example:animal:ferret:nose",
    "foo:",
    "smash:",
    "http://example.com",
    "https://example.com/",
    "https://example.com/a/b",
    "http://user@example.com/",
    "http://user:pass@example.com/",
    "http://example.com:80/",
    "http://example.com:65535/path",
    "http://example.com:/path",
    "http://192.168.0.1/",
    "http://192.168.0.1:8080/x",
    "http://[::1]/",
    "http://[::1]:443/",
    "http://[2001:db8::1]/path",
    "http://[v1.x]/",
    "ftp://ftp.example.com/file",
    "file:///etc/passwd",
    "http://example.com?",
    "http://example.com?a=b",
    "http://example.com?a=b&c=d",
    "http://example.com?a?/b",
    "http://example.com#",
    "http://example.com#section",
    "http://example.com#a?/b",
    "http://example.com/path?x=1#y",
    "mailto:user@example.com",
    "urn:isbn:0451450523",
    "http:",
    "http:/path",
    "http:path",
  };
  for (const char* s : pass) {
    ExpectPass<rfc::URI>(s);
  }
}

TEST(RFC3986, URI_FailPermutations) {
  const char* fail[] = {
    "",
    " ",
    "http://ex ample.com",
    "//example.com/path",
    "/just/path",
    "1http://example.com",
    "-http://example.com",
    "http://exa mple.com",
    "http://[::1",
    "http://example.com/%zz",
    "http://example.com/%",
    "http://example.com/path trash",
    "http://[256.0.0.1]/",
    "http://[::1::2]/",
    "relative/path",
    "?query",
    "#frag",
  };
  for (const char* s : fail) {
    ExpectFail<rfc::URI>(s);
  }
}

TEST(RFC3986, AbsoluteURI) {
  ExpectPass<rfc::absolute_URI>("http://example.com/path?x=1");
  ExpectPass<rfc::absolute_URI>("urn:example:animal:ferret:nose");
  ExpectPass<rfc::absolute_URI>("foo:");
  ExpectFail<rfc::absolute_URI>("http://example.com/path?x=1#frag");
  ExpectFail<rfc::absolute_URI>("//example.com/path");
  ExpectFail<rfc::absolute_URI>("/path");
  ExpectFail<rfc::absolute_URI>("");
}

TEST(RFC3986, RelativeRefAndURIReference) {
  const char* rel_pass[] = {
    "//example.com/path",
    "/absolute",
    "/absolute/path",
    "relative/path",
    "relative",
    "",
    "?query",
    "?a=b",
    "#frag",
    "//host?x=1#y",
    "/path?q#f",
  };
  for (const char* s : rel_pass) {
    ExpectPass<rfc::relative_ref>(s);
    ExpectPass<rfc::URI_reference>(s);
  }

  ExpectFail<rfc::relative_ref>("http://example.com/");

  ExpectPass<rfc::URI_reference>("http://example.com/");
  ExpectPass<rfc::URI_reference>("urn:foo:bar");
  ExpectPass<rfc::URI_reference>("foo://example.com:8042/over/there?name=ferret#nose");
  ExpectFail<rfc::URI_reference>("http://ex ample.com");
  ExpectFail<rfc::URI_reference>(" ");
}

TEST(RFC3986, HierPart) {
  ExpectPass<rfc::hier_part>("//fnord/squeegy");
  ExpectPass<rfc::hier_part>("//example.com");
  ExpectPass<rfc::hier_part>("/abs");
  ExpectPass<rfc::hier_part>("rootless");
  ExpectPass<rfc::hier_part>("");
}

TEST(RFC3986, AST_Shape_RFCExample) {
  std::shared_ptr<rfc::URI> ast;
  ASSERT_TRUE(parse_all(std::string("foo://example.com:8042/over/there?name=ferret#nose"), ast));
  ExpectHas<rfc::scheme>(*ast);
  ExpectHas<rfc::hier_part>(*ast);
  ExpectHas<rfc::authority>(*ast);
  ExpectHas<rfc::host>(*ast);
  ExpectHas<rfc::reg_name>(*ast);
  ExpectHas<rfc::path_abempty>(*ast);
  ExpectHas<rfc::query>(*ast);
  ExpectHas<rfc::fragment>(*ast);
  ExpectHas<rfc::port>(*ast);
}

TEST(RFC3986, AST_Shape_HttpWithUserinfo) {
  std::shared_ptr<rfc::URI> ast;
  ASSERT_TRUE(parse_all(std::string("https://user:pass@192.168.0.1:8443/a/b?x=1#frag"), ast));
  ExpectHas<rfc::scheme>(*ast);
  ExpectHas<rfc::userinfo>(*ast);
  ExpectHas<rfc::host>(*ast);
  ExpectHas<rfc::IPv4address>(*ast);
  ExpectHas<rfc::port>(*ast);
  ExpectHas<rfc::path_abempty>(*ast);
  ExpectHas<rfc::query>(*ast);
  ExpectHas<rfc::fragment>(*ast);
}

TEST(RFC3986, AST_Shape_IPv6Literal) {
  std::shared_ptr<rfc::URI> ast;
  ASSERT_TRUE(parse_all(std::string("http://[2001:db8::1]/"), ast));
  ExpectHas<rfc::IP_literal>(*ast);
  ExpectHas<rfc::IPv6address>(*ast);
}

TEST(RFC3986, AST_Shape_Urn) {
  std::shared_ptr<rfc::URI> ast;
  ASSERT_TRUE(parse_all(std::string("urn:example:animal:ferret:nose"), ast));
  ExpectHas<rfc::scheme>(*ast);
  ExpectHas<rfc::path_rootless>(*ast);
}
