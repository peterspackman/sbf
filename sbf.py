# SBF module
from pathlib import Path
import numpy as np
import struct

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

types = {
    0: np.uint8,
    1: np.int32,
    2: np.int64,
    3: np.float32,
    4: np.float64,
    5: np.complex64,
    6: np.complex128,
    7: np.dtype('S1')
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
        string_name = (struct[0].split(b'\0'))[0].decode('utf-8')
        return Dataset(string_name,
                       struct[1],
                       types[struct[2]],
                       shape)

    def _read_data(self, f):
        n = np.product(self._shape)
        if self._dtype == 'S1' and self._shape.ndim == 1:
            data = f.read(n)
            self._data = struct.unpack('={}s'.format(n), data)[0].decode('utf-8')
        else:
            self._data = np.fromfile(f, dtype=self._dtype,
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

    def __str__(self):
        return 'Dataset({}, {}, {})'.format(self._name, repr(self._dtype), self._shape)

    def __repr__(self):
        return str(self)


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
    np.set_printoptions(threshold=10)
    for path in sys.argv[1:]:
        print(path)
        t1 = time.process_time()
        f = File(path)
        f.read()
        t2 = time.process_time()
        print('Time: {:.5}ms'.format((t2 - t1)*1000))
        for dset in f.datasets():
            print(dset)
