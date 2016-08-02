#define CATCH_CONFIG_MAIN
#include <iostream>
#include "sbf.hpp"
#include "catch.hpp"


TEST_CASE("Header compatibility", "[headers]") {
    sbf::DataHeader cpp_header;
    std::cout << "Size of DataHeader: " << sizeof(cpp_header) << std::endl;
    REQUIRE(sizeof(cpp_header) == 128);

}

TEST_CASE("DataHeader basics", "[headers]") {
    sbf::DataHeader header;
    REQUIRE(header.flags == 0);
}
