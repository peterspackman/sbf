program write_read_file
    use sbf
    implicit none
    character(len=256) :: filename = "/tmp/sbf_test_fortran.sbf"
    type(sbf_Dataset) :: dset
    type(sbf_DataHeader) :: header
    type(sbf_File) :: data_file_write, data_file_read
    integer :: i
    character(sbf_byte), dimension(:), allocatable :: bytes
    integer(sbf_integer), dimension(1000) :: data = [(i*i, i=0,999)]
    integer(sbf_integer), dimension(:), allocatable :: read_data
    data_file_write%filename = filename
    dset = sbf_Dataset("integer_dataset", data)
    call data_file_write%add_dataset(dset)
    call data_file_write%serialize
    call data_file_write%close
    print *, "Opening file" 
    data_file_read%filename = filename
    print *, "Deserializing"
    call data_file_read%deserialize
    print *, "Getting dataset"
    call data_file_read%get("integer_dataset", read_data)
    print *, "closing"
    call data_file_read%close
    if(all(abs(data - read_data) == 0)) print *, "equal"

end program