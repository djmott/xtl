#include <xtd/windows/image_help.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

TEST_CASE("can inspect running binary", "[imagehlp]") {
    auto & oBinary = xtd::windows::image_help::binary::this_binary();
    auto & oSect = oBinary.section_header();
    auto & oNTHeader = oBinary.nt_header();
    auto & oOptional = oNTHeader.optional_header();
    REQUIRE(oBinary.e_magic > 0);
}
