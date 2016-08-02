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

TEST_CASE("Write to file", "[io, headers]"){
    sbf::File file("/tmp/test_cpp.sbf", sbf::writing);
    sbf::sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }
    REQUIRE(file.open() == sbf::success);
    SECTION("Write headers") {
        REQUIRE(file.write_headers() == sbf::success);
    }
    //res = file.add_dataset<sbf_integer, 1000>("integer_dataset", ints);
    REQUIRE(file.close() == sbf::success);
}
