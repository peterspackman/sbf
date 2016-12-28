# SBF (Simple Binary Format) v0.2.0

A simple binary format for storing data.
SBF files are designed to be as braindead as possible, 
i.e. easy to write, easy to read.

# Why? 

It's nice to have a simple binary data format
for straightforward applications that don't have complex
requirements. (e.g. HDF5, netCDF etc.) Plus it's (much)
faster than passing ASCII/UTF8 encoded numerical data around.

# What data formats does it support, how are they defined?

As of now there are byte, character, integer,
long, float, double, complex float and complex
double datatypes.

These are defined as follows:
```
sbf_byte             =   C uint8_t
sbf_character        =   C char
sbf_integer          =   C int32_t
sbf_long             =   C int64_t 
sbf_float            =   C float
sbf_double           =   C double
sbf_complex_float    =   2 C floats  {real, imaginary}
sbf_complex_double   =   2 C doubles {real, imaginary}
```

# Does this format support XYZ?

If XYZ is anything other than writing binary data arrays
of dimension up to 8, consisting of the aforementioned 
data types, then no.

# Why not XYZ {hierarchical data, compression etc.}?

Basically:
- I want to keep the format as simple as possible, have only simple
responsibilities.

- I believe hierarchy can be done by the file system and directories etc.

- I might support compression later, but doing so requires dependencies,
something I don't wish to add. 

# File Structure

An SBF File is structured as follows:
```
+--------------------+
|   sbf_FileHeader   | file description e.g. number of datasets
+--------------------+
|  sbf_DataHeader(s) | 0 or more, one for EACH dataset, containing a 
|        ...         | description of what is stored in `binary_data`
+--------------------+ 
|    binary_blobs    | binary data described in data header section(s)
+--------------------+
```
