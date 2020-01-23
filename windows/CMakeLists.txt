cmake_minimum_required(VERSION 3.0.2)
include(../template.cmake)
include(MacroAddInterface.cmake)
set (LIBRARY_NAME pvtbox-menu)
project(${LIBRARY_NAME})


#======================= SOURCE FILES ==================================
set(PROJECT_IDL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ShellContextMenu.idl)
file (GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c*)
set(SRC_FILES
    ${SRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/MenuDescription.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/PvtboxClientAPI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ShellContextMenu.def
)

file (GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h*)
set(HEADER_FILES
    ${HEADER_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/MenuDescription.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/PvtboxClientAPI.h)

SOURCE_GROUP("Header files" FILES ${HEADER_FILES})

file (GLOB RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.rc ${CMAKE_CURRENT_SOURCE_DIR}/*.rgs)
SOURCE_GROUP("Resource files" FILES ${RESOURCE_FILES})

# ===================== THIRD PARTY =====================================
third_party_target(jsoncpp)
third_party_target(nanomsg)
third_party_target(libintl)


# ===================== GENERATE FROM MIDL ==============================
MACRO_ADD_INTERFACES(GENERATED_IDL_HEADERS GENERATED_IDL_IIDS GENERATED_IDL_PROXYS GENERATED_IDL_DLLDATA ${PROJECT_IDL_FILES})
SOURCE_GROUP("Auto generated IDL files" FILES ${GENERATED_IDL_HEADERS} ${GENERATED_IDL_IIDS} ${PROJECT_IDL_FILES})

# ======================= COMPILER OPTIONS ====================================
add_definitions(
    /Zi # Generates complete debugging information.
    /nologo # Suppresses display of sign-on banner.
    /W3 # Sets output warning level.
    /D WIN32 # Defines constants and macros.
    /D _WINDOWS # Defines constants and macros.
    /D NDEBUG # Defines constants and macros.
    /D _USRDLL # Defines constants and macros.
    /D _WINDLL # Defines constants and macros.
    /EHsc # Specifies the model of exception handling.
    /TP #   Specifies a C++ source file.
    /errorReport:queue # nables you to provide internal compiler error (ICE) information directly to the Visual C++ team.
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${LIBRARY_NAME} SHARED
    ${SRC_FILES}
    ${HEADER_FILES}
    ${PROJECT_IDL_FILES}
    ${GENERATED_IDL_HEADERS}
    ${GENERATED_IDL_IIDS}
    ${RESOURCE_FILES})

target_third_party_dependencies(${LIBRARY_NAME} jsoncpp jsoncpp.lib)
target_third_party_dependencies(${LIBRARY_NAME} nanomsg nanomsg.lib)
target_third_party_dependencies(${LIBRARY_NAME} libintl libintl.lib)

install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

add_subdirectory(pvtbox-overlay-icons)