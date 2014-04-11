CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
if(APPLE)
endif(APPLE)

IF(APPLE)
        option(USE_CLANG "Build with clang" ON) # ON is the default

        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
        SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" )
        SET( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++" )
        SET( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++" )

        SET( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" )
        SET( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )

ENDIF(APPLE)

PROJECT(safe-append-cpp)

SET(EXECUTABLE_NAME satest)
SET(LIBRARY_SHA_NAME  safeappend)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/include/boost")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/local/lib")

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

FILE(GLOB inc_files
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

FILE(GLOB src_files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

add_library(
    ${LIBRARY_SHA_NAME}
    STATIC
    ${src_files}
    ${inc_files}
)

include_directories(include)

SET(safe_append_src
main.cpp
)

ADD_EXECUTABLE(${EXECUTABLE_NAME} ${safe_append_src})

target_link_libraries(
    ${EXECUTABLE_NAME}
    ${LIBRARY_SHA_NAME}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)



