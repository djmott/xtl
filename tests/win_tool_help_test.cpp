/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#include <xtd/windows/tool_help.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
namespace toolhelp = xtd::windows::toolhelp;

TEST_CASE("can enumerate processes", "[toolhelp]") {  
  REQUIRE(toolhelp::process::running_processes().size());
}

TEST_CASE("can enumerate dlls", "[toolhelp]") {
  auto oProcs = toolhelp::process::running_processes();
  auto oThisProc = oProcs.find(GetCurrentProcessId());
  REQUIRE(oProcs.end() != oThisProc);
  REQUIRE(oThisProc->second->dlls().size());
}

TEST_CASE("can enumerate threads", "[toolhelp]") {
  auto oProcs = toolhelp::process::running_processes();
  auto oThisProc = oProcs.find(GetCurrentProcessId());
  REQUIRE(oProcs.end() != oThisProc);
  REQUIRE(oThisProc->second->threads().size());
}