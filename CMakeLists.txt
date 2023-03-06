cmake_minimum_required(VERSION 3.23)

project(qcon)

# Retrieve common CMake stuff
include(FetchContent)
FetchContent_Declare(qc-cmake GIT_REPOSITORY https://github.com/Daskie/qc-cmake.git)
FetchContent_MakeAvailable(qc-cmake)

set(QC_CXX_STANDARD 20)

qc_setup_target(qcon INTERFACE_LIBRARY)

add_subdirectory(test)

add_subdirectory(examples)

qc_setup_install(TARGETS qcon)
