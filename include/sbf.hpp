#pragma once
#include <algorithm>
#include <fstream>
#include <map>
#include <cerrno>
#include <iostream>
#include <iterator>
#include <array>
#include <vector>
#include <complex>
#include <cstdint>
#include <string>

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

constexpr sbf_byte sbf_version_major('0');
constexpr sbf_byte sbf_version_minor('2');
constexpr sbf_byte sbf_version_minor_minor('0');

namespace limits {
constexpr sbf_size max_dataset_dimensions(8);
constexpr sbf_size name_length(62);
constexpr sbf_size n_datasets_max(64);
}

namespace flags {
constexpr sbf_byte big_endian(0b10000000);
constexpr sbf_byte column_major(0b01000000);
constexpr sbf_byte custom_datatype(0b00101111);
constexpr sbf_byte unused_bit(0b00010000);
constexpr sbf_byte dimension_bits(0b00001111);
// changes depending on platform
constexpr sbf_byte default_flags(0b00000000);
}

typedef std::array<sbf_size, limits::max_dataset_dimensions> sbf_dimensions;
typedef std::array<sbf_character, limits::name_length> sbf_string;

std::string as_string(sbf_string sbf_str) {
    std::string str;
    std::copy_n(begin(sbf_str), sbf_str.size(), std::back_inserter(str));
    str.erase(str.find('\0'));
    return str;
}

sbf_string as_sbf_string(std::string str) {
    sbf_string sbf_str {{0}};
    std::copy_n(begin(str), std::min(sbf_str.size(), str.size()),
                begin(sbf_str));
    return sbf_str;
}

// DATA TYPE FLAGS
enum DataType : sbf_byte {
    SBF_BYTE = 0,
    SBF_INT,
    SBF_LONG,
    SBF_FLOAT,
    SBF_DOUBLE,
    SBF_CFLOAT,
    SBF_CDOUBLE,
    SBF_CHAR
};

enum AccessMode { reading = std::ios::in, writing = std::ios::out };

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
    constexpr static size_t header_size = sizeof(token_version_string) + sizeof(n_datasets);
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

template <typename T> struct SBFTypeTraits {
    static constexpr char const *type_name = "sbf_byte";
    static const size_t size = 1;
    static const bool is_specialized = false;
    static const DataType type = SBF_BYTE;
};

template<>
struct SBFTypeTraits<sbf_integer> {
    static constexpr char const *type_name = "sbf_integer";
    static const size_t size = sizeof(sbf_integer);
    static const bool is_specialized = true;
    static const DataType type = SBF_INT;
};

template<>
struct SBFTypeTraits<sbf_double> {
    static constexpr char const *type_name = "sbf_double";
    static const size_t size = sizeof(sbf_double);
    static const bool is_specialized = true;
    static const DataType type = SBF_DOUBLE;
};


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

private:
size_t _offset = 0;
sbf_string _name = {{0}};
sbf_byte _flags = flags::default_flags;
DataType _type = SBF_BYTE;     // how big is each block of data
sbf_dimensions _shape = {{0}}; // how many blocks of data do we have
bool _written_to_file = false;

public:
constexpr static size_t header_size = sizeof(_name) +
    sizeof(_flags) + sizeof(_type) + sizeof(_shape);

Dataset() {}

Dataset(const std::string &name_string)
    : _name(as_sbf_string(name_string)) {}

Dataset(const std::string &name_string, const sbf_dimensions &shape,
        const DataType type, const sbf_byte flags = flags::default_flags)
    : _name(as_sbf_string(name_string)), _flags(flags), _type(type)
{
    std::copy(begin(shape), end(shape), begin(_shape));
    sbf_byte dims = 0;
    for(dims = 0; (dims < _shape.size()) && (_shape[dims]); dims++){};
    set_dimensions(dims);
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
    case SBF_CHAR:
        bytes = sizeof(sbf_character);
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

};

/*
 * Serialize this Dataset into a datastream
 */
std::ostream &operator<<(std::ostream &os, const Dataset &dset) {
    // Fields are done separately in order to avoid potential struct padding
    // issues
    os.write(reinterpret_cast<const char *>(&(dset._name)), sizeof(dset._name));
    os.write(reinterpret_cast<const char *>(&(dset._flags)), sizeof(dset._flags));
    os.write(reinterpret_cast<const char *>(&(dset._type)), sizeof(dset._type));
    os.write(reinterpret_cast<const char *>(&(dset._shape)), sizeof(dset._shape));
    return os;
}

/*
 * Deserialize from a data stream into this Dataset
 */
std::istream &operator>>(std::istream &is, Dataset &dset) {
    dset._written_to_file = true;
    is.read(reinterpret_cast<char *>(&(dset._name)), sizeof(dset._name));
    is.read(reinterpret_cast<char *>(&(dset._flags)), sizeof(dset._flags));
    is.read(reinterpret_cast<char *>(&(dset._type)), sizeof(dset._type));
    is.read(reinterpret_cast<char *>(&(dset._shape)), sizeof(dset._shape));
    return is;
}

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

    ResultType read_headers() {
        FileHeader file_header;
        file_stream >> file_header;

        if (file_stream.fail() || file_stream.bad()) {
            return read_failure;
        }

        size_t offset = Dataset::header_size * file_header.n_datasets + FileHeader::header_size;
        for (auto i = 0; i < file_header.n_datasets; i++) {
            Dataset dset;
            file_stream >> dset;
            if (file_stream.fail()) {
                return read_failure;
            }
            dset._offset = offset;
            m_dataset_names[dset.name()] = static_cast<int>(datasets.size());
            datasets.push_back(dset);
            offset += dset.size();
        }
        return success;
    }

    // read a dataset
    template<typename T, class Traits = SBFTypeTraits<T>>
    ResultType read_data(const std::string& dset_name, T *data) {
        auto dset = get_dataset(dset_name);
        bool valid = (Traits::type == dset.get_type());
        if(!valid) return ResultType::read_failure;
        if(!is_open()) return ResultType::read_failure;
        file_stream.seekg(dset._offset);
        file_stream.read(reinterpret_cast<char*>(data),
                         static_cast<std::streamsize>(dset.size()));
        return ResultType::success; 
    }

    // read a dataset
    template<typename T, class Traits = SBFTypeTraits<T>>
    ResultType write_data(const std::string& dset_name, T *data) {
        auto dset = get_dataset(dset_name);
        bool valid = (Traits::type == dset.get_type());
        if(!valid) return ResultType::write_failure;
        if(data != nullptr) {
            file_stream.seekg(dset._offset);
            std::cout << "File@ " << file_stream.tellg() 
                << " should be @ " << dset._offset << "\n";
            file_stream.write(
                    reinterpret_cast<const char *>(data),
                    static_cast<std::streamsize>(dset.size()));
        }
        dset._written_to_file = true;
        return ResultType::success; 
    }



    ResultType add_dataset(Dataset& dset) {
        // add +1 for this dataset
        size_t offset = FileHeader::header_size + (datasets.size() + 1) * Dataset::header_size;
        for(auto& x: datasets) {
            x._offset = offset; 
            offset += x.size(); 
        }
        std::cout << "Offset of '" << dset.name() <<  "' data = " << offset << "\n";
        m_dataset_names[dset.name()] = static_cast<int>(datasets.size());
        dset._offset = offset;
        datasets.push_back(dset);
        return success;
    }

    bool is_open() const {
        return file_stream.is_open();
    }

    std::size_t n_datasets() const {
        return datasets.size();
    }

    const std::vector<Dataset> get_datasets() const {
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
    std::vector<Dataset> datasets;
};

}
