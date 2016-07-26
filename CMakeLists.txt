# xoreos - A reimplementation of BioWare's Aurora engine
#
# xoreos is the legal property of its developers, whose names
# can be found in the AUTHORS file distributed with this source
# distribution.
#
# xoreos is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# xoreos is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with xoreos. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8.12)

project(xoreos-tools C CXX)
set(xoreos-tools_VERSION 0.0.4)


# -------------------------------------------------------------------------
# load some cmake modules from cmake/ subfolder
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(GNUInstallDirs)
include(CheckIncludeFile)
include(CMakeAM)
include(SetCheckCompilerFlag)


# -------------------------------------------------------------------------
# platform specific flags
if(CMAKE_HOST_APPLE)
  add_definitions(-DUNIX -DMACOSX)

elseif(CMAKE_HOST_UNIX)
  add_definitions(-DUNIX)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections -fno-show-column")

  # Wrap libraries in --start-group and --end-group to easily support static linking and symbol resolution, maybe useful on APPLE also, but I don't know
  string(REPLACE "<LINK_LIBRARIES>" "-Wl,--gc-sections -Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")
  string(REPLACE "<LINK_LIBRARIES>" "-Wl,--gc-sections -Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group" CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")
  string(REPLACE "<LINK_LIBRARIES>" "-Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")
  string(REPLACE "<LINK_LIBRARIES>" "-Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group" CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY}")
  string(REPLACE "<LINK_LIBRARIES>" "-Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group" CMAKE_C_CREATE_SHARED_MODULE "${CMAKE_C_CREATE_SHARED_MODULE}")
  string(REPLACE "<LINK_LIBRARIES>" "-Wl,--start-group <LINK_LIBRARIES> -Wl,--end-group" CMAKE_CXX_CREATE_SHARED_MODULE "${CMAKE_CXX_CREATE_SHARED_MODULE}")
elseif(CMAKE_HOST_WIN32)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()
else()
  message(STATUS "Unknown platform, maybe not supported")
endif()

# C/C++ standards we're compiling against
set_check_compiler_flag_c("-std=c99")
set_check_compiler_flag_cxx("-std=c++03")

# Compiler warning flags
set_check_compiler_flag_c_cxx("-Wignored-qualifiers")
set_check_compiler_flag_c_cxx("-Wpointer-arith")
set_check_compiler_flag_c_cxx("-Wshadow")
set_check_compiler_flag_c_cxx("-Wsign-compare")
set_check_compiler_flag_c_cxx("-Wtype-limits")
set_check_compiler_flag_c_cxx("-Wuninitialized")
set_check_compiler_flag_c_cxx("-Wunused-parameter")
set_check_compiler_flag_c_cxx("-Wunused-but-set-parameter")
set_check_compiler_flag_c_cxx("-Wlong-long" "-Wno-long-long")

set_check_compiler_flag_cxx("-Wnon-virtual-dtor")
set_check_compiler_flag_cxx("-Woverloaded-virtual" "-Wno-overloaded-virtual")
set_check_compiler_flag_cxx("-Wc++11-long-long" "-Wno-c++11-long-long")

# platform specific warning settings
if(CMAKE_HOST_WIN32)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

    set(WARNINGS_DISABLE
      # ignore
      4250 # 'class1' : inherits 'class2::member' via dominance
      #4101 # 'identifier' : unreferenced local variable
      4100 # 'identifier' : unreferenced formal parameter
      4127 # conditional expression is constant (template parameter already evaluated)
      4189 # 'identifier' : local variable is initialized but not referenced
      4245 # 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
      4435 # 'class1' : Object layout under /vd2 will change due to virtual base 'class2'
      4510 # 'class' : default constructor could not be generated
      4512 # 'class' : assignment operator could not be generated
      4610 # object 'class' can never be instantiated - user-defined constructor required
      #4702 # unreachable code
      4706 # assignment within conditional expression
      4710 # 'function' : function not inlined
      4714 # function 'function' marked as __forceinline not inlined

      # investigate later
      4305 # 'identifier' : truncation from 'type1' to 'type2'
      4244 # 'argument' : conversion from 'type1' to 'type2', possible loss of data
      4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
      4996 # 'function': was declared deprecated

      # investigate now
      #4005 # 'identifier' : macro redefinition
      #4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
      #4146 # unary minus operator applied to unsigned type, result still unsigned
      )

    foreach(d ${WARNINGS_DISABLE})
      set(WARNINGS "${WARNINGS} /wd${d}")
    endforeach(d)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${MT_BUILD}")
  endif()
endif()


# header detection, could get rid of it, but it is used by lua as well
include(CheckIncludeFile)
function(check_has_header FILE_NAME PP_NAME)
  check_include_file(${FILE_NAME} ${PP_NAME})
  if(${PP_NAME})
    add_definitions(-D${PP_NAME}=1)
  endif()
endfunction()

check_has_header("stdint.h"    HAVE_STDINT_H)
check_has_header("inttypes.h"  HAVE_INTTYPES_H)
check_has_header("sys/types.h" HAVE_SYS_TYPES_H)

# type size checks
include(CheckTypeSize)
check_type_size("char"      SIZEOF_CHAR)
check_type_size("short"     SIZEOF_SHORT)
check_type_size("int"       SIZEOF_INT)
check_type_size("long"      SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)
check_type_size("void *"    SIZEOF_VOID_P)
check_type_size("intptr_t"  SIZEOF_INTPTR_T)
check_type_size("uintptr_t" SIZEOF_UINTPTR_T)

if(HAVE_SIZEOF_CHAR AND SIZEOF_CHAR)
  add_definitions(-DSIZEOF_CHAR=${SIZEOF_CHAR})
endif()
if(HAVE_SIZEOF_SHORT AND SIZEOF_SHORT)
  add_definitions(-DSIZEOF_SHORT=${SIZEOF_SHORT})
endif()
if(HAVE_SIZEOF_INT AND SIZEOF_INT)
  add_definitions(-DSIZEOF_INT=${SIZEOF_INT})
endif()
if(HAVE_SIZEOF_LONG AND SIZEOF_LONG)
  add_definitions(-DSIZEOF_LONG=${SIZEOF_LONG})
endif()
if(HAVE_SIZEOF_LONG_LONG AND SIZEOF_LONG_LONG)
  add_definitions(-DSIZEOF_LONG_LONG=${SIZEOF_LONG_LONG})
endif()
if(HAVE_SIZEOF_VOID_P AND SIZEOF_VOID_P)
  add_definitions(-DSIZEOF_VOID_P=${SIZEOF_VOID_P})
endif()

if(HAVE_SIZEOF_INTPTR_T AND SIZEOF_INTPTR_T)
  add_definitions(-DHAVE_INTPTR_T=1)
endif()
if(HAVE_SIZEOF_UINTPTR_T AND SIZEOF_UINTPTR_T)
  add_definitions(-DHAVE_UINTPTR_T=1)
endif()

# function detection
include(CheckCXXSymbolExists)
function(check_has_function FUNCTION_NAME FILE_NAME PP_NAME)
  check_cxx_symbol_exists(${FUNCTION_NAME} "${FILE_NAME}" ${PP_NAME})
  if(${PP_NAME})
    add_definitions(-D${PP_NAME}=1)
  endif()
endfunction()

check_has_function(strtof   "cstdlib" HAVE_STRTOF)
check_has_function(strtoll  "cstdlib" HAVE_STRTOLL)
check_has_function(strtoull "cstdlib" HAVE_STRTOULL)


# endianess detection, could be replaced by including Boost.Config
include(TestBigEndian)
test_big_endian(XOREOS_BIG_ENDIAN)
if(XOREOS_BIG_ENDIAN)
  add_definitions(-DXOREOS_BIG_ENDIAN=1)
else()
  add_definitions(-DXOREOS_LITTLE_ENDIAN=1)
endif()


# -------------------------------------------------------------------------
# subfolders where built binaries and libraries will be located, relative to the build folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# -------------------------------------------------------------------------
# find the required libraries
set(XOREOSTOOLS_LIBRARIES "")

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
list(APPEND XOREOSTOOLS_LIBRARIES ${ZLIB_LIBRARIES})

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
list(APPEND XOREOSTOOLS_LIBRARIES ${LIBXML2_LIBRARIES})

find_package(Iconv REQUIRED)
include_directories(${ICONV_INCLUDE_DIRS})
list(APPEND XOREOSTOOLS_LIBRARIES ${ICONV_LIBRARIES})

if(ICONV_SECOND_ARGUMENT_IS_CONST)
  add_definitions(-DICONV_CONST=const)
else(ICONV_SECOND_ARGUMENT_IS_CONST)
  add_definitions(-DICONV_CONST=)
endif(ICONV_SECOND_ARGUMENT_IS_CONST)


# -------------------------------------------------------------------------
# xoreos-tools main targets, parsed from configure.ac and */Makefile.am
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src)
add_definitions(-DPACKAGE_STRING="xoreos-tools ${xoreos-tools_VERSION}")
parse_configure(configure.ac src)
target_link_libraries(gff2xml ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(tlk2xml ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(ssf2xml ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(xml2tlk ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(xml2ssf ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(convert2da ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(fixpremiumgff ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(unerf ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(unherf ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(unrim ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(unkeybif ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(unnds ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(unnsbtx ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(desmall ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(xoreostex2tga ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(nbfs2tga ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(ncgr2tga ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(cbgt2tga ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(cdpth2tga ${XOREOSTOOLS_LIBRARIES})
target_link_libraries(ncsdis ${XOREOSTOOLS_LIBRARIES})


# -------------------------------------------------------------------------
# try to add version information from git to src/common/version.cpp
# this is not 100% clean, and doesn't reconfigure when there's only a local change since last
#  build (so it does not always add the .dirty suffix)
string(TIMESTAMP XOREOS_BUILDDATE "%Y-%m-%dT%H:%M:%SZ" UTC)
set_property(SOURCE src/common/version.cpp APPEND PROPERTY COMPILE_DEFINITIONS XOREOS_BUILDDATE="${XOREOS_BUILDDATE}")

find_package(Git)
if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --long --match desc/* OUTPUT_VARIABLE XOREOS_REVDESC OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "desc/(.*)-([^-]*)-([^-]*)" "\\1+\\2.\\3" XOREOS_REVDESC "${XOREOS_REVDESC}")

  execute_process(
    COMMAND ${GIT_EXECUTABLE} update-index --refresh --unmerged
    COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD
    OUTPUT_QUIET ERROR_QUIET
    RESULT_VARIABLE XOREOS_REVDIRT
  )
  if(NOT XOREOS_REVDIRT EQUAL 0)
    set(XOREOS_REVDESC "${XOREOS_REVDESC}.dirty")
  endif()

  string(REGEX REPLACE "^[^+]*\\+" "" XOREOS_REV "${XOREOS_REVDESC}")
  message(STATUS "Building version: ${XOREOS_REVDESC}")

  set_property(SOURCE src/common/version.cpp APPEND PROPERTY COMPILE_DEFINITIONS XOREOS_REVDESC="${XOREOS_REVDESC}")
  set_property(SOURCE src/common/version.cpp APPEND PROPERTY COMPILE_DEFINITIONS XOREOS_REV="${XOREOS_REV}")

  # use .git/logs/HEAD to track if git state has changed and if we need to reconfigure
  if(EXISTS ${CMAKE_SOURCE_DIR}/.git/logs/HEAD)
    configure_file(${CMAKE_SOURCE_DIR}/.git/logs/HEAD .dirstamp)
  endif()
endif()
