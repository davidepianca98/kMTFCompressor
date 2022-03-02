cmake_minimum_required(VERSION 3.16)
project(MTF)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/Core.cpp
        src/MTFHash.cpp
        src/MTFHashTable.cpp
        src/hash/DumbHash.cpp
        src/hash/MinimiserHash.cpp
        src/hash/RabinFingerprint.cpp
        src/hash/SimHash.cpp
        src/hash/VectorHash.cpp)

set(HEADER_FILES
        src/Core.h
        src/MTF.h
        src/MTFHash.h
        src/MTFHashTable.h
        src/RLE.h
        src/hash/DumbHash.h
        src/hash/Hash.h
        src/hash/MinimiserHash.h
        src/hash/RabinFingerprint.h
        src/SPSCQueue.h
        src/hash/SimHash.h
        src/hash/VectorHash.h)

enable_testing()

add_library(MTF_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
include_directories(src)
include_directories(src/hash)


FIND_PACKAGE(Boost 1.71 COMPONENTS program_options REQUIRED)
TARGET_LINK_LIBRARIES(MTF_lib ${Boost_LIBRARIES})

target_link_directories(MTF_lib PUBLIC /usr/local/lib)
target_include_directories(MTF_lib PUBLIC /usr/local/include/fastpfor)
target_link_libraries(MTF_lib FastPFOR)
target_link_libraries(MTF_lib pthread)

add_subdirectory(test)

add_executable(MTF main.cpp)
target_link_libraries(MTF MTF_lib)

