cmake_minimum_required(VERSION 3.0.2)

include(../template.cmake)
include(../fixInstallPrefix.cmake)

# ============================ MAIN LIBRARY ==============================
set (LIBRARY_NAME pvtbox-thunarx-3-menu)
project(${LIBRARY_NAME} LANGUAGES CXX)

set (SRC_FILES
     ../common/gtk/GStringGuard.cpp
     ../common/MenuDescription.cpp
     ../common/gtk/FileInfo.cpp
     ../common/gtk/Utils.cpp
     ../common/PvtboxClientAPI.cpp
     Provider.cpp
     PvtboxThunar.c)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden -std=c++11")

find_package(PkgConfig REQUIRED)
link_module(thunarx-3)
third_party_target(jsoncpp)
third_party_target(nanomsg)

add_library (${LIBRARY_NAME} SHARED ${SRC_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES PREFIX "")

target_third_party_dependencies(${LIBRARY_NAME} jsoncpp libjsoncpp.a)
target_third_party_dependencies(${LIBRARY_NAME} nanomsg libnanomsg.so)
target_link_module(${LIBRARY_NAME} thunarx-3)


# ============================ INSTALLATION ==============================
execute_process(COMMAND pkg-config --variable=extensionsdir thunarx-3
                OUTPUT_VARIABLE INSTALLATION_DIR)
string(STRIP ${INSTALLATION_DIR} INSTALLATION_DIR)

message("Thunar Installation path: ${CMAKE_INSTALL_PREFIX}${INSTALLATION_DIR}")
install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}${INSTALLATION_DIR})
