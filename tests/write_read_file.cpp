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
    using namespace sbf;
    File file(test_filename, sbf::writing);
    sbf_integer * ints = new sbf_integer[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }
    REQUIRE(file.open() == sbf::success);
    sbf::sbf_dimensions shape{{0}};
    shape[0] = 1000;

    sbf::Dataset dset1("integer_dataset", shape, sbf::SBF_INT);
    sbf::Dataset dset2("integer_dataset_negative", shape, sbf::SBF_INT);
    std::cout << "Created datasets" << std::endl;

    REQUIRE(file.add_dataset(dset2) == sbf::success);
    std::cout << "Added dset2" << std::endl;
    REQUIRE(file.n_datasets() == 1);
    std::cout << "check n_dataset" << std::endl;

    REQUIRE(file.add_dataset(dset1) == sbf::success);
    std::cout << "Added dset1" << std::endl;
    REQUIRE(file.n_datasets() == 2);
    std::cout << "check n_dataset" << std::endl;

    REQUIRE(file.write_headers() == sbf::success);
    std::cout << "wrote_headers" << std::endl;

    REQUIRE(file.write_data("integer_dataset", ints) == sbf::success);
    std::cout << "wrote data for dset1" << std::endl;

    for (int i = 0; i < 1000; i++) {
        ints[i] = -ints[i];
    }
    REQUIRE(file.write_data("integer_dataset_negative", ints) == sbf::success);
    std::cout << "wrote data for dset2" << std::endl;

    REQUIRE(file.close() == sbf::success);
    std::cout << "Wrote dataset" << std::endl;
}

TEST_CASE("Read from file", "[io, headers]") {
    sbf::File file(test_filename);
    REQUIRE(file.n_datasets() == 2);
    auto dset = file.get_dataset("integer_dataset_negative");
    std::cout << "Read dataset: " << dset.name() << std::endl;
    sbf::sbf_integer ints[1000];
    REQUIRE(file.read_data<sbf::sbf_integer>("integer_dataset_negative", ints));
    REQUIRE(ints != nullptr);
    for (int i = 0; i < 1000; i++) {
        REQUIRE(ints[i] == - i * i);
    }
    REQUIRE(file.close() == sbf::success);
    sbf::File file_fail("does not exist");
    REQUIRE(file_fail.status() == sbf::File::Status::FailedOpening);
}
