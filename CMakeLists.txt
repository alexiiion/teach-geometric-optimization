cmake_minimum_required(VERSION 3.1)
project(GeometricOptimization)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/")



########################################
######## configure DEPENDENCIES ########
########################################

# libigl settings
# make compilation faster using static linking
option(LIBIGL_USE_STATIC_LIBRARY 	 "Use libigl as static library" OFF)
option(LIBIGL_WITH_ANTTWEAKBAR       "Use AntTweakBar"    OFF)
option(LIBIGL_WITH_CGAL              "Use CGAL"           ON)
option(LIBIGL_WITH_COMISO            "Use CoMiso"         OFF)
option(LIBIGL_WITH_CORK              "Use Cork"           OFF)
option(LIBIGL_WITH_EMBREE            "Use Embree"         OFF)
option(LIBIGL_WITH_LIM               "Use LIM"            OFF)
option(LIBIGL_WITH_MATLAB            "Use Matlab"         OFF)
option(LIBIGL_WITH_MOSEK             "Use MOSEK"          OFF)
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)
option(LIBIGL_WITH_OPENGL_GLFW_IMGUI "Use ImGui"          ON)
option(LIBIGL_WITH_PNG               "Use PNG"            OFF)
option(LIBIGL_WITH_PYTHON            "Use Python"         OFF)
option(LIBIGL_WITH_TETGEN            "Use Tetgen"         OFF)
option(LIBIGL_WITH_TRIANGLE          "Use Triangle"       OFF)
option(LIBIGL_WITH_VIEWER            "Use OpenGL viewer"  ON)
option(LIBIGL_WITH_XML               "Use XML"            ON)

# Include libigl
set(LIBIGL_DIR "${DEPENDENCIES_DIR}/libigl/")
set(LIBIGL_INCLUDE_DIR "${LIBIGL_DIR}/include/")
list(APPEND CMAKE_MODULE_PATH "${LIBIGL_INCLUDE_DIR}/../cmake")

# add_subdirectory(${LIBIGL_DIR})
include(libigl)


# Include LBFGSpp
include_directories("${DEPENDENCIES_DIR}/LBFGSpp/include")

# Include mesh plane intersection library
#include_directories("${DEPENDENCIES_DIR}/mesh-plane-intersection/src")




###################################
######## configure PROJECT ########
###################################

# set c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wuninitialized -pedantic-errors -Wno-unused-parameter)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wc++20-extensions)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

#set (CMAKE_CXX_CLANG_TIDY clang-tidy)




###############################
######## build PROJECT ########
###############################

# include("./cmake/IncludeMacros.txt")


# Add source code such that it mirrors the structure on the file system in the project

set(SRC_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/")
include_directories(${SRC_ROOT_PATH})


# Add header files recursively
file(
    GLOB_RECURSE HEADER_FILES 
    LIST_DIRECTORIES false
    "${SRC_ROOT_PATH}/*.h*"
)

# Add source files recursively
file(
    GLOB_RECURSE SRC_FILES 
    LIST_DIRECTORIES false
    "${SRC_ROOT_PATH}/*.c*"
	"${SRC_ROOT_PATH}/*.h*"
)


# Group source files for IDE (Visual Studio) -- remove this if it causes problems for other IDEs
foreach(SRC_FILE IN ITEMS ${SRC_FILES})
    get_filename_component(SRC_FILE_PATH "${SRC_FILE}" PATH)
    file(RELATIVE_PATH SRC_PATH_RELATIVE "${SRC_ROOT_PATH}" "${SRC_FILE_PATH}")
    string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_RELATIVE}")
    source_group("${GROUP_PATH}" FILES "${SRC_FILE}")
endforeach()

if(WIN32)
	add_definitions (-D_USE_MATH_DEFINES)
	message("  use -D_USE_MATH_DEFINES on windows")
endif()


add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC igl::core igl::opengl igl::opengl_glfw igl::opengl_glfw_imgui igl::xml igl::cgal)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    link_libraries(stdc++fs)
endif()
