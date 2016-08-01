#include <iostream>
#include "sbf.hpp"

bool test_open_close() {
    sbf::File file("/tmp/test_cpp.sbf", sbf::writing);
    if(file.open() != sbf::success) {
        std::cout << "Failed to open" << std::endl;
        return false;
    }
    if(file.close() != sbf::success) {
        return false;
        std::cout << "Failed to close file" << std::endl;
    }
    return true;
}

bool test_write() {
    sbf::File file("/tmp/test_cpp.sbf", sbf::writing);
    if(file.open() != sbf::success) {
        std::cout << "Failed to open" << std::endl;
        return false;
    }

    if(file.write_headers() != sbf::success) {
        std::cout << "Failed to write headers" << std::endl;
        return false;
    }

    sbf::sbf_integer ints[1000];
    for (int i = 0; i < 1000; i++) {
        ints[i] = i * i;
    }

    //res = file.add_dataset<sbf_integer, 1000>("integer_dataset", ints);

    if(file.close() != sbf::success) {
        std::cout << "Failed to close file" << std::endl;
        return false;
    }
    return true;
}

bool test_read() {
    sbf::File file("/tmp/test_cpp.sbf", sbf::reading);
    if(file.open() != sbf::success) {
        std::cout << "Failed to open" << std::endl;
        return false;
    }

    if(file.read_headers() != sbf::success) {
        std::cout << "Failed to read headers" << std::endl;
        return false;
    }


    if(file.close() != sbf::success) {
        std::cout << "Failed to close file" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if(!test_open_close()) return 1;
    if(!test_write()) return 1;
    if(!test_read()) return 1;
    return 0;
}
