/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#include <xtd/process.hpp>

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

TEST_CASE("can retrieve running processes", "[process]") {
  auto oProcs = xtd::process::system_processes();
	REQUIRE(oProcs.size());
}

TEST_CASE("can retrieve current pid", "[process]") {
  auto oProc = xtd::process::this_process();
  REQUIRE(oProc.id());
}

TEST_CASE("can enumerate loaded modules", "[process]") {
  auto oProc = xtd::process::this_process();
  REQUIRE(oProc.libraries().size());
}