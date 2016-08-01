#pragma once
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cstdint>
#include <complex>
#include <vector>
#include <deque>
#include <array>

#define SBF_VERSION_MAJOR '0'
#define SBF_VERSION_MINOR '1'
#define SBF_VERSION_MINOR_MINOR '1'

#define SBF_MAX_DIM 8
#define SBF_MAX_DATASETS 16
#define SBF_NAME_LENGTH 32
#define SBF_ROW_MAJOR 0b01000000
#define SBF_BIG_ENDIAN 0b1000000
#define SBF_DIMENSION_BITS 0b00001111

/*
 * sbf.hpp
 *
 * A simple binary format for storing data.
 * SBF files are designed to be as braindead as possible.
 *
 */
namespace sbf {

    typedef uint8_t sbf_byte;
    typedef uint64_t sbf_size;
    typedef int32_t sbf_integer;
    typedef int64_t sbf_long;
    typedef float sbf_float;
    typedef double sbf_double;
    typedef char sbf_character;
    typedef std::complex<float> sbf_complex_float;
    typedef std::complex<double> sbf_complex_double;

    // DATA TYPE FLAGS
    enum DataType {
        SBF_INT = 1,
        SBF_LONG,
        SBF_FLOAT,
        SBF_DOUBLE,
        SBF_CFLOAT,
        SBF_CDOUBLE
    };

    enum AccessMode {
        reading,
        writing
    };

    // RESULT TYPE FLAGS
    enum ResultType {
        success = 1,
        file_open_failure,
        file_close_failure,
        write_failure,
        read_failure,
        null_failure,
        max_datasets_exceeded_failure
    } sbf_result;

    struct FileHeader {

        FileHeader() {
            token = {{'S', 'B', 'F'}};
            version_string = {{SBF_VERSION_MAJOR, SBF_VERSION_MINOR, SBF_VERSION_MINOR_MINOR}};
            n_datasets = 0;
        } 

        std::array<sbf_character, 3> token;
        std::array<sbf_character, 3> version_string;
        sbf_byte n_datasets;
    };

    std::ostream& operator<<(std::ostream& os, const FileHeader& f) {
        os.write(reinterpret_cast<const char*>(&f), sizeof(f));
        return os;
    }

    std::istream& operator>>(std::istream& is, FileHeader& f) {
        is.read(reinterpret_cast<char*>(&f), sizeof(f));
        return is;
    }


    struct DataHeader {
        std::array<sbf_character, SBF_NAME_LENGTH> name; // what is the name of this dataset
        sbf_byte flags;
        DataType data_type;     // how big is each block of data
        std::array<sbf_size, SBF_MAX_DIM> shape; // how many blocks of data do we have

        std::size_t size() {
            std::size_t s = 4 * shape[0];
            for (auto i = 1; i < SBF_MAX_DIM; i++) {
                if (shape[i] == 0)
                    break;
                s *= shape[i];
            }
            return s;
        }
    };

    class File {
        public:
        File(std::string name, AccessMode mode=reading) : accessmode(mode), filename(name),  datasets(0) {}

        ResultType open() {
            switch(accessmode) {
                case reading:
                    file_stream.open(filename, std::ios::binary | std::ios::in);
                    break;
                case writing:
                    file_stream.open(filename, std::ios::binary | std::ios::out);
            }
            if(!file_stream.is_open()) {
                return file_open_failure;
            }
            return success;
        }

        ResultType close() {
            file_stream.close();

            return success;
        }


        ResultType write_headers() {
            FileHeader file_header;
            file_header.n_datasets = datasets.size();
            file_stream << file_header;

            if (file_stream.fail()) {
                return write_failure;
            }

            for(const auto dataset: datasets) {
                DataHeader header = dataset.first;
                file_stream.write(reinterpret_cast<char*>(&header), sizeof(header));
                if (file_stream.fail()) {
                    return write_failure;
                }

            }
            return success;
        }

        ResultType read_headers() {
            FileHeader file_header;
            file_stream >> file_header;

            if (file_stream.fail()) {
                return read_failure;
            }

            for(auto i = 0; i < file_header.n_datasets; i++) {
                DataHeader header;
                file_stream.read(reinterpret_cast<char*>(&header), sizeof(header));
                if (file_stream.fail()) {
                    return read_failure;
                }
                datasets.push_back(std::make_pair(header, nullptr));
            }
            return success;
        }

        bool is_open() {
            return file_stream.is_open();
        }

        private:

        std::fstream file_stream;
        AccessMode accessmode;
        std::string filename;
        std::deque<std::pair<DataHeader, void *>> datasets;
    };
}
