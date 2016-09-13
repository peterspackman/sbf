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
    complex(sbf_float), dimension(100,100) :: cdata = reshape([(i*i, i = 0,9999)], [100, 100])
    integer(sbf_integer), dimension(:), allocatable :: read_data
    integer :: errflag
    print *, "Setting filename"
    data_file_write%filename = filename
    print *, "Creating dataset"
    dset = sbf_Dataset("integer_dataset", data)
    print *, "Adding dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("complex_dataset", cdata)
    print *, "Adding complex dataset"
    call data_file_write%add_dataset(dset)
    print *, "Writing file"
    call data_file_write%serialize
    print *, "Closing file"
    call data_file_write%close
    print *, "Opening file" 
    data_file_read%filename = filename
    print *, "Deserializing"
    call data_file_read%deserialize
    print *, "Getting dataset"
    call data_file_read%get("integer_dataset", read_data, errflag)
    print *, "Closing"
    call data_file_read%close
    if (errflag == 1) then
        if(all(abs(data - read_data) == 0)) print *, "equal"

    else; print *, "There was an error reading the dataset: ", sbf_strerr(errflag)
    end if

end program
