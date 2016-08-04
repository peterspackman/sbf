#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "sbf.hpp"

TEST_CASE("Open and close files", "[io, headers]") {
    SECTION("for writing") {
        sbf::File file("/tmp/test_cpp.sbf", sbf::writing);
        REQUIRE(file.open() == sbf::success);
        REQUIRE(file.close() == sbf::success);
    }
    SECTION("for reading") {
        sbf::File file("/tmp/test_cpp.sbf", sbf::reading);
        REQUIRE(file.open() == sbf::success);
        REQUIRE(file.close() == sbf::success);
    }
}

TEST_CASE("Write to file", "[io, headers]") {
    sbf::File file("/tmp/test_cpp.sbf", sbf::writing);
    sbf::sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }
    REQUIRE(file.open() == sbf::success);
    sbf::sbf_dimensions shape;
    shape[0] = 1000;
    sbf::Dataset dset("integer_dataset", shape, sbf::SBF_INT);
    REQUIRE(file.add_dataset(dset, ints) == sbf::success);
    REQUIRE(file.n_datasets() == 1);
    REQUIRE(file.write_headers() == sbf::success);
    REQUIRE(file.close() == sbf::success);
}

TEST_CASE("Read from file", "[io, headers]") {
    sbf::File file("/tmp/test_cpp.sbf", sbf::reading);
    sbf::sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }
    REQUIRE(file.open() == sbf::success);
    REQUIRE(file.read_headers() == sbf::success);
    REQUIRE(file.n_datasets() == 1);
    // res = file.add_dataset<sbf_integer, 1000>("integer_dataset", ints);
    REQUIRE(file.close() == sbf::success);
}
