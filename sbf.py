# SBF module
from pathlib import Path
import numpy as np
import struct
import ctypes
from collections import OrderedDict
from enum import IntEnum

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
_pack_fileheader = struct.Struct(SBF_FILEHEADER_FMT).pack
_pack_dataheader = struct.Struct(SBF_DATAHEADER_FMT).pack

_OUTPUT_FORMAT_STRING = """dataset:\t'{self.name}'
dtype:\t\t{self.datatype.name}
dtype size:\t{datatype_width}
flags:\t\t{self._flags:08b}
dimensions:\t{self.dimensions}
shape:\t\t{self._shape}
storage:\t{storage}
endianness:\t{endianness}{data_sep}{data}{data_sep}
"""


def bytes2str(bytes_arr):
    return (bytes_arr.split(b'\0')[0]).decode('utf-8')


class SBFType(IntEnum):
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
    """Corresponds to a datset inside an sbf file"""
    def __init__(self, name, flags, dtype, shape):
        self._name = name
        self._dtype = dtype
        self._flags = flags
        self._column_major = flags & SBF_COLUMN_MAJOR_FLAG
        self._data = None
        self._shape = shape[np.nonzero(shape)]

    @staticmethod
    def from_struct_and_shape(struct, shape):
        string_name = bytes2str(struct[0])
        return Dataset(string_name,
                       struct[1],
                       SBFType(struct[2]),
                       shape)

    def _read_data(self, f):
        n = np.product(self._shape)
        if self.datatype == SBFType.sbf_char and self.dimensions == 1:
            data = f.read(n)
            self._data = bytes2str(struct.unpack('={}s'.format(n), data)[0])
        else:
            self._data = np.fromfile(f, dtype=self.datatype.as_numpy(),
                                     count=n)
            kwargs = {}
            if self._column_major:
                kwargs['order'] = 'F'
            self._data = self._data.reshape(self._shape, **kwargs)

    def _write_header(self, f):
        struct

    def _write_datablock(self, f):
        np.save(f, self.data)

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
        return self._shape.size

    def _sbf_shape(self):
        arr = np.zeros(8, dtype=np.uint64)
        arr[:self.dimensions] = self._shape[:]
        return arr

    def __str__(self):
        return "Dataset('{}', {}, {})".format(
                self.name, self.datatype.name, self._shape)

    def __repr__(self):
        return str(self)

    def is_string(self):
        return (self.datatype == SBFType.sbf_char and
                self.dimensions == 1)

    def pretty_print(self, show_data=False, **kwargs):
        np.set_printoptions(**kwargs)
        sep = '\n----------------\n' if show_data else ''
        data = self.data if show_data else ''
        output = _OUTPUT_FORMAT_STRING.format(
                self=self,
                datatype_width=np.dtype(self.datatype.as_numpy()).itemsize * 8,
                storage="column major" if self._column_major else "row major",
                endianness="little endian",
                data_sep=sep, data=data)
        print(output)


class File:

    def __init__(self, path, access_mode='r'):
        if not isinstance(path, Path):
            path = Path(path)
        self._path = path
        self._datasets = OrderedDict()
        self._n_datasets = 0

    def read(self):
        with self._path.open("rb") as f:
            self._read_headers(f)
            self._read_data(f)

    def write(self, filename):
        with Path(filename).open('wb') as f:
            self._write_headers(f)
            self._write_data(f)

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

    def _write_headers(self, f):
        print(SBF_FILEHEADER_SIZE)
        file_header = _pack_fileheader(b'SBF', b'020', self._n_datasets)
        f.write(file_header)
        for dataset in self._datasets.values():
            flags = dataset._flags
            if flags | SBF_COLUMN_MAJOR_FLAG:
                flags &= ~(SBF_COLUMN_MAJOR_FLAG)
            data_header = _pack_dataheader(
                    dataset.name.encode('utf-8'),
                    flags,
                    int(dataset._dtype))
            f.write(data_header)
            dataset._sbf_shape().tofile(f)

    def _read_data(self, f):
        for name, dataset in self._datasets.items():
            dataset._read_data(f)

    def _write_data(self, f):
        for dataset in self._datasets.values():
            if dataset.is_string():
                np.fromstring(dataset.data,
                              dtype=np.uint8,
                              count=dataset._shape[0]).tofile(f)
            else:
                dataset.data.tofile(f)

    def __getitem__(self, key):
        return self._datasets[key]

    def datasets(self):
        return (d for d in self._datasets.values())

def main():
    import sys
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('paths', nargs='*')
    parser.add_argument('-p', '--print-datasets',
                        action='store_true', default=False,
                        help="Print out the contents of datasets")
    parser.add_argument('-c', '--compare-datasets',
                        action='store_true', default=False,
                        help='Compare the contents of the SBF datasets'
                             '(like the unix diff tool)')
    args = parser.parse_args()
    for path in args.paths:
        print(path)
        f = File(path)
        f.read()
        for dset in f.datasets():
            dset.pretty_print(show_data=args.print_datasets)
