cmake_minimum_required(VERSION 3.16)
project(MTF)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/MTF.cpp
        src/MTFHash.cpp
        src/MTFHashTable.cpp
        src/RLE.cpp
        src/hash/DumbHash.cpp
        src/hash/MinimiserHash.cpp
        src/hash/RabinFingerprint.cpp
        src/hash/VectorHash.cpp)

set(HEADER_FILES
        src/MTF.h
        src/MTFHash.h
        src/MTFHashTable.h
        src/RLE.h
        src/hash/DumbHash.h
        src/hash/Hash.h
        src/hash/MinimiserHash.h
        src/hash/RabinFingerprint.h
        src/hash/VectorHash.h)

enable_testing()

add_library(MTF_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
include_directories(src)
include_directories(src/hash)

target_link_directories(MTF_lib PUBLIC /usr/local/lib)
target_include_directories(MTF_lib PUBLIC /usr/local/include/fastpfor)
target_link_libraries(MTF_lib FastPFOR)

add_subdirectory(test)

add_executable(MTF main.cpp)
target_link_libraries(MTF MTF_lib)

