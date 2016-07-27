# SBF (Simple Binary Format)
A simple binary format for storing data.
SBF files are designed to be as braindead as possible.

# Why? 

It's nice to have a simple binary data format
for straightforward applications that don't have complex
requirements. (e.g. HDF5, netCDF etc.)

# What data formats does it support, how are they defined?

As of now we have integer, long, float, double,
complex float and complex double datatypes.

These are defined as follows:
```
integer              =   C int32_t
long                 =   C int64_t 
float                =   C float
double               =   C double
complex float        =   2 C floats  {real, imaginary}
complex double       =   2 C doubles {real, imaginary}
```

# Does this format support XYZ?

If XYZ is anything other than writing binary data arrays
of dimension up to 16, consisting of the aforementioned 
data types, then no.

# Why not XYZ {hierarchical data, compression etc.}?

Basically:
- I want to keep the format as simple as possible

- I believe hierarchy can be done by the file system and directories etc.

- I might support compression later, but doing so requires dependencies,
something I don't wish to add.

An SBF File is structured as follows:
+--------------------+
|   sbf_FileHeader   | file description e.g. number of datasets
+--------------------+
|   sbf_DataHeader   | 0 or more, one for EACH dataset, containing a 
|        ...         | description of what is stored in `binary_data`
+--------------------+ 
|    binary_data     | binary data described in header section
+--------------------+


