program basic
    use sbf
    implicit none
    type(sbf_Dataset) :: dset
    type(sbf_File) :: data_file
    integer :: i
    integer(sbf_integer), dimension(1000) :: data = [(i, i=1,1000)]
    data_file%filename = "test_fortran.out"
    dset = sbf_Dataset("integer_dataset", data)
    call data_file%add_dataset(dset)
    call data_file%serialize
end program