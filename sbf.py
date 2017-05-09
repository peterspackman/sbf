# SBF module
from pathlib import Path
import numpy as np
import struct
from collections import OrderedDict
from enum import IntEnum

__author__ = "Peter Spackman <peterspackman@fastmail.com>"
__version__ = "0.2.0"
SBF_FILEHEADER_FMT = "=3s3sb"
SBF_FILEHEADER_SIZE = struct.calcsize(SBF_FILEHEADER_FMT)
# this is everything except the last part of the dataheader
# which is the shape array
SBF_DATAHEADER_FMT = "=62sbb"
SBF_DATAHEADER_SIZE = struct.calcsize(SBF_DATAHEADER_FMT)
_unpack_fileheader = struct.Struct(SBF_FILEHEADER_FMT).unpack_from
_unpack_dataheader = struct.Struct(SBF_DATAHEADER_FMT).unpack_from
_pack_fileheader = struct.Struct(SBF_FILEHEADER_FMT).pack
_pack_dataheader = struct.Struct(SBF_DATAHEADER_FMT).pack

_OUTPUT_FORMAT_STRING = """dataset:\t'{self.name}'
dtype:\t\t{self.datatype.name}
dtype size:\t{datatype_width}
flags:\t\t{self.flags.binary:08b}
dimensions:\t{self.dimensions}
shape:\t\t{self._shape}
storage:\t{storage}
endianness:\t{endianness}{data_sep}{data}{data_sep}
"""


def read_file(filepath):
    """Helper method to read an SBF file from a given filepath
    """
    f = File(filepath)
    f.read()
    return f


def bytes2str(bytes_arr):
    """Helper method to convert a null terminated array of bytes
    to a unicode string.
    """
    return (bytes_arr.split(b'\0')[0]).decode('utf-8')


class InvalidDatasetError(Exception):
    pass


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

    @staticmethod
    def from_numpy_type(numpy_type):
        return _numpy_to_sbf_type[numpy_type]


_sbf_to_numpy_type = {
    SBFType.sbf_byte: np.dtype('uint8'),
    SBFType.sbf_integer: np.dtype('int32'),
    SBFType.sbf_long: np.dtype('int64'),
    SBFType.sbf_float: np.dtype('float32'),
    SBFType.sbf_double: np.dtype('float64'),
    SBFType.sbf_complex_float: np.dtype('complex64'),
    SBFType.sbf_complex_double: np.dtype('complex128'),
    SBFType.sbf_char: np.dtype('S1')
}

_numpy_to_sbf_type = {v: k for k, v in _sbf_to_numpy_type.items()}


class Flags:
    """Wrapper around the byte of flags per dataset, storing information
    about endianness, storage order and number of dimensions

    Keyword arguments
    binary -- integer/binary value of the flags (default 0b0000000)

    >>> f = Flags(column_major=True, dimensions=3)
    >>> f
    Flags(dim=3, col=True)
    >>> f.set_column_major(False)
    >>> f.column_major
    False
    >>> f.dimensions
    3
    >>> f.set_dimensions(2)
    >>> f.dimensions
    2
    """
    column_major_bit = 0b01000000
    dimension_bits = 0b00001111

    def __init__(self, *, binary=0, 
                 column_major=False,
                 dimensions=0):

        self.binary = binary
        self.set_column_major(column_major)
        self.set_dimensions(dimensions)
        self.binary = binary
            
    @property
    def dimensions(self):
        return self.binary & Flags.dimension_bits

    def set_dimensions(self, dims):
        self.clear_bits(Flags.dimension_bits)
        self.set_bits(dims & Flags.dimension_bits)

    @property
    def column_major(self):
        return bool(self.binary & Flags.column_major_bit)

    def set_column_major(self, value):
        if value:
            self.set_bits(Flags.column_major_bit)
        else:
            self.clear_bits(Flags.column_major_bit)

    def set_bits(self, mask):
        self.binary |= mask

    def clear_bits(self, mask):
        self.binary &= ~mask

    def __repr__(self):
        return "Flags(dim={d}, col={c})".format(
                d=self.dimensions,
                c=self.column_major)


class Dataset:
    """Corresponds to a dataset inside an sbf file,
    consisting of a header, and a binary blob

    Arguments:
    name -- the identifier of this dataset
    data -- numpy array of data to store

    Keyword arguments:
    flags -- manually set the flags
    dtype -- manually set the datatype
    shape -- manually set the shape
    """
    def __init__(self, name, data, *, flags=None, dtype=None, shape=None):
        data = np.array(data)
        self._data = data
        self._name = name
        self._dtype = SBFType.from_numpy_type(data.dtype)
        self._shape = np.array(data.shape)
        self._flags = Flags(dimensions=self._shape.size)
        if flags:
            self._flags = flags

    def set_data(self, data, flags=None):
        data = np.array(data)
        self._data = data
        self._dtype = SBFType.from_numpy_type(data.dtype)
        self._shape = np.array(data.shape)
        self._flags = Flags(dimensions=self._shape.size)
        if flags:
            self._flags = flags

    @staticmethod
    def empty():
        return Dataset('', [])

    @staticmethod
    def _from_header(struct, shape):
        name = bytes2str(struct[0])
        flags = struct[1]
        dtype = SBFType(struct[2])
        dset = Dataset.empty()
        dset._name = name
        dset._dtype = dtype
        dset._flags = Flags(binary=flags)
        dset._data = None
        dset._shape = shape[np.nonzero(shape)]
        return dset

    def _read_data(self, f):
        n = np.product(self._shape)
        if self.datatype == SBFType.sbf_char and self.dimensions == 1:
            data = f.read(n)
            self._data = bytes2str(struct.unpack('={}s'.format(n), data)[0])
        else:
            self._data = np.fromfile(f, dtype=self.datatype.as_numpy(),
                                     count=n)
            kwargs = {}
            if self.flags.column_major:
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
    def flags(self):
        return self._flags

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
                storage="column major" if
                        self.flags.column_major else "row major",
                endianness="little endian",
                data_sep=sep, data=data)
        print(output)


class File:

    def __init__(self, path):
        if not isinstance(path, Path):
            path = Path(path)
        self._path = path
        self._datasets = OrderedDict()
        self._n_datasets = 0

    def read(self):
        with self._path.open("rb") as f:
            self._read_headers(f)
            self._read_data(f)

    def write(self):
        with self._path.open('wb') as f:
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
            dataset = Dataset._from_header(data_header, shape)
            self._datasets[dataset.name] = dataset

    def _write_headers(self, f):
        file_header = _pack_fileheader(b'SBF', b'020', self._n_datasets)
        f.write(file_header)
        for dataset in self._datasets.values():
            flags = dataset._flags
            # if we're writing a file, unset the column major bit as
            # numpy arrays are stored row major
            if flags.column_major:
                flags.set_column_major(False)
            data_header = _pack_dataheader(
                    dataset.name.encode('utf-8'),
                    flags.binary,
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

    def __setitem__(self, key, item):
        if key in self._datasets:
            self._datasets[key].set_data(item)
        else:
            self._n_datasets += 1
            self._datasets[key] = Dataset(key, item)

    def add_dataset(self, name, data):
        self.__setitem__(name, data)

    def datasets(self):
        return (d for d in self._datasets.values())


def main():
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
