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
    real(sbf_double), dimension(10,10,10,10,10,10) :: ddata 
    complex(sbf_float), dimension(:,:), allocatable :: read_cdata
    integer(sbf_integer), dimension(:), allocatable :: read_data
    real(sbf_double), dimension(:,:,:,:,:,:), allocatable :: read_ddata
    real(sbf_float) :: write_scalar = 5.25, read_scalar = 0
    character(len=100) :: char_array = "test string please ignore"
    character(len=:), allocatable :: string
    real :: start, finish
    integer :: errflag
    ddata = 3.14159
    print *, "Setting filename"
    data_file_write%filename = filename
    print *, "Creating dataset"
    dset = sbf_Dataset("integer_dataset", data)
    print *, "Adding dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("complex_dataset", cdata)
    print *, "Adding complex dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("double_dataset", ddata)
    print *, "Adding double dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("character_array_dataset", char_array)
    print *, "Adding character dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("float scalar dataset", write_scalar)
    print *, "Adding scalar dataset"
    call data_file_write%add_dataset(dset)
    call cpu_time(start)
    print *, "Writing file"
    call data_file_write%serialize
    call cpu_time(finish)
    print '("Time = ",f6.3," seconds.")',finish-start
    print *, "Closing file"
    call data_file_write%close
    print *, "------------------ WRITING DONE ------------------"
    print *, "Opening file" 
    data_file_read%filename = filename
    call cpu_time(start)
    print *, "Deserializing"
    call data_file_read%deserialize
    call cpu_time(finish)
    print '("Time = ",f6.3," seconds.")',finish-start
    print *, "Getting datasets"
    call cpu_time(start)
    call data_file_read%get("complex_dataset", read_cdata, errflag)
    call data_file_read%get("integer_dataset", read_data, errflag)
    call data_file_read%get("double_dataset", read_ddata, errflag)
    call data_file_read%get("character_array_dataset", string, errflag)
    call data_file_read%get("float scalar dataset", read_scalar, errflag)
    call cpu_time(finish)
    print '("Time = ",f6.3," seconds.")',finish-start
    print *, "Closing"
    call data_file_read%close
    if (errflag == 1) then
        if(all(abs(data - read_data) == 0)) print *, "integer_dataset: all equal"
        if(all(abs(cdata - read_cdata) == 0)) print *, "complex_dataset: all equal"
        if(all(abs(ddata - read_ddata) == 0)) print *, "double_dataset: all equal"
        if(string == char_array) print *, "strings: equal"
        if( write_scalar == read_scalar) print *, "scalars: equal"
    else; print *, "There was an error reading the dataset: ", sbf_strerr(errflag)
    end if
end program
