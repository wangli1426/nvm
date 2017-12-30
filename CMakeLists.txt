cmake_minimum_required(VERSION 2.8)
project(nvm)

SET(CMAKE_CXX_FLAGS "-O0 -g -std=c++11")
#SET(CMAKE_CXX_FLAGS "-O3 -g  -std=c++11")

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


set(BOOST_REQUESTED_VERSION "1.65.1")
string(REPLACE "." "_" BOOST_REQUESTED_VERSION_UNDERSCORE ${BOOST_REQUESTED_VERSION})
include(ExternalProject)
ExternalProject_Add(
		Boost
		URL https://downloads.sourceforge.net/project/boost/boost/${BOOST_REQUESTED_VERSION}/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.zip
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/boost
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ./bootstrap.sh --prefix=${CMAKE_CURRENT_BINARY_DIR}/boost
		BUILD_COMMAND ./b2 ${BOOST_MAYBE_STATIC} --prefix=${CMAKE_CURRENT_BINARY_DIR}/boost --with-system --with-thread install
		BUILD_IN_SOURCE true
		INSTALL_COMMAND ""
		INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost
)
ExternalProject_Get_Property(Boost source_dir binary_dir)
add_library(libboost IMPORTED STATIC GLOBAL)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/boost/include")



# Used to generate lib when there are some cpp files.

#add_library(source src/main.cc)


# Generate main executable
add_executable(main src/main.cc)
target_link_libraries(main -lpthread
		${CMAKE_CURRENT_BINARY_DIR}/boost/lib/libboost_system.a
		${CMAKE_CURRENT_BINARY_DIR}/boost/lib/libboost_thread.a
#	${spdk/lib/libspdk.a}
#	${spdk/lib/librte.a}
#	-L${CMAKE_SOURCE_DIR}/spdk/lib
#  -lspdk -lrte -pthread -lnuma -ldl -luuid -lrt
		)
add_dependencies(main Boost)

#
#add_executable(access_pattern_benchmark src/access_pattern_benchmark.cpp)
#target_link_libraries(access_pattern_benchmark
#		${spdk/lib/libspdk.a}
#		${spdk/lib/librte.a}
#		-L${CMAKE_SOURCE_DIR}/spdk/lib
#		-lspdk -lrte -pthread -lnuma -ldl -luuid -lrt)

#
#
#
#add_library(libnvmblk src/blk/file_blk_accessor.cpp)
#target_link_libraries(libnvmblk
#				${spdk/lib/libspdk.a}
#				${spdk/lib/librte.a}
#				-L${CMAKE_SOURCE_DIR}/spdk/lib
#				-lspdk -lrte -pthread -lnuma -ldl -luuid -lrt)
#
#add_executable(blk_tree src/tree/blk_tree_main.cpp)
#

add_executable(tree_benchmark src/b_tree_benchmark.cc)
target_link_libraries(tree_benchmark
		${CMAKE_CURRENT_BINARY_DIR}/boost/lib/libboost_system.a
		${CMAKE_CURRENT_BINARY_DIR}/boost/lib/libboost_thread.a
		-lpthread
#		${spdk/lib/libspdk.a}
#		${spdk/lib/librte.a}
#		-L${CMAKE_SOURCE_DIR}/spdk/lib
#		-lspdk -lrte -lpthread -lnuma -ldl -luuid -lrt
		)
add_dependencies(tree_benchmark Boost)


set(TEST_FILES
		test/test.cc test/tree/leaf_node_test.cc test/tree/inner_node_test.cc test/tree/b_plus_tree_test.cc test/blk_test.cc
		test/tree/disk_based_b_plus_tree_test.cc test/lru_cache_test.cc
		test/utils/rwlock_test.cc
		test/sync/lock_manager_test.cc
		test/tree/concurrent_tree_test.cc
		test/tree/concurrent_disk_optimized_tree_test.cc)

add_executable(unittest ${TEST_FILES})
target_link_libraries(unittest
		${CMAKE_CURRENT_BINARY_DIR}/boost/lib/libboost_system.a
		${CMAKE_CURRENT_BINARY_DIR}/boost/lib/libboost_thread.a
		-lpthread
		libgtest
)
add_dependencies(unittest Boost)





