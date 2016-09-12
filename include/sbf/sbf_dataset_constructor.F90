
function ROUTINE_NAME(name, data)
    character(len=*) :: name
    FORTRAN_KIND(DATA_KIND), dimension(DIMENSIONS) :: data
    type(sbf_Dataset) :: ROUTINE_NAME
    integer :: length, name_length, i
    integer(sbf_byte) :: dims
    name_length = len(name)
    ! assign the character array from the given string
    do i=1, size(ROUTINE_NAME%header%name)
        if(i < name_length + 1) then
            ROUTINE_NAME%header%name(i) = name(i:i)
        else; ROUTINE_NAME%header%name(i) = char(0)
        end if
    end do
    ! how many bytes do we have
    length = size(transfer(data, ROUTINE_NAME%data))
    allocate(ROUTINE_NAME%data(length))
    ROUTINE_NAME%data = transfer(data, ROUTINE_NAME%data)
    ! set the data type in file
    ROUTINE_NAME%header%data_type = DATATYPE
    ! set the dimensions in file
    dims = size(shape(data))
    call sbf_dh_set_dims(ROUTINE_NAME%header, dims)
    ! set the shape in file
    ROUTINE_NAME%header%shape(:dims) = shape(data)
end function
