#include <xtd/dynamic_library.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

SCENARIO("dynamic_library used in code") {
  GIVEN("supported use") {
    WHEN("a library is loaded by name") {
      auto oDLL = xtd::dynamic_library::make("dummy_library.dll");
      THEN("exported methods can be called") {
        auto oFN = oDLL->get<int(int, int)>("dummy_export");
        REQUIRE(12 == oFN(5, 7));
      }
    }
  }
}