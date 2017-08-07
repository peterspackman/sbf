#pragma once
#include "sbf/dataset.hpp"
#include "sbf/types.hpp"
#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

/*
 * sbf.hpp
 *
 * A simple binary format for storing data.
 * SBF files are designed to be as braindead as possible.
 *
 */
namespace sbf {

/*
 * SBF container class
 *
 * Holds information about file, handles writing/reading datasets
 */
class File {
  public:
    File(std::string name, AccessMode mode = reading)
        : accessmode(mode), filename(name), datasets({}) {
    }

    ResultType open() {
        switch (accessmode) {
        case reading:
            file_stream.open(filename, std::ios::binary | std::ios::in);
            break;
        case writing:
            file_stream.open(filename, std::ios::binary | std::ios::out);
        }
        if (!file_stream.is_open()) {
            return file_open_failure;
        }
        return success;
    }

    ResultType close() {
        for(auto& item: datasets) {
            if(item.second._data != nullptr)
                ::operator delete(item.second._data);
        }
        file_stream.close();
        return success;
    }

    ResultType write_headers() {
        ResultType res = success;

        FileHeader file_header;
        file_header.n_datasets = datasets.size();
        file_stream << file_header;

        if (file_stream.fail()) {
            res = write_failure;
        } else {
            for (const auto item: datasets) {
                file_stream << item.second;
                if (file_stream.fail()) {
                    res = write_failure;
                    break;
                }
            }
        }
        return res;
    }

    ResultType write_datablocks() {
        //avoid calling destructor
        for (const auto& item: datasets) {
            if(item.second._data != nullptr) {
                file_stream.write(
                        reinterpret_cast<const char *>(item.second.data()),
                        item.second.size());
            }
        }
        return success;
    }



    ResultType read_headers() {
        FileHeader file_header;
        file_stream >> file_header;

        if (file_stream.fail() || file_stream.bad()) {
            return read_failure;
        }

        for (auto i = 0; i < file_header.n_datasets; i++) {
            Dataset dset;
            file_stream >> dset;
            if (file_stream.fail()) {
                return read_failure;
            }
            datasets.insert({dset.name(), dset});
        }
        return success;
    }

    ResultType read_datablocks() {
        for (auto& item: datasets) {
            Dataset& dset = item.second;
            dset._data = ::operator new(dset.size());
            file_stream.read(reinterpret_cast<char *>(dset._data), dset.size());
        }
        return success;
    }


    const ResultType add_dataset(const Dataset& dset) {
        datasets.insert({dset.name(), dset});
        return success;
    }

    bool is_open() const {
        return file_stream.is_open();
    }

    const std::size_t n_datasets() const{
        return datasets.size();
    }

    const std::map<std::string, Dataset> get_datasets() {
        return datasets;
    }

    const Dataset get_dataset(std::string name) {
        auto search = datasets.find(name);
        if(search != datasets.end()) {
            return search->second;
        }
        else{
            return Dataset();
        }
    }

  private:
    std::fstream file_stream;
    AccessMode accessmode;
    std::string filename;
    std::map<std::string, Dataset> datasets;
};

File read_file(const std::string& filename) {
    auto file = File(filename, sbf::reading);
    auto status = file.open();
    if(status != success) throw std::runtime_error("Could not open file");
    status = file.read_headers();
    if(status != success) throw status;
    status = file.read_datablocks();
    if(status != success) throw status;
    return file;
}

}
