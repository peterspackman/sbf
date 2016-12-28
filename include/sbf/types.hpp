#pragma once
#include <array>
#include <complex>
#include <cstdint>
#include <string>

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
constexpr sbf_size n_datasets_max(16);
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
    sbf_string sbf_str;
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

enum AccessMode { reading, writing };

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
}
