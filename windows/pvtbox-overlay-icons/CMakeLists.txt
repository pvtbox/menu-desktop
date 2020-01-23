cmake_minimum_required(VERSION 3.0.2)
set(LIBRARY_NAME pvtbox-overlays)
project(${LIBRARY_NAME})

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file (GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h*)
set(HEADER_FILES
    ${HEADER_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/ws_client.h)
	
third_party_target(jsoncpp)

add_library(${LIBRARY_NAME} SHARED
    DllMain.cpp
    Overlay.cpp
    OverlayFactory.cpp
    OverlayRegistrationHandler.cpp
    stdafx.cpp
    Overlay.rc
	pvtbox-overlays.def
    RemotePathChecker.cpp
    stdafx.cpp
    StringUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/ws_client.cpp
	
    ${HEADER_FILES}
)

target_third_party_dependencies(${LIBRARY_NAME} jsoncpp jsoncpp.lib)

install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
