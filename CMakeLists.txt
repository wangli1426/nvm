cmake_minimum_required(VERSION 2.8)
project(nvm)

SET(CMAKE_CXX_FLAGS "-O2 -fpermissive")

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
set(SOURCE_FILES
        src/main.cc)
add_library(source ${SOURCE_FILES})


# Generate b_tree executable
add_executable(nvm src/main.cc)
target_link_libraries(nvm source
	${spdk/lib/libspdk.a}
	${spdk/lib/librte.a}
	-L${CMAKE_SOURCE_DIR}/spdk/lib
  -lspdk -lrte -pthread -lnuma -ldl -luuid -lrt)

