cmake_minimum_required(VERSION 3.16)
project(MTF CXX)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        src/encoders/AdaptiveEliasGamma.cpp
        src/encoders/AdaptiveHuffman.cpp
        src/hash/randomized/LinearHash.cpp
        src/hash/randomized/MinimiserHash.cpp
        src/hash/randomized/RabinKarp.cpp
        src/hash/randomized/TabulationHash.cpp
        src/hash/Fnv1a.cpp
        src/hash/Hash.cpp
        src/hash/Identity.cpp
        src/hash/VectorHash.cpp
        src/stream/ibitstream/ibitstream.cpp
        src/stream/ibitstream/ibufbitstream.cpp
        src/stream/ibitstream/ifbitstream.cpp
        src/stream/obitstream/obitstream.cpp
        src/stream/obitstream/obufbitstream.cpp
        src/stream/obitstream/ofbitstream.cpp
        )

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
        src/hash/Fnv1a.h
        src/hash/Hash.h
        src/hash/Identity.h
        src/hash/randomized/LinearHash.h
        src/hash/randomized/MinimiserHash.h
        src/hash/randomized/RabinKarp.h
        src/hash/randomized/TabulationHash.h
        src/hash/VectorHash.h
        src/stream/ibitstream/ibitstream.h
        src/stream/ibitstream/ibufbitstream.h
        src/stream/ibitstream/ifbitstream.h
        src/stream/obitstream/obitstream.h
        src/stream/obitstream/obufbitstream.h
        src/stream/obitstream/ofbitstream.h)

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

