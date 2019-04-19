## build sources
SET(src "")

# check for gtest
find_package(GTest REQUIRED)
enable_testing()

# add current directory to sources
aux_source_directory(${CMAKE_SOURCE_DIR}/src src)

# remove main and replace with test suite
list(REMOVE_ITEM src "${CMAKE_SOURCE_DIR}/src/main.cpp")
list(APPEND src "${CMAKE_SOURCE_DIR}/test/test.cpp")

# add library src_directories to sources
foreach(x IN LISTS lib_source_dirs)
  aux_source_directory(${x} src)
endforeach(x IN lib_source_dirs)

# build project with sources and standard gl libraries
message(STATUS test " added ${src}")
add_executable(test ${src})
target_link_libraries(test PUBLIC ${stdgl_libraries})

# link with gtest
target_link_libraries(test PUBLIC
  gtest
  gtest_main
)