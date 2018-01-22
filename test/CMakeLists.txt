add_executable(libxio-test "")

# environment
set_target_properties(
    libxio-test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

# include path
target_include_directories(libxio-test PRIVATE src)

# source codes
file(GLOB_RECURSE CHEN_TEST_INC src/*.hpp)
file(GLOB_RECURSE CHEN_TEST_SRC src/*.cpp)

# generate app
target_sources(libxio-test PRIVATE ${CHEN_TEST_INC} ${CHEN_TEST_SRC})

# link library
target_link_libraries(libxio-test libxio)

# make test
add_test(libxio-test libxio-test)