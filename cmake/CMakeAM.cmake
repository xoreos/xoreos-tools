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

function(am_read_lines FILE_NAME OUTPUT_VARIABLE)
  message(STATUS "Parsing ${FILE_NAME}...")
  file(READ ${FILE_NAME} FILE_CONTENTS)

  string(REGEX REPLACE "\\$\\(([^ )]+)\\)"    "\${\\1}" FILE_CONTENTS "${FILE_CONTENTS}")
  string(REGEX REPLACE "\\$\\(shell [^)]+\\)" ""        FILE_CONTENTS "${FILE_CONTENTS}")
  string(REGEX REPLACE "\\\\\n" "  "                    FILE_CONTENTS "${FILE_CONTENTS}")
  string(REGEX REPLACE " +" " "                         FILE_CONTENTS "${FILE_CONTENTS}")
  string(REGEX REPLACE ";" "\\\\;"                      FILE_CONTENTS "${FILE_CONTENTS}")
  string(REGEX REPLACE "\n" ";"                         FILE_CONTENTS "${FILE_CONTENTS}")

  set(${OUTPUT_VARIABLE} ${FILE_CONTENTS} PARENT_SCOPE)
endfunction()


function(am_target_name AM_FOLDER AM_FILE_NAME AM_OUTPUT)
  get_filename_component(AM_FULLPATH  "${AM_FOLDER}/${AM_FILE_NAME}" ABSOLUTE)
  get_filename_component(AM_FOLDER    "${AM_FULLPATH}" PATH)
  get_filename_component(AM_FOLDER    "${AM_FOLDER}"   NAME)
  get_filename_component(AM_FILE_NAME "${AM_FULLPATH}" NAME_WE)
  string(REGEX REPLACE "^lib" "" AM_FILE_NAME "${AM_FILE_NAME}")

  string(REGEX REPLACE "${PROJECT_SOURCE_DIR}/(src/)?" "" AM_TARGET_NAME "${AM_FULLPATH}")

  if("${AM_FOLDER}" STREQUAL "${AM_FILE_NAME}")
    string(REGEX REPLACE "/[^/]+$" "" AM_TARGET_NAME "${AM_TARGET_NAME}")
  endif()

  string(REPLACE "/" "_" AM_TARGET_NAME "${AM_TARGET_NAME}")
  string(REGEX REPLACE "^src_" "" AM_TARGET_NAME "${AM_TARGET_NAME}")
  set(${AM_OUTPUT} ${AM_TARGET_NAME} PARENT_SCOPE)
endfunction()


function(am_add_target AM_TYPE AM_FOLDER AM_FILE AM_SOURCE_FILES AM_LINK_FILES)
  am_target_name(${AM_FOLDER} ${AM_FILE} AM_TARGET)

  set(AM_SOURCES "")
  foreach(AM_SOURCE_FILE ${AM_SOURCE_FILES})
    if(NOT AM_SOURCE_FILE STREQUAL "$(EMPTY)")
      list(APPEND AM_SOURCES ${AM_SOURCE_FILE})
    endif()
  endforeach()

  if(AM_TYPE STREQUAL "lib")
    add_library(${AM_TARGET} STATIC ${AM_SOURCES})
  else()
    add_executable(${AM_TARGET} ${AM_SOURCES})
  endif()

  get_target_property(AM_INCLUDE_DIRS ${AM_TARGET} INCLUDE_DIRECTORIES)
  list(APPEND AM_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/${AM_FOLDER})
  set_target_properties(${AM_TARGET} PROPERTIES INCLUDE_DIRECTORIES "${AM_INCLUDE_DIRS}")

  set(AM_LINK_TARGETS "")
  foreach(AM_LINK_FILE ${AM_LINK_FILES})
    if(NOT "${AM_LINK_FILE}" MATCHES "/lib(glew|lua).la$")
      if(NOT AM_LINK_FILE STREQUAL "$(EMPTY)" AND NOT AM_LINK_FILE STREQUAL "$(LDADD)")
        am_target_name(${AM_FOLDER} ${AM_LINK_FILE} AM_LINK_TARGET)
        list(APPEND AM_LINK_TARGETS ${AM_LINK_TARGET} ${${AM_LINK_TARGET}_LINK_TARGETS})
      endif()
    endif()
  endforeach()

  target_link_libraries(${AM_TARGET} ${AM_LINK_TARGETS})
  set(${AM_TARGET}_LINK_TARGETS ${AM_LINK_TARGETS} PARENT_SCOPE)
endfunction()

# Internal Automake -> CMake converter, doing the heavy lifting
function(convert_automake_internal AM_INPUT_FILE CMAKE_OUTPUT_FILE)
  am_read_lines(${AM_INPUT_FILE} AM_VARIABLE)
  foreach(AM_FILE_LINE ${AM_VARIABLE})

    # Recursively handle include directives
    if("${AM_FILE_LINE}" MATCHES "^include ")
      string(REGEX REPLACE "^include +" "" AM_INCLUDE_FILE "${AM_FILE_LINE}")
      file(APPEND "${CMAKE_OUTPUT_FILE}" "# Including ${AM_INCLUDE_FILE}\n")
      convert_automake_internal(${AM_INCLUDE_FILE} ${CMAKE_OUTPUT_FILE})
      continue()
    endif()

    string(REGEX REPLACE " *\\+= *" "+=" AM_FILE_LINE "${AM_FILE_LINE}")
    string(REGEX REPLACE " *= *"   "="   AM_FILE_LINE "${AM_FILE_LINE}")
    string(REGEX REPLACE "^ +"             ""   AM_FILE_LINE "${AM_FILE_LINE}")

    string(REGEX REPLACE "^(.*)\\+=(.*)$" "list(APPEND \\1 \\2)" AM_FILE_LINE "${AM_FILE_LINE}")
    string(REGEX REPLACE "^(.*)=(.*)$"    "set(\\1 \\2)" AM_FILE_LINE "${AM_FILE_LINE}")

    string(REGEX REPLACE "^if (.*)$"      "if(\\1)" AM_FILE_LINE "${AM_FILE_LINE}")
    string(REGEX REPLACE "^endif$"        "endif()" AM_FILE_LINE "${AM_FILE_LINE}")

    set(AM_FILE_LINE "#${AM_FILE_LINE}")

    string(REPLACE "#set("   "set("   AM_FILE_LINE "${AM_FILE_LINE}")
    string(REPLACE "#list("  "list("  AM_FILE_LINE "${AM_FILE_LINE}")
    string(REPLACE "#if("    "if("    AM_FILE_LINE "${AM_FILE_LINE}")
    string(REPLACE "#endif(" "endif(" AM_FILE_LINE "${AM_FILE_LINE}")

    # Comment out the gitstamp Automake rule hackery
    string(REGEX REPLACE "(.*\\$\\([ \t]*eval)" "#\\1" AM_FILE_LINE "${AM_FILE_LINE}")

    file(APPEND "${CMAKE_OUTPUT_FILE}" "${AM_FILE_LINE}\n")
  endforeach()
endfunction()

# Create the output file, and start the conversion
function(convert_automake AM_INPUT_FILE CMAKE_OUTPUT_FILE)
  file(WRITE "${CMAKE_OUTPUT_FILE}" "# Autogenerated file, do not edit!\n")
  convert_automake_internal(${AM_INPUT_FILE} ${CMAKE_OUTPUT_FILE})
endfunction()

function(parse_automake AM_FILE_NAME)
  get_filename_component(AM_FOLDER ${AM_FILE_NAME} PATH)
  if(NOT EXISTS ${AM_FOLDER})
    file(MAKE_DIRECTORY ${AM_FOLDER})
  endif()

  # Convert this Automake file into a CMake file and include it
  set(CMAKE_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${AM_FILE_NAME}.cmake")
  convert_automake(${AM_FILE_NAME} ${CMAKE_OUTPUT_FILE})

  include(${CMAKE_OUTPUT_FILE})

  # Search for convenience libraries, creating CMake targets
  set(AM_TARGETS)
  foreach(AM_FILE ${noinst_LTLIBRARIES})
    string(REPLACE "." "_" AM_NAME "${AM_FILE}")
    string(REPLACE "/" "_" AM_NAME "${AM_NAME}")
    am_add_target(lib ${AM_FOLDER} ${AM_FILE} "${${AM_NAME}_SOURCES}" "${${AM_NAME}_LIBADD}")

    am_target_name(${AM_FOLDER} ${AM_FILE} AM_TARGET)
    set(${AM_TARGET}_LINK_TARGETS ${${AM_TARGET}_LINK_TARGETS} PARENT_SCOPE)
    list(APPEND AM_TARGETS ${AM_TARGET})
  endforeach()

  # Search for programs, creating CMake targets
  foreach(AM_FILE ${bin_PROGRAMS})
    string(REPLACE "." "_" AM_NAME "${AM_FILE}")
    string(REPLACE "/" "_" AM_NAME "${AM_NAME}")
    am_add_target(bin ${AM_FOLDER} ${AM_FILE} "${${AM_NAME}_SOURCES}" "${${AM_NAME}_LDADD}")

    am_target_name(${AM_FOLDER} ${AM_FILE} AM_TARGET)
    set(${AM_TARGET}_LINK_TARGETS ${${AM_TARGET}_LINK_TARGETS} PARENT_SCOPE)
    list(APPEND AM_TARGETS ${AM_TARGET})
  endforeach()

  set(AM_TARGETS ${AM_TARGETS} PARENT_SCOPE)
endfunction()
