cmake_minimum_required(VERSION 2.8)

project(Zallocator)

aux_source_directory(src DIR_SRCS)

include_directories(./src)

set(LIBRARY_OUTPUT_PATH ./lib)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -fno-exceptions -fno-rtti")

add_library(Zallocator_static ${DIR_SRCS})
add_library(Zallocator SHARED ${DIR_SRCS})

target_link_libraries(Zallocator jemalloc pthread)

file(COPY  src/NVMalloc.h src/CLNVMAddrSpaceBuilder.h  
      DESTINATION  include)
