###############################################################################
#   
#   Pvtbox. Fast and secure file transfer & sync directly across your devices. 
#   Copyright © 2020  Pb Private Cloud Solutions Ltd. 
#   
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#   
###############################################################################
cmake_minimum_required(VERSION 3.0.2)
include(ExternalProject)
include(GNUInstallDirs)

set(LIB_DIRS)

function (link_module MODULE)
    pkg_search_module("${MODULE}_ALIAS" REQUIRED ${MODULE})
    link_directories("${${MODULE}_ALIAS_LIBRARY_DIRS}")
endfunction(link_module)


function (target_link_module TARGET MODULE)
    message("added include dir: ${${MODULE}_ALIAS_INCLUDE_DIRS}")
    target_include_directories(${TARGET} PUBLIC ${${MODULE}_ALIAS_INCLUDE_DIRS})
    target_compile_options(${TARGET} PUBLIC ${${MODULE}_ALIAS_LDFLAGS} ${${MODULE}_ALIAS_LDFLAGS_OTHER} ${${MODULE}_ALIAS_CFLAGS} ${${MODULE}_ALIAS_CFLAGS_OTHER})
    find_library(LIB ${MODULE})
    if (NOT ${LIB} STREQUAL "LIB-NOTFOUND")
        target_link_libraries(${TARGET} ${LIB})
    endif()
    target_link_libraries(${TARGET} ${${MODULE}_ALIAS_LIBRARIES})
endfunction(target_link_module)


get_filename_component(3RD_PARTY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../third_party ABSOLUTE)

function(third_party_target LIB_NAME)
    if (${${LIB_NAME}_INCLUDE} AND ${${LIB_NAME}_LIBS})
        include_directories(${${LIB_NAME}_INCLUDE})
        set(LIB_DIRS ${LIB_DIRS} ${${LIB_NAME}_LIBS})
        return()
    endif()

    get_filename_component(SOURCE_DIR ${3RD_PARTY_SOURCES}/${LIB_NAME} ABSOLUTE)
    get_filename_component(BUILD_DIR "${CMAKE_BINARY_DIR}/third_party/${LIB_NAME}" ABSOLUTE)

    if (NOT TARGET ${LIB_NAME})
        message("CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
        ExternalProject_Add( ${LIB_NAME}
            PREFIX ${BUILD_DIR}
            SOURCE_DIR ${SOURCE_DIR}
            BINARY_DIR ${BUILD_DIR}
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} -D CMAKE_INSTALL_PREFIX=<INSTALL_DIR> -D CMAKE_CXX_FLAGS=-fPIC ${SOURCE_DIR}
            BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release)
    endif()

    ExternalProject_Get_Property(${LIB_NAME} INSTALL_DIR)
    include_directories(${INSTALL_DIR}/include)
    set(LIB_DIRS ${LIB_DIRS} ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR})
endfunction (third_party_target)

function(link_third_party_directories)
    link_directories(${LIB_DIRS})
endfunction(link_third_party_directories)


function(target_third_party_dependencies TARGET LIB_NAME LIBRARIES)
    add_dependencies(${TARGET} ${LIB_NAME})
    set(${LIBS} "")
    ExternalProject_Get_Property(${LIB_NAME} INSTALL_DIR)
    foreach (LIB ${LIBRARIES})
        set(LIBS ${LIBS} ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${LIB})
    endforeach(LIB)
    target_link_libraries(${TARGET} ${LIBS})
endfunction (target_third_party_dependencies)


if (NOT TARGET translation)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../mui ${CMAKE_BINARY_DIR}/mui)
endif()

set(BITNESS "")
if((CMAKE_SIZEOF_VOID_P EQUAL 4) OR (${CMAKE_CXX_FLAGS} MATCHES ".*-m32.*"))
  set(BITNESS ":i386")
endif()
