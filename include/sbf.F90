#define SBF_VERSION_MAJOR '0'
#define SBF_VERSION_MINOR '1'
#define SBF_VERSION_MINOR_MINOR '1'

#define SBF_MAX_DIM 8
#define SBF_MAX_DATASETS 16
#define SBF_NAME_LENGTH 62
! Flag bits
#define SBF_BIG_ENDIAN B'10000000'
#define SBF_COLUMN_MAJOR B'01000000'
#define SBF_CUSTOM_DATATYPE B'00100000'
#define SBF_UNUSED_BIT B'00010000'
#define SBF_DIMENSION_BITS B'00001111'
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
    character(sbf_char), dimension(SBF_NAME_LENGTH) :: name = char(0)
    integer(sbf_byte) :: flags = 0
    integer(sbf_data_type) :: data_type = 0
    integer(sbf_size), dimension(SBF_MAX_DIM) :: shape = 0
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
    integer :: filehandle = 11
    integer(sbf_byte) :: n_datasets
    type(sbf_Dataset), dimension(SBF_MAX_DATASETS) :: datasets
    contains
    procedure :: serialize => write_sbf_file
    procedure :: add_dataset => sbf_add_dataset
    procedure :: close => close_sbf_file, open => open_sbf_file
end type

interface sbf_Dataset
    module procedure new_sbf_Dataset_int
    module procedure new_sbf_Dataset_int_2d
end interface

contains
! This is the fun way we get to do generics in 
! fortran without writing the same code again and again

! sbf_integer methods
#define FORTRAN_KIND integer
#define DATATYPE SBF_INT
#define DATA_KIND sbf_integer

#define ROUTINE_NAME new_sbf_Dataset_int
#define DIMENSIONS :
#include "sbf/sbf_dataset_constructor.F90"
#undef ROUTINE_NAME
#undef DIMENSIONS

#define ROUTINE_NAME new_sbf_Dataset_int_2d
#define DIMENSIONS :,:
#include "sbf/sbf_dataset_constructor.F90"
#undef ROUTINE_NAME
#undef DIMENSIONS

subroutine sbf_dh_get_dims(this, res)
    type(sbf_DataHeader), intent(in) :: this
    integer(sbf_byte) :: res
    res = iand(this%flags, SBF_DIMENSION_BITS)
end subroutine

subroutine sbf_dh_set_dims(this, dims)
    type(sbf_DataHeader), intent(inout) :: this
    integer(sbf_byte) :: dims
    this%flags = ior(this%flags, iand(dims, SBF_DIMENSION_BITS))
end subroutine

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

subroutine open_sbf_file(this)
    class(sbf_File), intent(inout) :: this
    logical :: file_exists
    inquire(file=this%filename, exist=file_exists)
    if (.not. file_exists) then
        open(unit=this%filehandle, file=this%filename, &
             status="new", access="stream", form="unformatted")
    else
        open(unit=this%filehandle, file=this%filename, &
             status="old", access="stream", form="unformatted")
    end if
end subroutine
 
subroutine close_sbf_file(this)
    class(sbf_File), intent(inout) :: this
    close(this%filehandle)
end subroutine

subroutine write_sbf_file(this)
    class(sbf_File), intent(inout) :: this
    integer :: i
    type(sbf_FileHeader) :: header
    logical :: is_open
    inquire(this%filehandle, opened=is_open)
    if(.not. is_open) then
        call this%open
    end if
    header%n_datasets = this%n_datasets
    write(this%filehandle) header
    do i = 1, this%n_datasets
        call this%datasets(i)%serialize(this%filehandle)
    end do
    call this%close
end subroutine 

end module