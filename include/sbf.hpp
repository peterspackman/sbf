#pragma once
#include "sbf/dataset.hpp"
#include "sbf/types.hpp"
#include <algorithm>
#include <deque>
#include <fstream>
#include <map>
#include <cerrno>
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

    enum Status {
        Open,
        Closed,
        FailedOpening,
        FailedClosing,
        FailedReadingHeaders,
        FailedReadingDatablocks
    };

    File() {}
    File(std::string name, AccessMode mode = reading, bool read = true)
        : accessmode(mode), filename(name), m_status(Closed), datasets({}) {

        if(mode == sbf::writing || !read) return;
        auto status = open();
        if (status != sbf::success) {
            m_status = FailedOpening;
            return;
        }
        else m_status = Open;

        status = read_headers();
        if (status != sbf::success) {
            m_status = FailedReadingHeaders;
            return;
        }
        status = read_datablocks();
        if (status != sbf::success) {
            m_status = FailedReadingDatablocks;
            return;
        }
        status = close();
        if (status == sbf::success) m_status = Closed;
        else m_status = FailedClosing;
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

        if (!file_stream) {
            res = write_failure;
        } else {
            for (auto item: datasets) {
                file_stream << item;
                if (!file_stream) {
                    res = write_failure;
                    break;
                }
            }
        }
        std::cerr << "Error " << strerror(errno) << std::endl;
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

    const Status status() const { return m_status; }

  private:
    std::fstream file_stream;
    AccessMode accessmode;
    std::string filename;
    Status m_status;
    std::map<std::string, int> m_dataset_names;
    Dataset empty;
    std::deque<Dataset> datasets;
};

}
