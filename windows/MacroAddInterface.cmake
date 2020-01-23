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
include(MacroAddFileDependencies)

MACRO (MACRO_ADD_INTERFACES _header_list _iid_list _proxy_list _dlldata_list)
  FOREACH(_in_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_out_FILE ${_in_FILE} NAME_WE)
    GET_FILENAME_COMPONENT(_in_PATH ${_in_FILE} PATH)

    SET(_out_header_name ${_out_FILE}.h)
    SET(_out_header ${CMAKE_CURRENT_BINARY_DIR}/${_out_header_name})
    SET(_out_iid_name ${_out_FILE}.c)
    SET(_out_iid ${CMAKE_CURRENT_BINARY_DIR}/${_out_iid_name})
    SET(_out_proxy_name ${_out_FILE}_p.c)
    SET(_out_proxy ${CMAKE_CURRENT_BINARY_DIR}/${_out_proxy_name})
    SET(_out_dlldata_name ${_out_FILE}_dlldata.c)
    SET(_out_dlldata ${CMAKE_CURRENT_BINARY_DIR}/${_out_dlldata_name})

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out_header} ${_out_iid} ${_out_proxy} ${_out_dlldata}
      DEPENDS ${_in_FILE}
      COMMAND midl /header ${_out_header_name} /iid ${_out_iid_name} /proxy ${_out_proxy_name} /dlldata ${_out_dlldata_name} /robust ${_in_FILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    MACRO_ADD_FILE_DEPENDENCIES(
      ${_out_header}
      ${_in_FILE}
    )

    SET_SOURCE_FILES_PROPERTIES(
      ${_out_header}
      ${_out_iid}
      ${_out_proxy}
      ${_out_dlldata}
      PROPERTIES
      GENERATED TRUE
    )
    SET_SOURCE_FILES_PROPERTIES(${_in_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)

    SET(${_header_list} ${${_header_list}} ${_out_header})
    SET(${_iid_list} ${${_iid_list}} ${_out_iid})
    SET(${_proxy_list} ${${_proxy_list}} ${_out_proxy})
    SET(${_dlldata_list} ${${_dlldata_list}} ${_out_dlldata})

  ENDFOREACH(_in_FILE ${ARGN})

ENDMACRO (MACRO_ADD_INTERFACES)
