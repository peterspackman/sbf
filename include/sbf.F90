#define SBF_VERSION_MAJOR '0'
#define SBF_VERSION_MINOR '1'
#define SBF_VERSION_MINOR_MINOR '1'

#define SBF_MAX_DIM 8
#define SBF_MAX_DATASETS 16
#define SBF_NAME_LENGTH 62
! Flag bits
#define SBF_BIG_ENDIAN 0b10000000
#define SBF_COLUMN_MAJOR 0b01000000
#define SBF_CUSTOM_DATATYPE 0b00100000
#define SBF_UNUSED_BIT 0b00010000
#define SBF_DIMENSION_BITS 0b00001111
! Types
#define SBF_BYTE 0
#define SBF_INT 1
#define SBF_LONG 2
#define SBF_FLOAT 3
#define SBF_DOUBLE 4
#define SBF_CFLOAT 5
#define SBF_CDOUBLE 6

#define SBF_RESULT_SUCCESS 1
#define SBF_RESULT_FILE_OPEN_FAILURE 2
#define SBF_RESULT_FILE_CLOSE_FAILURE 3
#define SBF_RESULT_WRITE_FAILURE 4
#define SBF_RESULT_READ_FAILURE 5
#define SBF_RESULT_MAX_DATASETS_EXCEEDED_FAILURE 6

#define SBF_FILE_READONLY 0
#define SBF_FILE_WRITEONLY 1
#define SBF_FILE_READWRITE 2

module sbf
use iso_c_binding


implicit none

! SBF custom datatypes
integer, parameter :: sbf_byte = c_int8_t, sbf_size = c_int64_t, &
    sbf_integer = c_int32_t, sbf_long = c_int64_t, sbf_float = c_float, &
    sbf_double = c_double, sbf_char = c_char, sbf_data_type = sbf_byte

type, public, bind(C) :: sbf_complex_float
    real(sbf_float) :: re, im
end type sbf_complex_float

type, public, bind(C) :: sbf_complex_double
    real(sbf_double) :: re, im
end type sbf_complex_double

type, public, bind(C) :: sbf_FileHeader
    character(sbf_char), dimension(3) :: token = ['S', 'B', 'F']
    character(sbf_char), dimension(3) :: version_string = [ &
        SBF_VERSION_MAJOR, SBF_VERSION_MINOR, SBF_VERSION_MINOR_MINOR]
    integer(sbf_byte) :: n_datasets = 0
end type

type, public, bind(C) :: sbf_DataHeader
    character(sbf_char), dimension(SBF_NAME_LENGTH) :: name
    integer(sbf_byte) :: flags
    integer(sbf_data_type) :: data_type
    integer(sbf_size), dimension(SBF_MAX_DIM) :: shape
end type

type, public :: sbf_Dataset
    character(len=1, kind=sbf_char), dimension(:), allocatable :: data
    type(sbf_DataHeader) :: header
    contains
    procedure :: serialize => write_dataset
end type

type, public :: sbf_File
    integer(sbf_byte) :: mode
    character(len=256) :: filename
    integer(sbf_byte) :: n_datasets
    type(sbf_Dataset), dimension(SBF_MAX_DATASETS) :: datasets
    contains
    procedure :: serialize => write_sbf_file
    procedure :: add_dataset => sbf_add_dataset
end type

interface sbf_Dataset
    module procedure new_sbf_Dataset_int
    module procedure new_sbf_Dataset_int_2d
end interface

contains
! This is the fun way we get to do generics in 
! fortran without writing the same code again and again
#define ROUTINE_NAME new_sbf_Dataset_int
#define DATATYPE sbf_integer
#define DIMENSIONS :
#include "sbf/sbf_dataset_constructor.F90"
#undef ROUTINE_NAME
#undef DATATYPE
#undef DIMENSIONS


#define ROUTINE_NAME new_sbf_Dataset_int_2d
#define DATATYPE sbf_integer
#define DIMENSIONS :,:
#include "sbf/sbf_dataset_constructor.F90"
#undef ROUTINE_NAME
#undef DATATYPE
#undef DIMENSIONS


subroutine write_dataset(this, unit)
    class(sbf_Dataset), intent(in) :: this
    integer :: unit
    write(unit) this%header
    write(unit) this%data
end subroutine

subroutine sbf_add_dataset(this, dset)
    class(sbf_File), intent(inout) :: this
    class(sbf_Dataset), intent(in) :: dset
    this%n_datasets = this%n_datasets + 1
    this%datasets(this%n_datasets) = dset
end subroutine

subroutine write_sbf_file(this)
    class(sbf_File), intent(in) :: this
    integer :: unit = 11, i
    type(sbf_FileHeader) :: header
    header%n_datasets = this%n_datasets
    
    open(unit=11, file=this%filename, access="stream", form="unformatted")
    write(unit) header
    do i = 1, this%n_datasets
        call this%datasets(i)%serialize(unit)
    end do
    close(unit)
end subroutine 

end module