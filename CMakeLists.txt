# Optical properties of dense plasma
#
# Nenad Sakan
# 20250326
########################################

cmake_minimum_required(VERSION 3.14)
project(UPF_routines VERSION 1.0.0)

# Include FetchContent
include(FetchContent)

# wxWidgets
set(wxWidgets_USE_STATIC 1)
set(wxWidgets_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets)

# First try to find a local installation
find_package(wxWidgets COMPONENTS core base aui QUIET)

if(NOT wxWidgets_FOUND)
    message(STATUS "wxWidgets not found locally, fetching from git...")
    FetchContent_Declare(
        wxwidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG v3.2.2
        GIT_SHALLOW TRUE
    )
    
    # Configure wxWidgets build options
    set(wxBUILD_SHARED OFF CACHE BOOL "Build wxWidgets as static libraries" FORCE)
    set(wxBUILD_PRECOMP OFF CACHE BOOL "Disable precompiled headers" FORCE)
    set(wxBUILD_SAMPLES OFF CACHE BOOL "Do not build wxWidgets samples" FORCE)
    set(wxBUILD_DEMOS OFF CACHE BOOL "Do not build wxWidgets demos" FORCE)
    set(wxBUILD_TESTS OFF CACHE BOOL "Do not build wxWidgets tests" FORCE)
    set(wxBUILD_EXAMPLES OFF CACHE BOOL "Do not build wxWidgets examples" FORCE)
    set(wxBUILD_COMPATIBILITY_3_0 ON CACHE BOOL "Enable wxWidgets 3.0 compatibility" FORCE)
    
    # Make wxWidgets available
    FetchContent_MakeAvailable(wxwidgets)
    
    # Add include directories
    include_directories(${wxwidgets_SOURCE_DIR}/include)
else()
    message(STATUS "Found wxWidgets: ${wxWidgets_VERSION_STRING}")
    include(${wxWidgets_USE_FILE})
endif()

# Using our own mathplot implementation

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GSL
find_package(GSL REQUIRED)
if(GSL_FOUND)
    message(STATUS "Found GSL: ${GSL_VERSION}")
else()
    message(FATAL_ERROR "Could not find GSL library")
endif()

include(FetchContent)

# Add pugixml source files
set(PUGIXML_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml)

# Add source files
set(SOURCE_FILES 
    src/main/main.cpp
    src/main/main.hpp
    src/globals/globals.hpp
    src/globals/globals.cpp
    src/UPF_reader/UPF_reader.cpp
    src/UPF_reader/UPF_reader.hpp
    src/output/gnuplot_exporter.cpp
    src/output/gnuplot_exporter.hpp
    external/pugixml/pugixml.cpp
    )

# Add include directories
include_directories(
    src
    src/main
    src/globals
    src/UPF_reader
    src/output
    ${pugixml_SOURCE_DIR}/src
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
if(wxWidgets_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        wx::core
        wx::base
        wx::aui
    )
endif()

# Link GSL
target_link_libraries(${PROJECT_NAME} PRIVATE GSL::gsl GSL::gslcblas)

