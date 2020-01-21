if(WIN32)
  cmake_minimum_required(VERSION 3.4)
else()
  cmake_minimum_required(VERSION 3.1)
endif()

# Fail immediately if not using an out-of-source build
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported.  Please create a build directory "
    "separate from the source directory")
endif()

#------------------------------------------------------------------------------#
# Parse version number from gecko.h
#------------------------------------------------------------------------------#
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/gecko.h _gecko_h_contents)
string(REGEX REPLACE ".*#define[ \t]+GECKO_VERSION_MAJOR[ \t]+([0-9]+).*"
     "\\1" GECKO_VERSION_MAJOR ${_gecko_h_contents})
string(REGEX REPLACE ".*#define[ \t]+GECKO_VERSION_MINOR[ \t]+([0-9]+).*"
    "\\1" GECKO_VERSION_MINOR ${_gecko_h_contents})
string(REGEX REPLACE ".*#define[ \t]+GECKO_VERSION_PATCH[ \t]+([0-9]+).*"
    "\\1" GECKO_VERSION_PATCH ${_gecko_h_contents})
set(GECKO_VERSION
  "${GECKO_VERSION_MAJOR}.${GECKO_VERSION_MINOR}.${GECKO_VERSION_PATCH}")

project(GECKO VERSION ${GECKO_VERSION})

#------------------------------------------------------------------------------#
# Some boilerplate to setup nice output directories
#------------------------------------------------------------------------------#
include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/gecko
  CACHE STRING "Installation CMake subdirectory")

list(INSERT CMAKE_MODULE_PATH 0 "${GECKO_SOURCE_DIR}/cmake")
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GECKO_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GECKO_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GECKO_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()

#------------------------------------------------------------------------------#
# Top level options
#------------------------------------------------------------------------------#

# Windows (Visual Studio) specific options
if(MSVC)
  # Use this to get a usable export library when building a DLL on Windows
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

  # Silence extraneous Visual Studio specific warnings
#  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS /wd4146 /wd4305)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

# Suggest C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(MSVC OR MINGW)
  set(CMAKE_C_STANDARD 90)
endif()

message(STATUS "Compiling with C standard: ${CMAKE_C_STANDARD}")

# Suggest C++98
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 98)
endif()
message(STATUS "Compiling with C++ standard: ${CMAKE_CXX_STANDARD}")

include(CMakeDependentOption)

# Typically you'd always be able to enable shared libraries but default
# configurations with the Cray toolchain will explicitly disable shared lib
# support and only allow static libs.  Making this a cmake_dependent_option
# will ensure that shared library support will be disabled if the system does
# not support it.

# Setup shared library / -fPIC stuff
get_property(SHARED_LIBS_SUPPORTED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
cmake_dependent_option(BUILD_SHARED_LIBS
  "Whether or not to build shared libraries" ON
  "SHARED_LIBS_SUPPORTED" OFF)

# PIC is always on for shared libs.  This allows it to be selectable for
# static libs.
if(DEFINED GECKO_ENABLE_PIC)
  set(GECKO_ENABLE_PIC_DEFAULT ${GECKO_ENABLE_PIC})
elseif(DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(GECKO_ENABLE_PIC_DEFAULT ${CMAKE_POSITION_INDEPENDENT_CODE})
else()
  set(GECKO_ENABLE_PIC_DEFAULT ${SHARED_LIBS_SUPPORTED})
endif()
cmake_dependent_option(GECKO_ENABLE_PIC
  "Build with Position Independent Code" ${GECKO_ENABLE_PIC_DEFAULT}
  "SHARED_LIBS_SUPPORTED" OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ${GECKO_ENABLE_PIC})

# Compile-time options

set(GECKO_PART_FRAC 4 CACHE STRING "Ratio of max to min weight for aggregation")
set_property(CACHE GECKO_PART_FRAC PROPERTY STRINGS "4")

set(GECKO_CR_SWEEPS 1 CACHE STRING "Number of compatible relaxation sweeps")
set_property(CACHE GECKO_CR_SWEEPS PROPERTY STRINGS "1")

set(GECKO_GS_SWEEPS 1 CACHE STRING "Number of Gauss-Seidel relaxation sweeps")
set_property(CACHE GECKO_GS_SWEEPS PROPERTY STRINGS "1")

set(GECKO_WINDOW_MAX 16 CACHE STRING "Max number of nodes in subgraph")
set_property(CACHE GECKO_WINDOW_MAX PROPERTY STRINGS "16")

option(GECKO_WITH_ADJLIST "Use adjacency list" OFF)

option(GECKO_WITH_NONRECURSIVE "Use nonrecursive permutation algorithm" OFF)

option(GECKO_WITH_DOUBLE_PRECISION "Use double-precision computations" OFF)

# Handle compile-time macros

list(APPEND gecko_private_defs GECKO_PART_FRAC=${GECKO_PART_FRAC})
list(APPEND gecko_private_defs GECKO_CR_SWEEPS=${GECKO_CR_SWEEPS})
list(APPEND gecko_private_defs GECKO_GS_SWEEPS=${GECKO_GS_SWEEPS})
list(APPEND gecko_private_defs GECKO_WINDOW_MAX=${GECKO_WINDOW_MAX})
list(APPEND gecko_private_defs GECKO_WIDTH_ADJLIST=${GECKO_WITH_ADJLIST})
list(APPEND gecko_private_defs GECKO_WIDTH_NONRECURSIVE=${GECKO_WITH_NONRECURSIVE})
list(APPEND gecko_private_defs GECKO_WIDTH_DOUBLE_PRECISION=${GECKO_WITH_DOUBLE_PRECISION})

# Link libm only if necessary
include(CheckCSourceCompiles)
check_c_source_compiles("#include<math.h>\nfloat f; int main(){sqrt(f);return 0;}" HAVE_MATH)
if(NOT HAVE_MATH)
  set(CMAKE_REQUIRED_LIBRARIES m)
  check_c_source_compiles("#include<math.h>\nfloat f; int main(){sqrt(f);return 0;}" HAVE_LIBM_MATH)
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(NOT HAVE_LIBM_MATH)
    message(FATAL_ERROR "Unable to use C math library functions (with or without -lm)")
  endif()
endif()

#------------------------------------------------------------------------------#
# Add source code
#------------------------------------------------------------------------------#
include(CTest)
if(BUILD_TESTING)
  enable_testing()
endif()

set(GECKO_LIBRARY_PREFIX "" CACHE STRING
  "Prefix to prepend to the output library name")
mark_as_advanced(GECKO_LIBRARY_PREFIX)

add_subdirectory(src)

option(BUILD_ALL "Build all subdirectories" OFF)
if(BUILD_ALL)
  set(BUILD_UTILITIES ON CACHE BOOL "Build command-line utilities for gecko" FORCE)
  set(BUILD_TESTING ON CACHE BOOL "Build tests" FORCE)
endif()

option(BUILD_UTILITIES "Build command-line utilities for gecko" ON)
if(BUILD_UTILITIES)
  add_subdirectory(utils)
endif()

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

#------------------------------------------------------------------------------#
# Header install
#------------------------------------------------------------------------------#
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#------------------------------------------------------------------------------#
# Build type: one of None, Debug, Release, RelWithDebInfo, MinSizeRel
#------------------------------------------------------------------------------#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
endif()
