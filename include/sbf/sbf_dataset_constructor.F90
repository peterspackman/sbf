
function ROUTINE_NAME(name, data)
    character(len=*) :: name
    integer(DATATYPE), dimension(DIMENSIONS) :: data
    type(sbf_Dataset) :: ROUTINE_NAME
    integer :: length, i
    do i=1, len(name)
        ROUTINE_NAME%header%name(i) = name(i:i)
    end do
    length = size(transfer(data, ROUTINE_NAME%data))
    allocate(ROUTINE_NAME%data(length))
    ROUTINE_NAME%data = transfer(data, ROUTINE_NAME%data)
end function
