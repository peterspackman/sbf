#pragma once
#include "sbf/types.hpp"
#include <array>
#include <vector>

namespace sbf {
class File;
/*
 * File header container structure
 *
 * Contains SBF basic information such as token and
 * version strings, along with the number of datasets
 * in the file.
 */
struct FileHeader {
    FileHeader() {
        token_version_string = {{'S', 'B', 'F', sbf_version_major, sbf_version_minor, sbf_version_minor_minor}};
        n_datasets = 0;
    }

    std::array<sbf_character, 6> token_version_string;
    sbf_byte n_datasets;
};

/*
 * Serialize this FileHeader into a datastream
 */
std::ostream &operator<<(std::ostream &os, const FileHeader &f) {
    // Fields are done separately in order to avoid potential struct padding
    // issues
    os.write(reinterpret_cast<const char *>(&(f.token_version_string)),
             sizeof(f.token_version_string));
    os.write(reinterpret_cast<const char *>(&(f.n_datasets)),
             sizeof(f.n_datasets));
    return os;
}

/*
 * Deserialize from a datastream in to this FileHeader
 */
std::istream &operator>>(std::istream &is, FileHeader &f) {
    is.read(reinterpret_cast<char *>(&(f.token_version_string)),
            sizeof(f.token_version_string));
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
class Dataset {
  friend class File;
  public:
    Dataset() {
    }
    Dataset(const std::string &name_string) {
        _name = as_sbf_string(name_string);
    }

    Dataset(const std::string &name_string, const sbf_dimensions &shape,
            const DataType type, void * data,
            const sbf_byte flags = flags::default_flags) {
        _data = data;
        _name = as_sbf_string(name_string);
        _flags = flags;
        std::copy(begin(shape), end(shape), begin(_shape));
        sbf_byte dims = 0;
        for(dims = 0; (dims < _shape.size()) && (_shape[dims]); dims++){};
        set_dimensions(dims);
        _type = type;
    }

    const std::string name() const {
        return as_string(_name);
    }

    sbf_string raw_name() const {
        return _name;
    }

    /* Is the 'flags' big endian bit set?*/
    inline const bool is_big_endian() const {
        return _flags & flags::big_endian;
    }

    /* Is the 'flags' big endian bit not set? (equivalent to !is_big_endian() */
    inline const bool is_little_endian() const {
        return !is_big_endian();
    }

    inline const bool is_empty() const {
        return get_dimensions() == 0;
    }

    /* Extract number of dimensions from 'flags'?*/
    inline const sbf_byte get_dimensions() const {
        return (_flags & flags::dimension_bits);
    }

    inline void set_dimensions(sbf_byte dimensions) {
        _flags |= (dimensions & flags::dimension_bits);
    }

    const sbf_byte get_flags() const {
        return _flags;
    }

    const DataType get_type() const {
        return _type;
    }

    const sbf_dimensions get_shape() const {
        return _shape;
    }

    const std::vector<sbf_size> get_shape_vector() const {
        return std::vector<sbf_size>(_shape.data(), _shape.data() + _shape.size());
    }

    const void * data() const {
        return _data;
    }

    template<typename T>
    T * data_as() const {
        return reinterpret_cast<T*>(_data);
    }

    /*
     * Size of the datatype, in bytes
     *
     * TODO fix implementation to not be so dirty
     */
    const std::size_t datatype_size() const {
        sbf_size bytes = 0;
        switch (_type) {
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
    const std::size_t size() const {
        std::size_t product = datatype_size();
        for (const auto dim : _shape) {
            if (dim == 0)
                break;
            product *= dim;
        }
        return product;
    }

    friend std::ostream &operator<<(std::ostream &os, const Dataset &dset);
    friend std::istream &operator>>(std::istream &is, Dataset &dset);

  private:
    void * _data = nullptr;
    sbf_string _name;
    sbf_byte _flags = flags::default_flags;
    DataType _type = SBF_BYTE;     // how big is each block of data
    sbf_dimensions _shape = {{0}}; // how many blocks of data do we have
};

/*
 * Serialize this Dataset into a datastream
 */
std::ostream &operator<<(std::ostream &os, const Dataset &f) {
    // Fields are done separately in order to avoid potential struct padding
    // issues
    auto raw_name = f.raw_name();
    auto flags = f.get_flags();
    auto type = f.get_type();
    auto shape = f.get_shape();
    os.write(reinterpret_cast<const char *>(&(raw_name)), sizeof(raw_name));
    os.write(reinterpret_cast<const char *>(&(flags)), sizeof(flags));
    os.write(reinterpret_cast<const char *>(&(type)), sizeof(type));
    os.write(reinterpret_cast<const char *>(&(shape)), sizeof(shape));
    return os;
}

/*
 * Deserialize from a data stream into this Dataset
 */
std::istream &operator>>(std::istream &is, Dataset &dset) {
    is.read(reinterpret_cast<char *>(&(dset._name)), sizeof(dset._name));
    is.read(reinterpret_cast<char *>(&(dset._flags)), sizeof(dset._flags));
    is.read(reinterpret_cast<char *>(&(dset._type)), sizeof(dset._type));
    is.read(reinterpret_cast<char *>(&(dset._shape)), sizeof(dset._shape));
    return is;
}
}
