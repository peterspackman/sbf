#if DIMENSION == 1
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1)))
    data = transfer(dset%data, mold=data)
end subroutine 
#elif DIMENSION == 2
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:,:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1), dset%header%shape(2)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 3
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 4
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 5
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:,:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4), dset%header%shape(5)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 6
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:,:,:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4), dset%header%shape(5), dset%header%shape(6)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 7
subroutine ROUTINE_NAME(this, name, data)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    type(sbf_Dataset) :: dset
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:,:,:,:), allocatable, intent(out) :: data
    ! should check the index exists
    dset = this%datasets(index_of_dataset_by_name(this,name))
    ! need error checking
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4), dset%header%shape(5), dset%header%shape(6), &
                  dset%header%shape(7)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#endif
