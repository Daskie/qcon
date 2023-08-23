cmake_minimum_required(VERSION 3.24)

project(qcon)

# Fetch dependencies
if(${PROJECT_IS_TOP_LEVEL})
    include(FetchContent)

    FetchContent_Declare(qc-cmake SOURCE_DIR ${CMAKE_SOURCE_DIR}/../qc-cmake)

    option(BUILD_GMOCK "" OFF)
    option(INSTALL_GTEST "" OFF)
    option(gtest_force_shared_crt "" ON)
    FetchContent_Declare(GTest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0 OVERRIDE_FIND_PACKAGE)

    FetchContent_MakeAvailable(GTest qc-cmake)
endif()

set(QC_CXX_STANDARD 20)

qc_setup_target(qcon INTERFACE_LIBRARY)

if(${PROJECT_IS_TOP_LEVEL})
    add_subdirectory(test EXCLUDE_FROM_ALL)
    add_subdirectory(examples EXCLUDE_FROM_ALL)
endif()
