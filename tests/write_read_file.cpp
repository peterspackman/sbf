#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "sbf.hpp"
std::string test_filename = "/tmp/sbf_test_cpp.sbf";

TEST_CASE("Open and close files", "[io, headers]") {
    SECTION("for writing") {
        sbf::File file(test_filename, sbf::writing);
        REQUIRE(file.open() == sbf::success);
        REQUIRE(file.close() == sbf::success);
    }
    SECTION("for reading") {
        sbf::File file(test_filename, sbf::reading);
        REQUIRE(file.open() == sbf::success);
        REQUIRE(file.close() == sbf::success);
    }
}

TEST_CASE("Write to file", "[io, headers]") {
    sbf::File file(test_filename, sbf::writing);
    sbf::sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }
    REQUIRE(file.open() == sbf::success);
    sbf::sbf_dimensions shape{{0}};
    shape[0] = 1000;
    sbf::Dataset dset("integer_dataset", shape, sbf::SBF_INT, reinterpret_cast<void *>(ints));
    REQUIRE(file.add_dataset(dset) == sbf::success);
    REQUIRE(file.n_datasets() == 1);
    REQUIRE(file.write_headers() == sbf::success);
    REQUIRE(file.write_datablocks() == sbf::success);
    REQUIRE(file.close() == sbf::success);
}

TEST_CASE("Read from file", "[io, headers]") {
    sbf::File file(test_filename, sbf::reading);
    REQUIRE(file.open() == sbf::success);
    REQUIRE(file.read_headers() == sbf::success);
    REQUIRE(file.n_datasets() == 1);
    REQUIRE(file.read_datablocks() == sbf::success);
    auto dset = file.get_dataset("integer_dataset");
    std::cout << "Read dataset: " << dset.name() << std::endl;
    auto ints = dset.data_as<sbf::sbf_integer>();
    REQUIRE(ints != nullptr);
    for (int i = 0; i < 1000; i++) {
        REQUIRE(ints[i] == i * i);
    }
    // res = file.add_dataset<sbf_integer, 1000>("integer_dataset", ints);
    REQUIRE(file.close() == sbf::success);
}
