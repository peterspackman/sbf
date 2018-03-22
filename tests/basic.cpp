#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "sbf.hpp"
#include <iostream>

constexpr size_t dset_header_size = sbf::Dataset::header_size;
constexpr size_t file_header_size = sbf::FileHeader::header_size;

TEST_CASE("Dataset basics", "[dsets]") {
    sbf::Dataset dset("integer_dataset");
    REQUIRE(dset.get_flags() == 0);
    REQUIRE(dset_header_size == 128);
}

TEST_CASE("FileHeader basics", "[files]") {
    REQUIRE(file_header_size == 7);
}
