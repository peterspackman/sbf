#define SBF_VERSION_MAJOR '0'
#define SBF_VERSION_MINOR '1'
#define SBF_VERSION_MINOR_MINOR '1'

#define SBF_MAX_DIM 8
#ifndef SBF_MAX_DATASETS
#define SBF_MAX_DATASETS 16
#endif
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
#define SBF_RESULT_DATASET_NOT_FOUND 7
#define SBF_RESULT_INCOMPATIBLE_DATA_TYPES 8

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

integer(sbf_byte), parameter :: sbf_writeonly = 2, sbf_readonly = 5, sbf_readwrite = 6

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
    procedure :: serialize => write_dataset, deserialize => read_dataset
end type

type, public :: sbf_File
    !!! class for information about a SBF
    !!!
    !!! Members:
    !!!     mode        file mode to use with `open` [default=sbf_readonly]
    !!!     filename    the name of the file on disk [default='out.sbf']
    !!!     filehandle  the fortran file `unit` for use with `open`/`close` [default=11]
    !!!     n_datasets  the number of datasets in this object [default=0]
    !!!     datasets    the array of actual datasets, size SBF_MAX_DATASETS
    !!!
    !!! Methods:
    !!!     serialize, deserialize      write/read an sbf file to/from disk 
    !!!
    !!!     add_dataset                 append a dataset to this file to be written later 
    !!!
    !!!     open/close                  wrappers for calls to fortran's `open`/`close`
    !!!                                 will need to be used to open a file for writing.
    !!!     get                         populate the given array with the dataset from this file
    !!!                                 matching `name`. 
    integer(sbf_byte) :: mode = sbf_readonly
    character(len=256) :: filename = "out.sbf"
    integer :: filehandle = 11
    integer(sbf_byte) :: n_datasets = 0
    type(sbf_Dataset), dimension(SBF_MAX_DATASETS) :: datasets
    contains
    procedure :: serialize => write_sbf_file, deserialize => read_sbf_file
    procedure :: add_dataset => sbf_add_dataset
    procedure :: close => close_sbf_file, open => open_sbf_file
    ! getters
    generic, public :: get =>  get_sbf_Dataset_sbf_char_1d, get_sbf_Dataset_sbf_char_2d, &
        get_sbf_Dataset_sbf_char_3d, get_sbf_Dataset_sbf_char_4d, &
        get_sbf_Dataset_sbf_char_5d, get_sbf_Dataset_sbf_char_6d, &
        get_sbf_Dataset_sbf_char_7d, get_sbf_Dataset_sbf_byte_1d, &
        get_sbf_Dataset_sbf_byte_2d, get_sbf_Dataset_sbf_byte_3d, &
        get_sbf_Dataset_sbf_byte_4d, get_sbf_Dataset_sbf_byte_5d, &
        get_sbf_Dataset_sbf_byte_6d, get_sbf_Dataset_sbf_byte_7d, &
        get_sbf_Dataset_sbf_integer_1d, get_sbf_Dataset_sbf_integer_2d, &
        get_sbf_Dataset_sbf_integer_3d, get_sbf_Dataset_sbf_integer_4d, &
        get_sbf_Dataset_sbf_integer_5d, get_sbf_Dataset_sbf_integer_6d, &
        get_sbf_Dataset_sbf_integer_7d, get_sbf_Dataset_sbf_long_1d, &
        get_sbf_Dataset_sbf_long_2d, get_sbf_Dataset_sbf_long_3d, &
        get_sbf_Dataset_sbf_long_4d, get_sbf_Dataset_sbf_long_5d, &
        get_sbf_Dataset_sbf_long_6d, get_sbf_Dataset_sbf_long_7d, &
        get_sbf_Dataset_sbf_float_1d, get_sbf_Dataset_sbf_float_2d, &
        get_sbf_Dataset_sbf_float_3d, get_sbf_Dataset_sbf_float_4d, &
        get_sbf_Dataset_sbf_float_5d, get_sbf_Dataset_sbf_float_6d, &
        get_sbf_Dataset_sbf_float_7d, get_sbf_Dataset_sbf_double_1d, &
        get_sbf_Dataset_sbf_double_2d, get_sbf_Dataset_sbf_double_3d, &
        get_sbf_Dataset_sbf_double_4d, get_sbf_Dataset_sbf_double_5d, &
        get_sbf_Dataset_sbf_double_6d, get_sbf_Dataset_sbf_double_7d, &
        get_sbf_Dataset_cpx_sbf_float_1d, get_sbf_Dataset_cpx_sbf_float_2d, &
        get_sbf_Dataset_cpx_sbf_float_3d, get_sbf_Dataset_cpx_sbf_float_4d, &
        get_sbf_Dataset_cpx_sbf_float_5d, get_sbf_Dataset_cpx_sbf_float_6d, &
        get_sbf_Dataset_cpx_sbf_float_7d, get_sbf_Dataset_cpx_sbf_double_1d, &
        get_sbf_Dataset_cpx_sbf_double_2d, get_sbf_Dataset_cpx_sbf_double_3d, &
        get_sbf_Dataset_cpx_sbf_double_4d, get_sbf_Dataset_cpx_sbf_double_5d, &
        get_sbf_Dataset_cpx_sbf_double_6d, get_sbf_Dataset_cpx_sbf_double_7d

    procedure, private :: get_sbf_Dataset_sbf_char_1d, get_sbf_Dataset_sbf_char_2d, &
        get_sbf_Dataset_sbf_char_3d, get_sbf_Dataset_sbf_char_4d, &
        get_sbf_Dataset_sbf_char_5d, get_sbf_Dataset_sbf_char_6d, &
        get_sbf_Dataset_sbf_char_7d, get_sbf_Dataset_sbf_byte_1d, &
        get_sbf_Dataset_sbf_byte_2d, get_sbf_Dataset_sbf_byte_3d, &
        get_sbf_Dataset_sbf_byte_4d, get_sbf_Dataset_sbf_byte_5d, &
        get_sbf_Dataset_sbf_byte_6d, get_sbf_Dataset_sbf_byte_7d, &
        get_sbf_Dataset_sbf_integer_1d, get_sbf_Dataset_sbf_integer_2d, &
        get_sbf_Dataset_sbf_integer_3d, get_sbf_Dataset_sbf_integer_4d, &
        get_sbf_Dataset_sbf_integer_5d, get_sbf_Dataset_sbf_integer_6d, &
        get_sbf_Dataset_sbf_integer_7d, get_sbf_Dataset_sbf_long_1d, &
        get_sbf_Dataset_sbf_long_2d, get_sbf_Dataset_sbf_long_3d, &
        get_sbf_Dataset_sbf_long_4d, get_sbf_Dataset_sbf_long_5d, &
        get_sbf_Dataset_sbf_long_6d, get_sbf_Dataset_sbf_long_7d, &
        get_sbf_Dataset_sbf_float_1d, get_sbf_Dataset_sbf_float_2d, &
        get_sbf_Dataset_sbf_float_3d, get_sbf_Dataset_sbf_float_4d, &
        get_sbf_Dataset_sbf_float_5d, get_sbf_Dataset_sbf_float_6d, &
        get_sbf_Dataset_sbf_float_7d, get_sbf_Dataset_sbf_double_1d, &
        get_sbf_Dataset_sbf_double_2d, get_sbf_Dataset_sbf_double_3d, &
        get_sbf_Dataset_sbf_double_4d, get_sbf_Dataset_sbf_double_5d, &
        get_sbf_Dataset_sbf_double_6d, get_sbf_Dataset_sbf_double_7d, &
        get_sbf_Dataset_cpx_sbf_float_1d, get_sbf_Dataset_cpx_sbf_float_2d, &
        get_sbf_Dataset_cpx_sbf_float_3d, get_sbf_Dataset_cpx_sbf_float_4d, &
        get_sbf_Dataset_cpx_sbf_float_5d, get_sbf_Dataset_cpx_sbf_float_6d, &
        get_sbf_Dataset_cpx_sbf_float_7d, get_sbf_Dataset_cpx_sbf_double_1d, &
        get_sbf_Dataset_cpx_sbf_double_2d, get_sbf_Dataset_cpx_sbf_double_3d, &
        get_sbf_Dataset_cpx_sbf_double_4d, get_sbf_Dataset_cpx_sbf_double_5d, &
        get_sbf_Dataset_cpx_sbf_double_6d, get_sbf_Dataset_cpx_sbf_double_7d
end type

! Interfaces
interface sbf_Dataset
    module procedure new_sbf_Dataset_sbf_byte_1d, new_sbf_Dataset_sbf_byte_2d, &
    new_sbf_Dataset_sbf_byte_3d, new_sbf_Dataset_sbf_byte_4d, &
    new_sbf_Dataset_sbf_byte_5d, new_sbf_Dataset_sbf_byte_6d, &
    new_sbf_Dataset_sbf_byte_7d, &
    new_sbf_Dataset_sbf_integer_1d, new_sbf_Dataset_sbf_integer_2d, &
    new_sbf_Dataset_sbf_integer_3d, new_sbf_Dataset_sbf_integer_4d, &
    new_sbf_Dataset_sbf_integer_5d, new_sbf_Dataset_sbf_integer_6d, &
    new_sbf_Dataset_sbf_integer_7d, &
    new_sbf_Dataset_sbf_long_1d, new_sbf_Dataset_sbf_long_2d, &
    new_sbf_Dataset_sbf_long_3d, new_sbf_Dataset_sbf_long_4d, &
    new_sbf_Dataset_sbf_long_5d, new_sbf_Dataset_sbf_long_6d, &
    new_sbf_Dataset_sbf_long_7d, &
    new_sbf_Dataset_sbf_char_1d, new_sbf_Dataset_sbf_char_2d, &
    new_sbf_Dataset_sbf_char_3d, new_sbf_Dataset_sbf_char_4d, &
    new_sbf_Dataset_sbf_char_5d, new_sbf_Dataset_sbf_char_6d, &
    new_sbf_Dataset_sbf_char_7d,  &
    new_sbf_Dataset_cpx_sbf_float_1d, new_sbf_Dataset_cpx_sbf_float_2d, &
    new_sbf_Dataset_cpx_sbf_float_3d, new_sbf_Dataset_cpx_sbf_float_4d, &
    new_sbf_Dataset_cpx_sbf_float_5d, new_sbf_Dataset_cpx_sbf_float_6d, &
    new_sbf_Dataset_cpx_sbf_float_7d, &
    new_sbf_Dataset_cpx_sbf_double_1d, new_sbf_Dataset_cpx_sbf_double_2d, &
    new_sbf_Dataset_cpx_sbf_double_3d, new_sbf_Dataset_cpx_sbf_double_4d, &
    new_sbf_Dataset_cpx_sbf_double_5d, new_sbf_Dataset_cpx_sbf_double_6d, &
    new_sbf_Dataset_cpx_sbf_double_7d, &
    new_sbf_Dataset_sbf_float_1d, new_sbf_Dataset_sbf_float_2d, &
    new_sbf_Dataset_sbf_float_3d, new_sbf_Dataset_sbf_float_4d, &
    new_sbf_Dataset_sbf_float_5d, new_sbf_Dataset_sbf_float_6d, &
    new_sbf_Dataset_sbf_float_7d, &
    new_sbf_Dataset_sbf_double_1d, new_sbf_Dataset_sbf_double_2d, &
    new_sbf_Dataset_sbf_double_3d, new_sbf_Dataset_sbf_double_4d, &
    new_sbf_Dataset_sbf_double_5d, new_sbf_Dataset_sbf_double_6d, &
    new_sbf_Dataset_sbf_double_7d
end interface

contains

! This is the 'fun' way we get to do 'generics' in 
! fortran without writing the same code again and again

! create methods
#include "sbf/sbf_dataset_constructors.F90"

! get methods
#include "sbf/sbf_get_datasets.F90"

logical function sbf_dt_compatible(sbf_dtype, size_bytes)
    integer(sbf_byte) :: sbf_dtype
    integer(sbf_integer) :: int
    integer(sbf_long) :: long
    real(sbf_float) :: float
    real(sbf_double) :: double
    integer(c_size_t) :: size_bytes
    integer :: sbf_dtype_size_bytes
    select case (sbf_dtype)
        case (SBF_BYTE)
            sbf_dtype_size_bytes = c_sizeof(sbf_dtype)
        case (SBF_FLOAT)
            sbf_dtype_size_bytes = c_sizeof(float)
        case (SBF_INT)
            sbf_dtype_size_bytes = c_sizeof(int)
        case (SBF_DOUBLE)
            sbf_dtype_size_bytes = c_sizeof(double)
        case (SBF_LONG)
            sbf_dtype_size_bytes = c_sizeof(long)
        case (SBF_CFLOAT)
            sbf_dtype_size_bytes = 2 * c_sizeof(float)
        case (SBF_CDOUBLE)
            sbf_dtype_size_bytes = 2 * c_sizeof(double)
        case default
            sbf_dtype_size_bytes = 1
    end select
    print *, "sbf_dtype_size_bytes = ", sbf_dtype_size_bytes, "size_bytes = ", size_bytes
    sbf_dt_compatible = (sbf_dtype_size_bytes == size_bytes)
end function

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
    ! placeholder wrapper in case we want to change behaviour in the future
    write(unit) this%header
    write(unit) this%data
end subroutine

subroutine read_dataset(this, unit)
    class(sbf_Dataset), intent(inout) :: this
    integer :: unit, i
    integer(sbf_byte) :: dims
    integer(sbf_size) :: n_bytes = 4
    ! placeholder wrapper in case we want to change behaviour in the future
    read(unit) this%header
    call sbf_dh_get_dims(this%header, dims)
    do i = 1, dims
        n_bytes = n_bytes * this%header%shape(i)
    end do
    allocate(this%data(n_bytes))
    read(unit) this%data
end subroutine

logical function match(char_array, str)
    character(sbf_char), dimension(SBF_NAME_LENGTH), intent(in) :: char_array
    character(len=*), intent(in) :: str
    integer :: i
    match = .true.
    do i = 1, len(str) 
        if (char_array(i) .ne. str(i:i)) then
            match = .false.
            return
        end if
    end do
end function

integer function index_of_dataset_by_name(this, name)
    character(len=*) :: name
    class(sbf_File), intent(in) :: this
    integer :: i
    index_of_dataset_by_name = -1
    do i = 1, this%n_datasets
        if (match(this%datasets(i)%header%name, name)) then
            index_of_dataset_by_name = i
            return
        end if
    end do
end function

subroutine sbf_add_dataset(this, dset)
    class(sbf_File), intent(inout) :: this
    class(sbf_Dataset), intent(in) :: dset
    ! increment the store of the number of datasets
    this%n_datasets = this%n_datasets + 1
    ! assign it
    this%datasets(this%n_datasets) = dset
end subroutine

subroutine open_sbf_file(this, mode)
    class(sbf_File), intent(inout) :: this
    integer(sbf_byte), optional :: mode
    logical :: file_exists

    ! default: read only
    character(len=10) :: action = "read"
    if(present(mode)) then
        select case (mode)
            case (sbf_writeonly)
                action = "write"
            case (sbf_readwrite)
                action = "readwrite"
            case default
                action = "read"
        end select
    endif

    ! check if the file exists
    inquire(file=this%filename, exist=file_exists)
    if (.not. file_exists) then
        open(unit=this%filehandle, file=this%filename, &
             status="new", access="stream", &
             action=action, form="unformatted")
    else
        open(unit=this%filehandle, file=this%filename, &
             status="old", access="stream", &
             action=action, form="unformatted")
    end if
end subroutine
 
subroutine close_sbf_file(this)
    class(sbf_File), intent(inout) :: this
    logical :: is_open
    ! check that the file is open
    inquire(this%filehandle, opened=is_open)
    ! if so: close it
    if(is_open) then
        close(this%filehandle)
    end if
end subroutine

subroutine write_sbf_file(this)
    class(sbf_File), intent(inout) :: this
    integer :: i
    type(sbf_FileHeader) :: header
    logical :: is_open

    ! check whether the file is already open, if not: open it
    inquire(this%filehandle, opened=is_open)
    if(.not. is_open) then
        call this%open(mode=sbf_writeonly)
    end if

    ! set up the file header
    header%n_datasets = this%n_datasets

    ! write the header
    write(this%filehandle) header

    ! write all the datasets
    do i = 1, this%n_datasets
        call this%datasets(i)%serialize(this%filehandle)
    end do

    ! close the file
    call this%close
end subroutine 

subroutine read_sbf_file(this)
    class(sbf_File), intent(inout) :: this
    type(sbf_FileHeader) :: header
    integer :: i
    logical :: is_open

    inquire(this%filehandle, opened=is_open)
    if(.not. is_open) then
        call this%open(mode=sbf_readonly)
    end if

    read(this%filehandle) header
    this%n_datasets = header%n_datasets
    do i = 1, this%n_datasets
        call this%datasets(i)%deserialize(this%filehandle)
    end do

    call this%close
end subroutine

function sbf_strerr(code)
    integer :: code
    character(len=128) :: sbf_strerr
    sbf_strerr = "success"
    select case (code)
        case(SBF_RESULT_INCOMPATIBLE_DATA_TYPES)
            sbf_strerr = "SBF: Incompatible data types in `get` call"
        case(SBF_RESULT_DATASET_NOT_FOUND)
            sbf_strerr = "SBF: Dataset not found"
        case(SBF_RESULT_READ_FAILURE)
            sbf_strerr = "SBF: Read failure"
        case(SBF_RESULT_WRITE_FAILURE)
            sbf_strerr = "SBF: Write failure"
        case(SBF_RESULT_MAX_DATASETS_EXCEEDED_FAILURE)
            sbf_strerr = "SBF: Maximum number of datasets exceeded"
        case(SBF_RESULT_FILE_CLOSE_FAILURE)
            sbf_strerr = "SBF: Failed to close file"
        case(SBF_RESULT_FILE_OPEN_FAILURE)
            sbf_strerr = "SBF: Failed to open file"
    end select
end function

end module