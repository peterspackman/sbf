#pragma once
#include "sbf/types.hpp"
#include <array>

namespace sbf {
/*
 * File header container structure
 *
 * Contains SBF basic information such as token and
 * version strings, along with the number of datasets
 * in the file.
 */
struct FileHeader {
    FileHeader() {
        token = {{'S', 'B', 'F'}};
        version_string = {
            {sbf_version_major, sbf_version_minor, sbf_version_minor_minor}};
        n_datasets = 0;
    }

    std::array<sbf_character, 3> token;
    std::array<sbf_character, 3> version_string;
    sbf_byte n_datasets;
};

/*
 * Serialize this FileHeader into a datastream
 */
std::ostream &operator<<(std::ostream &os, const FileHeader &f) {
    // Fields are done separately in order to avoid potential struct padding
    // issues
    os.write(reinterpret_cast<const char *>(&(f.token)), sizeof(f.token));
    os.write(reinterpret_cast<const char *>(&(f.version_string)),
             sizeof(f.version_string));
    os.write(reinterpret_cast<const char *>(&(f.n_datasets)),
             sizeof(f.n_datasets));
    return os;
}

/* 
 * Deserialize from a datastream in to this FileHeader
 */
std::istream &operator>>(std::istream &is, FileHeader &f) {
    is.read(reinterpret_cast<char *>(&(f.token)), sizeof(f.token));
    is.read(reinterpret_cast<char *>(&(f.version_string)),
            sizeof(f.version_string));
    is.read(reinterpret_cast<char *>(&(f.n_datasets)), sizeof(f.n_datasets));
    return is;
}

/*
 * Data header container structure
 *
 * Consists of the following:
 * - The name of the dataset
 * - The data_type of the dataset
 * - Flags (e.g. row major, little endian)
 * - dimensions of the dataset
 */
struct DataHeader {
    DataHeader() {
    }

    DataHeader(const std::string &name_string) {
        std::copy_n(begin(name_string),
                    std::min(name.size(), name_string.size()), begin(name));
    }

    // 62 'bytes'
    std::array<sbf_character, limits::name_length> name = {
        {0}}; // what is the name of this dataset
    // 1 byte
    sbf_byte flags = flags::default_flags;
    // 1 byte
    DataType data_type = SBF_BYTE; // how big is each block of data
    // 8 * 8 bytes
    sbf_dimensions shape = {{0}}; // how many blocks of data do we have

    std::string name_string() {
        std::string n;
        std::copy_n(begin(name), name.size(), std::back_inserter(n));
        return n;
    }

    /* Is the 'flags' big endian bit set?*/
    inline const bool is_big_endian() {
        return flags & flags::big_endian;
    }

    /* Is the 'flags' big endian bit not set? (equivalent to !is_big_endian() */
    inline const bool is_little_endian() {
        return !is_big_endian();
    }

    /* Extract number of dimensions from 'flags'?*/
    inline const sbf_byte dimensions() {
        return (flags & flags::dimension_bits);
    }

    inline void set_dimensions(sbf_byte dimensions) {
        flags |= (dimensions & flags::dimension_bits);
    }
    
    /*
     * Size of the datatype, in bytes
     *
     * TODO fix implementation to not be so dirty
     */
    const std::size_t datatype_size() {
        sbf_size bytes = 0;
        switch (data_type) {
        case SBF_BYTE:
            bytes = sizeof(sbf_byte);
            break;
        case SBF_DOUBLE:
            bytes = sizeof(sbf_double);
            break;
        case SBF_INT:
            bytes = sizeof(sbf_integer);
            break;
        case SBF_LONG:
            bytes = sizeof(sbf_long);
            break;
        case SBF_FLOAT:
            bytes = sizeof(sbf_float);
            break;
        case SBF_CFLOAT:
            bytes = sizeof(sbf_complex_float);
            break;
        case SBF_CDOUBLE:
            bytes = sizeof(sbf_complex_double);
            break;
        }
        return bytes;
    }

    /* Total number of bytes occupied by the binary blob of this dataset,
     * i.e. num_blocks * block_size 
     */
    const std::size_t size() {
        std::size_t product = datatype_size();
        for (const auto dim: shape) {
            if (dim == 0)
                break;
            product *= dim;
        }
        return product;
    }
};

/*
 * Serialize this DataHeader into a datastream
 */
std::ostream &operator<<(std::ostream &os, const DataHeader &f) {
    // Fields are done separately in order to avoid potential struct padding
    // issues
    os.write(reinterpret_cast<const char *>(&(f.name)), sizeof(f.name));
    os.write(reinterpret_cast<const char *>(&(f.flags)), sizeof(f.flags));
    os.write(reinterpret_cast<const char *>(&(f.data_type)),
             sizeof(f.data_type));
    os.write(reinterpret_cast<const char *>(&(f.shape)), sizeof(f.shape));
    return os;
}

/*
 * Deserialize from a data stream into this DataHeader
 */
std::istream &operator>>(std::istream &is, DataHeader &f) {
    is.read(reinterpret_cast<char *>(&(f.name)), sizeof(f.name));
    is.read(reinterpret_cast<char *>(&(f.flags)), sizeof(f.flags));
    is.read(reinterpret_cast<char *>(&(f.data_type)), sizeof(f.data_type));
    is.read(reinterpret_cast<char *>(&(f.shape)), sizeof(f.shape));
    return is;
}
}
