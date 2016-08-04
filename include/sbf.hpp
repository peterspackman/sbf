#pragma once
#include "sbf/headers.hpp"
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
            for (const auto dataset : datasets) {
                DataHeader header = dataset.first;
                file_stream << header;
                if (file_stream.fail()) {
                    res = write_failure;
                    break;
                }
            }
        }
        return res;
    }

    ResultType read_headers() {
        FileHeader file_header;
        file_stream >> file_header;

        if (file_stream.fail() || file_stream.bad()) {
            return read_failure;
        }

        for (auto i = 0; i < file_header.n_datasets; i++) {
            DataHeader header;
            file_stream >> header;
            std::cout << "DataHeader(" << header.name_string() << ")"
                      << std::endl;
            if (file_stream.fail()) {
                return read_failure;
            }
            datasets.push_back(std::make_pair(header, nullptr));
        }
        return success;
    }

    ResultType add_dataset(std::string name, sbf_dimensions shape,
                           DataType data_type, void *data,
                           sbf_byte flags = flags::default_flags) {
        DataHeader header(name);
        header.data_type = data_type;

        sbf_byte dimensions;
        for(dimensions = 0; dimensions < shape.size(); dimensions++) {
            if(shape[dimensions] == 0) break;
        }
        header.set_dimensions(dimensions);
        std::copy_n(begin(shape), dimensions, begin(header.shape));
        datasets.push_back(std::make_pair(header, data));
        return success;
    }

    bool is_open() {
        return file_stream.is_open();
    }

    std::size_t n_datasets() {
        return datasets.size();
    }

  private:
    std::fstream file_stream;
    AccessMode accessmode;
    std::string filename;
    std::deque<std::pair<DataHeader, void *>> datasets;
};
}
