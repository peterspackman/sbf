# SBF module
from pathlib import Path
import numpy as np
import struct
from enum import Enum

__author__ = "Peter Spackman <peterspackman@fastmail.com>"
__version__ = "0.2.0"

SBF_COLUMN_MAJOR_FLAG = 0b01000000
SBF_FILEHEADER_FMT = "=3s3sb"
SBF_FILEHEADER_SIZE = struct.calcsize(SBF_FILEHEADER_FMT)
# this is everything except the last part of the dataheader
SBF_DATAHEADER_FMT = "=62sbb"
SBF_DATAHEADER_SIZE = struct.calcsize(SBF_DATAHEADER_FMT)
_unpack_fileheader = struct.Struct(SBF_FILEHEADER_FMT).unpack_from
_unpack_dataheader = struct.Struct(SBF_DATAHEADER_FMT).unpack_from

def _c_str_to_str(bytes_arr):
    return (bytes_arr.split(b'\0')[0]).decode('utf-8')



class SBFType(Enum):
    sbf_byte = 0
    sbf_integer = 1
    sbf_long = 2
    sbf_float = 3
    sbf_double = 4
    sbf_complex_float = 5
    sbf_complex_double = 6
    sbf_char = 7

    def as_numpy(self):
        return _sbf_to_numpy_type[self]

_sbf_to_numpy_type = {
    SBFType.sbf_byte: np.uint8,
    SBFType.sbf_integer: np.int32,
    SBFType.sbf_long: np.int64,
    SBFType.sbf_float: np.float32,
    SBFType.sbf_double: np.float64,
    SBFType.sbf_complex_float: np.complex64,
    SBFType.sbf_complex_double: np.complex128,
    SBFType.sbf_char: np.dtype('S1')
}

class Dataset:
    def __init__(self, name, flags, dtype, shape):
        self._name = name
        self._dtype = dtype
        self._flags = flags
        self._column_major = flags & SBF_COLUMN_MAJOR_FLAG
        self._data = None
        self._shape = shape[np.nonzero(shape)]

    @staticmethod
    def from_struct_and_shape(struct, shape):
        string_name = _c_str_to_str(struct[0])
        return Dataset(string_name,
                       struct[1],
                       SBFType(struct[2]),
                       shape)

    def _read_data(self, f):
        n = np.product(self._shape)
        if self.datatype == 'S1' and self.dimensions == 1:
            data = f.read(n)
            self._data = _c_str_to_str(struct.unpack('={}s'.format(n), data)[0])
        else:
            self._data = np.fromfile(f, dtype=self.datatype.as_numpy(),
                                     count=n)
            kwargs = {}
            if self._column_major: kwargs['order'] = 'F'
            self._data = self._data.reshape(self._shape, **kwargs)
                                            
    
    @property
    def name(self):
        return self._name

    @property
    def data(self):
        return self._data

    @property
    def datatype(self):
        return self._dtype

    @property
    def dimensions(self):
        return self._shape.ndim

    def __str__(self):
        return "Dataset('{}', {}, {})".format(self.name, self.datatype.name, self._shape)

    def __repr__(self):
        return str(self)

    def pretty_print(self, show_data=False, **kwargs):
        np.set_printoptions(**kwargs)
        print("dataset:\t'{}'".format(self.name))
        print("dtype:\t\t{}".format(self.datatype.name))
        print("dtype size:\t{} bit".format(np.dtype(self.datatype.as_numpy()).itemsize * 8))
        print("flags:\t\t{:08b}".format(self._flags))
        print("dimensions:\t{}".format(self.dimensions))
        print("shape:\t\t{}".format(self._shape))
        print("storage:\t{}".format("column major" if self._column_major else "row major"))
        print("endianness:\t{}\n".format("little endian"))
        if show_data:
            print('-----------------')
            print(self.data)
            print('-----------------')


class File:

    def __init__(self, path, access_mode='r'):
        if not isinstance(path, Path):
            path = Path(path)
        self._path = path 
        self._datasets = {}
        self._n_datasets = 0

    def read(self):
        with self._path.open("rb") as f:
            self._read_headers(f)
            self._read_data(f)

    def _read_headers(self, f):
        file_header_raw = f.read(SBF_FILEHEADER_SIZE)
        assert(file_header_raw) 
        file_header = _unpack_fileheader(file_header_raw)
        self._n_datasets = file_header[2]
        for i in range(self._n_datasets):
            data_header_raw = f.read(SBF_DATAHEADER_SIZE)
            assert(data_header_raw)
            data_header = _unpack_dataheader(data_header_raw)
            shape = np.fromfile(f, dtype=np.uint64, count=8)
            dataset = Dataset.from_struct_and_shape(data_header, shape)
            self._datasets[dataset.name] = dataset
            
    def _read_data(self, f):
        for name, dataset in self._datasets.items():
            dataset._read_data(f)

    def __getitem__(self, key):
        return self._datasets[key]

    def datasets(self):
        return (d for d in self._datasets.values())

if __name__ == '__main__':
    import time
    import sys
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('paths', nargs='*')
    parser.add_argument('-p', '--print-datasets', action='store_true', default=False,
                        help="Print out the contents of datasets")
    parser.add_argument('-c', '--compare-datasets', action='store_true', default=False,
                        help='Compare the contents of the SBF datasets (like the unix diff tool)')
    args = parser.parse_args()
    for path in args.paths:
        print(path)
        t1 = time.process_time()
        f = File(path)
        f.read()
        t2 = time.process_time()
        print('Time: {:.5}ms'.format((t2 - t1)*1000))
        for dset in f.datasets():
            dset.pretty_print(show_data=args.print_datasets)
