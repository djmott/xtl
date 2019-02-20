#include <xtd/dynamic_library.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

SCENARIO("dynamic_library used in code") {
  GIVEN("supported use") {
    WHEN("a library is loaded by name") {
#if (XTD_OS_WINDOWS & XTD_OS)
      auto oDLL = xtd::dynamic_library::make("dummy_library.dll");
#elif (XTD_OS_UNIX & XTD_OS)
      auto oDLL = xtd::dynamic_library::make("./libdummy_library.so");
#endif
      THEN("exported methods can be called") {
        auto oFN = oDLL->get<int(int, int)>("dummy_export");
        REQUIRE(12 == oFN(5, 7));
      }
    }
  }
}