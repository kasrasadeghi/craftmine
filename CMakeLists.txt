CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

project(minecraft)
message("project: " ${CMAKE_PROJECT_NAME})

# create stdgl_libraries and aux_sources
FILE(GLOB cmakes ${CMAKE_SOURCE_DIR}/cmake/*.cmake)
FOREACH(cmake ${cmakes})
	INCLUDE(${cmake})
ENDFOREACH(cmake)

# NOTE: 
#   stdgl_libraries ::= opengl related libraries are stored in 
#   lib_source_dirs ::= auxiliary source directories for the main project
MESSAGE(STATUS "stdgl: ${stdgl_libraries}")
MESSAGE(STATUS "lib sources: ${lib_source_dirs}")

ADD_SUBDIRECTORY(src)

# IF (EXISTS ${CMAKE_SOURCE_DIR}/sln/CMakeLists.txt)
# 	ADD_SUBDIRECTORY(sln)
# ENDIF()
