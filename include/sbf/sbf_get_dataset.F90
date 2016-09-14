#if DIMENSION == 0
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), intent(out) :: data
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(data))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    data = transfer(dset%data, mold=data)
    if(present(errflag)) errflag = error
end subroutine 
#elif DIMENSION == 1
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1)))
    data = transfer(dset%data, mold=data)
    if(present(errflag)) errflag = error
end subroutine 
#elif DIMENSION == 2
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:,:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1), dset%header%shape(2)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 3
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 4
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 5
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:,:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4), dset%header%shape(5)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 6
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:,:,:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4), dset%header%shape(5), dset%header%shape(6)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#elif DIMENSION == 7
subroutine ROUTINE_NAME(this, name, data, errflag)
    character(len=*), intent(in) :: name
    class(sbf_File), intent(in) :: this
    FORTRAN_KIND(DATA_KIND), dimension(:,:,:,:,:,:,:), allocatable, intent(out) :: data
    FORTRAN_KIND(DATA_KIND) :: example_value
    integer, intent(out), optional :: errflag
    type(sbf_Dataset) :: dset
    integer :: error = SBF_RESULT_SUCCESS, ind
    ind = index_of_dataset_by_name(this,name)
    if ((ind < 1) .or. (ind > this%n_datasets)) then
        error = SBF_RESULT_DATASET_NOT_FOUND
        if(present(errflag)) errflag = error
        return
    end if
    dset = this%datasets(ind)
    if (.not. sbf_dt_compatible(dset%header%data_type, c_sizeof(example_value))) then
        error = SBF_RESULT_INCOMPATIBLE_DATA_TYPES
        if(present(errflag)) errflag = error
        return
    endif
    allocate(data(dset%header%shape(1), dset%header%shape(2), dset%header%shape(3), &
                  dset%header%shape(4), dset%header%shape(5), dset%header%shape(6), &
                  dset%header%shape(7)))
    data = reshape(transfer(dset%data, mold=data), dset%header%shape(:DIMENSION))
end subroutine 
#endif
