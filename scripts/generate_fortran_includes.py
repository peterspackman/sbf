
datatypes = {'integer': ['sbf_byte', 'sbf_integer', 'sbf_long'], 
             'real': ['sbf_float', 'sbf_double'], 'character': ['sbf_char'],
             'complex': ['sbf_float', 'sbf_double']}
datatype_ids = {'sbf_byte': 'SBF_BYTE', 'sbf_integer': 'SBF_INT', 'sbf_long': 'SBF_LONG',
                'sbf_float': 'SBF_FLOAT', 'sbf_double': 'SBF_DOUBLE', 'sbf_char': 'SBF_BYTE'}

complex_datatype_ids = {'sbf_float': 'SBF_CFLOAT', 'sbf_double':'SBF_CDOUBLE'}

MAX_DIMS = 8
fortran_kind_string = """#define FORTRAN_KIND {fortran_kind}"""
header_string = """
! {comment}
#define DATATYPE {datatype_id}
#define DATA_KIND {data_kind}
"""
routine_string = """
#define ROUTINE_NAME new_sbf_Dataset_{abbrev}_{dims}d
#define DIMENSIONS {dim_string}
#include "sbf_dataset_constructor.F90"
#undef ROUTINE_NAME
#undef DIMENSIONS
"""
footer_string = """
#undef DATATYPE
#undef DATA_KIND
"""
routine_names = []
for fortran_kind, kinds in datatypes.items():
    print(fortran_kind_string.format(fortran_kind=fortran_kind))
    for data_kind in kinds:
        if fortran_kind == 'complex':
            datatype_id = complex_datatype_ids[data_kind]
        else:
            datatype_id = datatype_ids.get(data_kind, 'complex??')
        comment = 'sbf {} methods'.format(data_kind)
        print(header_string.format(comment=comment, 
                                   datatype_id=datatype_id,
                                   data_kind=data_kind))
        for dims in range(1,MAX_DIMS+1):
            colons = [':' for i in range(dims)]
            dim_string = ",".join(colons)
            abbrev = ("cpx_" if fortran_kind == "complex" else "") + data_kind
            routine_names.append("new_sbf_Dataset_{abbrev}_{dims}d".format(abbrev=abbrev, dims=dims))
            print(routine_string.format(abbrev=abbrev, dims=dims, dim_string=dim_string))
        print(footer_string)
    print("#undef FORTRAN_KIND")

print(", ".join(routine_names))

