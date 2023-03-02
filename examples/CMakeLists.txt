qc_setup_target(
    qcon-example-dom
    EXECUTABLE
    SOURCE_FILES
        example-dom.cpp
    PRIVATE_LINKS
        qcon)

qc_setup_target(
    qcon-example-encode-options
    EXECUTABLE
    SOURCE_FILES
        example-encode-options.cpp
    PRIVATE_LINKS
        qcon)

qc_setup_target(
    qcon-example-sax
    EXECUTABLE
    SOURCE_FILES
        example-sax.cpp
    PRIVATE_LINKS
        qcon)
