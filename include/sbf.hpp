#pragma once
#include "sbf/dataset.hpp"
#include "sbf/types.hpp"
#include <algorithm>
#include <deque>
#include <fstream>
#include <map>
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
            for (auto item: datasets) {
                file_stream << item;
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
            if(item.data() != nullptr) {
                file_stream.write(
                        reinterpret_cast<const char *>(item.data()),
                        static_cast<std::streamsize>(item.size()));
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
            m_dataset_names[dset.name()] = static_cast<int>(datasets.size());
            datasets.push_back(dset);
        }
        return success;
    }

    ResultType read_datablocks() {
        for (Dataset& dset: datasets) {
            char * data = new char[dset.size()];
            file_stream.read(data, static_cast<std::streamsize>(dset.size()));
            dset._data = reinterpret_cast<void *>(data);
        }
        return success;
    }


    ResultType add_dataset(const Dataset& dset) {
        m_dataset_names[dset.name()] = static_cast<int>(datasets.size());
        datasets.push_back(dset);
        return success;
    }

    bool is_open() const {
        return file_stream.is_open();
    }

    std::size_t n_datasets() const {
        return datasets.size();
    }

    const std::deque<Dataset> get_datasets() const {
        return datasets;
    }

    const Dataset get_dataset(std::string name) const {
        auto search = m_dataset_names.find(name);
        int index = -1;
        if(search != m_dataset_names.end()) {
            index = search->second;
            return datasets[index];
        }
        else {
            return empty;
        }
    }

  private:
    std::fstream file_stream;
    AccessMode accessmode;
    std::string filename;
    std::map<std::string, int> m_dataset_names;
    Dataset empty;
    std::deque<Dataset> datasets;
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
