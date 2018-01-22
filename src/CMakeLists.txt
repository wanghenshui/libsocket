add_library(libxio "")

# environment
set_target_properties(
    libxio PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    VERSION ${libxio_VERSION}
    OUTPUT_NAME libxio PREFIX ""
)

target_compile_definitions(
    libxio PUBLIC
    XIO_VERSION_MAJOR=${libxio_VERSION_MAJOR}
    XIO_VERSION_MINOR=${libxio_VERSION_MINOR}
    XIO_VERSION_PATCH=${libxio_VERSION_PATCH}
    XIO_VERSION="${libxio_VERSION}"
)

# sublibraries
if(NOT TARGET libchen)  # create it locally if not exist
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/libchen)
        execute_process(
            COMMAND git clone -b master https://github.com/chensoft/libchen.git
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
        )
    endif()
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libchen lib/libchen)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/ifaddrs lib/ifaddrs)

# include path
target_include_directories(libxio PUBLIC ${PROJECT_SOURCE_DIR}/include)

# source codes
file(GLOB_RECURSE XIO_INC ${PROJECT_SOURCE_DIR}/include/*.hpp)  # I hate to list all the files
file(GLOB_RECURSE XIO_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

# static build
# use -DBUILD_SHARED_LIBS=ON to build a shared library
target_sources(libxio PRIVATE ${XIO_INC} ${XIO_SRC})

# link library
target_link_libraries(libxio libchen ifaddrs)

# install libs
include(GNUInstallDirs)

install(
    TARGETS libxio
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/xio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# todo test libchen
# code warnings
#if(UNIX)
#    target_compile_options(libxio PUBLIC -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)
#endif()

# todo libchen option use PUBLIC, remove link libs
# code coverage
#option(XIO_CODE_COVERAGE "Enable libxio code coverage." OFF)
#
#if(XIO_CODE_COVERAGE)
#    target_compile_options(libxio PRIVATE -g -O0 --coverage)  # debug, no optimization, enable coverage
#    target_link_libraries(libxio gcov)
#endif()