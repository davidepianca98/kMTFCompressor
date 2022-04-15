cmake_minimum_required(VERSION 3.16)
project(MTF CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -pedantic -flto -DNDEBUG")

set(SOURCE_FILES
        src/encoders/AdaptiveEliasGamma.cpp
        src/encoders/AdaptiveEliasGamma.h
        src/encoders/AdaptiveHuffman.cpp
        src/encoders/AdaptiveHuffman.h
        src/encoders/RunLength.cpp
        src/encoders/RunLength.h
        src/hash/randomized/LinearHash.cpp
        src/hash/randomized/LinearHash.h
        src/hash/randomized/MinimiserHash.cpp
        src/hash/randomized/MinimiserHash.h
        src/hash/randomized/RabinKarp.cpp
        src/hash/randomized/RabinKarp.h
        src/hash/randomized/TabulationHash.cpp
        src/hash/randomized/TabulationHash.h
        src/hash/Hash.cpp
        src/hash/Hash.h
        src/hash/Identity.cpp
        src/hash/Identity.h
        src/mtf/buffer/MTFBuffer.h
        src/mtf/buffer/MTFRankBuffer.h
        src/mtf/mtftable/MTFBlockWorker.h
        src/mtf/mtftable/MTFHashContext.h
        src/mtf/mtftable/MTFHashTable.h
        src/mtf/mtftable/MTFHashTableStream.h
        src/mtf/MTF.cpp
        src/mtf/MTF.h
        src/mtf/MTFHashCompressor.h
        src/stream/ibitstream/ibitstream.cpp
        src/stream/ibitstream/ibitstream.h
        src/stream/ibitstream/ibufbitstream.cpp
        src/stream/ibitstream/ibufbitstream.h
        src/stream/ibitstream/ifbitstream.cpp
        src/stream/ibitstream/ifbitstream.h
        src/stream/obitstream/obitstream.cpp
        src/stream/obitstream/obitstream.h
        src/stream/obitstream/obufbitstream.cpp
        src/stream/obitstream/obufbitstream.h
        src/stream/obitstream/ofbitstream.cpp
        src/stream/obitstream/ofbitstream.h
        )

enable_testing()

add_library(MTF_lib STATIC ${SOURCE_FILES})
include_directories(src)
include_directories(src/hash)


FIND_PACKAGE(Boost 1.71 COMPONENTS program_options REQUIRED)
target_link_libraries(MTF_lib ${Boost_LIBRARIES})

target_link_libraries(MTF_lib pthread)

add_subdirectory(test)

add_executable(MTF main.cpp)
target_link_libraries(MTF MTF_lib)

