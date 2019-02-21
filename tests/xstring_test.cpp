/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#include <xtd/xstring.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

TEST_CASE("trims leading white space", "[xstring]") {
  REQUIRE("X  " == xtd::cstring("\r\n\t X  ").ltrim());
  REQUIRE(L"X  " == xtd::ustring(L"\r\n\t X  ").ltrim());
}


TEST_CASE("trims trailing white space", "[xstring]") {
  REQUIRE("  X" == xtd::cstring("  X\r\n\t ").rtrim());
  REQUIRE(L"  X" == xtd::ustring(L"  X\r\n\t ").rtrim());
}

TEST_CASE("trims leading and trailing white space", "[xstring]") {
  REQUIRE("X" == xtd::cstring("\r\n\t  X  \r\n\t").trim());
  REQUIRE(L"X" == xtd::ustring(L"\r\n\t  X  \r\n\t").trim());
}

TEST_CASE("replaces list of character", "[xstring]") {
  REQUIRE("XXX" == xtd::cstring("<X>").replace({ '<', '>' }, 'X'));
  REQUIRE(L"XXX" == xtd::ustring(L"<X>").replace({ L'<', L'>' }, L'X'));
}

TEST_CASE("converts lower case to upper case", "[xstring]") {
  REQUIRE("XXX" == xtd::cstring("xxx").to_upper());
  REQUIRE(L"XXX" == xtd::ustring(L"xxx").to_upper());
}

TEST_CASE("converts upper case to lower case", "[xstring]") {
  REQUIRE("xxx" == xtd::cstring("XXX").to_lower());
  REQUIRE(L"xxx" == xtd::ustring(L"XXX").to_lower());
}