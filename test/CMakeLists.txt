find_package(GTest CONFIG REQUIRED)

qc_setup_target(
    qcon-test-dom
    EXECUTABLE
    SOURCE_FILES
        test-dom.cpp
    PRIVATE_LINKS
        qcon
        GTest::gtest_main)

qc_setup_target(
    qcon-test-decode
    EXECUTABLE
    SOURCE_FILES
        test-decode.cpp
    PRIVATE_LINKS
        qcon
        GTest::gtest_main)

qc_setup_target(
    qcon-test-encode
    EXECUTABLE
    SOURCE_FILES
        test-encode.cpp
    PRIVATE_LINKS
        qcon
        GTest::gtest_main)
