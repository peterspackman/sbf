module basic_tests
use sbf
use iso_c_binding
implicit none
contains

logical function test_header()
    type(sbf_DataHeader) :: header
    test_header = (c_sizeof(header) == 128)
end function

end module
program basic
    use sbf
    use basic_tests
    implicit none
    logical :: success
    if(.not. test_header()) then
        call EXIT(1)
    end if
end program