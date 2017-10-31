cmake_minimum_required(VERSION 2.8)
project(nvm)

SET(CMAKE_CXX_FLAGS "-O0 -g -fpermissive -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/spdk/include)


set(CMAKE_CXX_STANDARD 11)
find_package(Threads REQUIRED)

include(ExternalProject)

ExternalProject_Add(
        gtest
        URL https://github.com/google/googletest/archive/master.zip
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        # Disable install step
        INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )
include_directories("${source_dir}/googletest/include"
        "${source_dir}/googlemock/include")

# Used to generate lib when there are some cpp files.

#add_library(source src/main.cc)


# Generate main executable
add_executable(main src/main.cc)
target_link_libraries(main)
#	${spdk/lib/libspdk.a}
#	${spdk/lib/librte.a}
#	-L${CMAKE_SOURCE_DIR}/spdk/lib
#  -lspdk -lrte -pthread -lnuma -ldl -luuid -lrt)

#add_library(perf src/perf.cpp)
#add_executable(perf src/perf.cpp)
#target_link_libraries(perf
#		${spdk/lib/libspdk.a}
#		${spdk/lib/librte.a}
#		-L${CMAKE_SOURCE_DIR}/spdk/lib
#		-lspdk -lrte -pthread -lnuma -ldl -luuid -lrt)

#add_library(perf src/access_pattern_benchmark.cpp)
#add_executable(access_pattern_benchmark src/access_pattern_benchmark.cpp)
#target_link_libraries(access_pattern_benchmark
#		${spdk/lib/libspdk.a}
#		${spdk/lib/librte.a}
#		-L${CMAKE_SOURCE_DIR}/spdk/lib
#		-lspdk -lrte -pthread -lnuma -ldl -luuid -lrt)

#add_library(libblk src/blk/file_blk_accessor.cpp src/blk/void_blk_accessor.cpp src/blk/void_blk_accessor.h)

add_executable(blk_tree src/tree/blk_tree_main.cpp src/tree/blk_inner.h)
#target_link_libraries(blk_tree libblk)


set(TEST_FILES
		test/test.cc test/leaf_node_test.cc test/inner_node_test.cc test/b_plus_tree_test.cc test/blk_test.cc
		test/disk_based_b_plus_tree_test.cc)

add_executable(unittest ${TEST_FILES})
target_link_libraries(unittest libgtest)





