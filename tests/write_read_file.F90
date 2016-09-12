program write_read_file
    use sbf
    use basic_tests
    implicit none
    character(len=256) :: filename = "/tmp/sbf_test_fortran.sbf"
    type(sbf_Dataset) :: dset
    type(sbf_DataHeader) :: header
    type(sbf_File) :: data_file
    integer :: i
    character(sbf_byte), dimension(:), allocatable :: bytes
    integer(sbf_integer), dimension(1000) :: data = [(i*i, i=0,999)]
    data_file%filename = filename
    dset = sbf_Dataset("integer_dataset", data)
    call data_file%add_dataset(dset)
    call data_file%serialize
    print *, size(transfer(header, bytes))
end program