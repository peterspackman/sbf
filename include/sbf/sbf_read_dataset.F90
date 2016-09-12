subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(DIMENSIONS), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1)))
    data = transfer(dset%data, data)
end subroutine 