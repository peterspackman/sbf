add_executable(sbftool sbftool.c)
target_include_directories(sbftool
    PUBLIC
    # Headers from build location
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    # Headers from installed location
    "$<INSTALL_INTERFACE:include>"
)
