find_package(GTest CONFIG REQUIRED)

qc_setup_target(
    qcon-test
    EXECUTABLE
    SOURCE_FILES
        test.cpp
    PRIVATE_LINKS
        qcon
        GTest::gtest_main)

qc_setup_target(
    qcon-decode-test
    EXECUTABLE
    SOURCE_FILES
        test-decode.cpp
    PRIVATE_LINKS
        qcon
        GTest::gtest_main)

qc_setup_target(
    qcon-encode-test
    EXECUTABLE
    SOURCE_FILES
        test-encode.cpp
    PRIVATE_LINKS
        qcon
        GTest::gtest_main)
