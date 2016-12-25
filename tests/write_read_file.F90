program write_read_file
    use sbf
    implicit none
    character(len=256) :: filename = "/tmp/sbf_test_fortran.sbf"
    type(sbf_Dataset) :: dset
    type(sbf_DataHeader) :: header
    type(sbf_File) :: data_file_write, data_file_read
    integer :: i,j,k,l,m
    character(sbf_byte), dimension(:), allocatable :: bytes
    integer(sbf_integer), dimension(10) :: data = [(i*i, i=0,9)]
    integer(sbf_long), dimension(2,2,2,2,2) :: ldata
    complex(sbf_float), dimension(5,5) :: cdata
    real(sbf_double), dimension(4,4,4,4) :: ddata 
    real(sbf_float), dimension(5,5,5) :: fdata 
    complex(sbf_float), dimension(:,:), allocatable :: read_cdata
    integer(sbf_integer), dimension(:), allocatable :: read_data
    integer(sbf_long), dimension(:,:,:,:,:), allocatable :: read_ldata
    real(sbf_double), dimension(:,:,:,:), allocatable :: read_ddata
    real(sbf_float), dimension(:,:,:), allocatable :: read_fdata
    real(sbf_float) :: write_scalar = 5.25, read_scalar = 0
    character(len=100) :: char_array = "test string please ignore"
    character(len=:), allocatable :: string
    real :: start, finish
    integer :: errflag

    do i = 1,size(fdata, 1)
    do j = 1,size(fdata, 2)
    do k = 1,size(fdata, 3)
        fdata(i,j,k) = i * 100 + j * 10 + k
    end do
    end do
    end do

    do i = 1,size(cdata, 1)
    do j = 1,size(cdata, 2)
        cdata(i,j) = cmplx(i,j)
    end do
    end do

    do i = 1,size(ddata, 1)
    do j = 1,size(ddata, 2)
    do k = 1,size(ddata, 3)
    do l = 1,size(ddata, 4)
        ddata(i,j,k,l) = i * 1000 + j * 100 + k * 10 + l
    end do
    end do
    end do
    end do

    do i = 1,size(ldata, 1)
    do j = 1,size(ldata, 2)
    do k = 1,size(ldata, 3)
    do l = 1,size(ldata, 4)
    do m = 1,size(ldata, 5)
        ldata(i,j,k,l,m) = i * 10000 + j * 1000 + k * 100 + l * 10 + m
    end do
    end do
    end do
    end do
    end do


    print *, "Setting filename"
    data_file_write%filename = filename
    print *, "Creating dataset"
    print *, "Adding dataset"
    call data_file_write%add_dataset(sbf_Dataset("integer_dataset", data))
    print *, "Adding long dataset"
    call data_file_write%add_dataset(sbf_Dataset("long_integer_dataset", ldata))
    dset = sbf_Dataset("complex_dataset", cdata)
    print *, "Adding complex dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("double_dataset", ddata)
    print *, "Adding double dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("float_dataset", fdata)
    print *, "Adding float dataset"
    call data_file_write%add_dataset(dset)
    dset = sbf_Dataset("string_dataset", char_array)
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
    if (errflag .ne. 1) then
        print *, "There was an error reading the complex_dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif

    call data_file_read%get("integer_dataset", read_data, errflag)
    if (errflag .ne. 1) then
        print *, "There was an error reading the integer_dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif

    call data_file_read%get("long_integer_dataset", read_ldata, errflag)
    if (errflag .ne. 1) then
        print *, "There was an error reading the long_integer_dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif

    call data_file_read%get("double_dataset", read_ddata, errflag)
    if (errflag .ne. 1) then
        print *, "There was an error reading the double_dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif

    call data_file_read%get("float_dataset", read_fdata, errflag)
    if (errflag .ne. 1) then
        print *, "There was an error reading the float_dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif

    call data_file_read%get("string_dataset", string, errflag)
    if (errflag .ne. 1) then
        print *, "There was an error reading the string_dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif
   
    call data_file_read%get("float scalar dataset", read_scalar, errflag)
    if (errflag .ne. 1) then
        print *, "There was an error reading the scalar dataset: ", sbf_strerr(errflag)
        call exit(1)
    endif
    call cpu_time(finish)
    print '("Time = ",f6.3," seconds.")',finish-start

    print *, "Closing file"
    call data_file_read%close
    if(.not. (all(abs(data - read_data) == 0))) then
        print *, "integer_dataset: not all are equal"
        call exit(1)
    endif
    if(.not. (all(abs(cdata - read_cdata) == 0))) then
        print *, "complex_dataset: not all are equal"
        call exit(1)
    endif
    if(.not. (all(abs(ldata - read_ldata) == 0))) then
        print *, "long_integer_dataset: not all are equal"
        call exit(1)
    endif
    if(.not. (all(abs(ddata - read_ddata) == 0))) then
        print *, "double_dataset: not all are equal"
        call exit(1)
    endif
    if(.not. (all(abs(fdata - read_fdata) == 0))) then
        print *, "float_dataset: not all are equal"
        call exit(1)
    endif

    if(.not. (string == char_array)) then
        print *, "strings dataset: not equal"
        call exit(1)
    end if
    if(.not. (write_scalar == read_scalar)) then
        print *, "scalar dataset: not equal"
        call exit(1)
    end if
end program
