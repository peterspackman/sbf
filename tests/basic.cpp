#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "sbf.hpp"
#include <iostream>

TEST_CASE("Dataset basics", "[dsets]") {
    sbf::Dataset dset("integer_dataset");
    REQUIRE(dset.get_flags() == 0);
}
