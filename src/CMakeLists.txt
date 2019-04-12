SET(current_dir ${CMAKE_CURRENT_LIST_DIR})

SET(src "")
AUX_SOURCE_DIRECTORY(${current_dir} src)
add_executable(${CMAKE_PROJECT_NAME} ${src})
message(STATUS ${CMAKE_PROJECT_NAME} " added ${src}")

target_link_libraries(${CMAKE_PROJECT_NAME} ${stdgl_libraries})
