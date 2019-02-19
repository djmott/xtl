#include <xtd/var.hpp>
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>


SCENARIO("var objects are defined", "[var]") {
	GIVEN("a var definition") {
		WHEN("an empty object created on the stack") {
			xtd::var v1;
			THEN("the typeid of the underlying type is xtd::var::empty") {
				REQUIRE(typeid(xtd::var::empty) == v1.get_type());
			}
		}
	}
}

