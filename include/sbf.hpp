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
        : accessmode(mode), filename(name), datasets(0) {
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
            for (const auto dataset: datasets) {
                file_stream << dataset;
                if (file_stream.fail()) {
                    res = write_failure;
                    break;
                }
            }
        }
        return res;
    }

    ResultType write_datablocks() {
        for (Dataset& dset: datasets) {
            file_stream.write(reinterpret_cast<const char *>(dset.data()), dset.size());
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
            datasets.push_back(dset);
        }
        return success;
    }

    ResultType read_datablocks() {
        for (Dataset& dset: datasets) {
            char * data = new char[dset.size()];
            file_stream.read(data, dset.size());
            dset._data = reinterpret_cast<void *>(data);
        }
        return success;
    }


    const ResultType add_dataset(const Dataset& dset) {
        datasets.push_back(dset);
        return success;
    }

    bool is_open() const {
        return file_stream.is_open();
    }

    const std::size_t n_datasets() const{
        return datasets.size();
    }

    const std::deque<Dataset> get_datasets() {
        return datasets;
    }

    const Dataset get_dataset(std::string name) {
        for(const auto d: datasets) {
            if(d.name() == name) return d;
        }
        return Dataset();
    }

  private:
    std::fstream file_stream;
    AccessMode accessmode;
    std::string filename;
    std::deque<Dataset> datasets;
};
}
