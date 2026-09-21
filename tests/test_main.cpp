// This is the ONLY translation unit that defines CATCH_CONFIG_MAIN.
// All other test files must include <catch2/catch.hpp> WITHOUT that define.
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Sanity Check", "[sanity]") {
    REQUIRE(true);
}
