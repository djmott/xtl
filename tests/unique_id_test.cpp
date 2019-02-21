/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#include <xtd/unique_id.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

SCENARIO("unique_id used in code", "[unique_id]") {
  GIVEN("supported use") {
    THEN("is 128 bits wide") {
      REQUIRE(16 == sizeof(xtd::unique_id));
    }
    THEN("is default constructable") {
      REQUIRE(std::is_default_constructible<xtd::unique_id>::value);
    }
    
    WHEN("default constructed") {
      xtd::unique_id id1;
      THEN("is not null") {
        auto pTemp = reinterpret_cast<uint64_t*>(&id1);
        REQUIRE((0 != pTemp[0] || 0 != pTemp[1]));
      }
    }
  }
}
