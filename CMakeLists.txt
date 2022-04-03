cmake_minimum_required(VERSION 3.16)
project(MTF)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")

set(SOURCE_FILES
        src/MTFHashTable.cpp
        src/MTFHashTableBlock.cpp
        src/MTFHashTableStream.cpp)

set(HEADER_FILES
        src/MTFBuffer.h
        src/MTFBlockWorker.h
        src/MTF.h
        src/MTFHashCompressor.h
        src/MTFHashTable.h
        src/MTFHashTableBlock.h
        src/MTFHashTableStream.h
        src/MTFRankBuffer.h
        src/encoders/AdaptiveEliasGamma.h
        src/encoders/AdaptiveHuffman.h
        src/hash/Adler32.h
        src/hash/CRC.h
        src/hash/Fnv1a.h
        src/hash/Hash.h
        src/hash/Identity.h
        src/hash/JumpConsistent.h
        src/hash/LinearHash.h
        src/hash/RabinKarp.h
        src/hash/TabulationHash.h
        src/hash/VectorHash.h
        src/stream/ibitstream.h
        src/stream/ibufbitstream.h
        src/stream/ifbitstream.h
        src/stream/obitstream.h
        src/stream/obufbitstream.h
        src/stream/ofbitstream.h)

enable_testing()

add_library(MTF_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})
include_directories(src)
include_directories(src/hash)


FIND_PACKAGE(Boost 1.71 COMPONENTS program_options REQUIRED)
target_link_libraries(MTF_lib ${Boost_LIBRARIES})

target_link_libraries(MTF_lib pthread)

add_subdirectory(test)

add_executable(MTF main.cpp)
target_link_libraries(MTF MTF_lib)

