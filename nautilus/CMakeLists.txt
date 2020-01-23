cmake_minimum_required(VERSION 3.0.2)
include(../template.cmake)
include(../fixInstallPrefix.cmake)

# ============================ MAIN LIBRARY ==============================
set (LIBRARY_NAME pvtbox-nautilus-menu)
project(${LIBRARY_NAME} LANGUAGES CXX)

set (SRC_FILES
     ../common/gtk/GStringGuard.cpp
     ../common/MenuDescription.cpp
     ../common/gtk/FileInfo.cpp
     ../common/gtk/Utils.cpp
     ../common/PvtboxClientAPI.cpp
     ../common/ws_client.cpp
     WSListener.cpp
     PvtboxNautilus.cpp)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden -std=c++11 -pthread")

find_package(PkgConfig REQUIRED)
link_module(libnautilus-extension)
third_party_target(jsoncpp)
third_party_target(nanomsg)

add_library (${LIBRARY_NAME} SHARED ${SRC_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES PREFIX "")

target_third_party_dependencies(${LIBRARY_NAME} jsoncpp libjsoncpp.a)
target_third_party_dependencies(${LIBRARY_NAME} nanomsg libnanomsg.so)
target_link_module(${LIBRARY_NAME} libnautilus-extension)


# ============================ INSTALLATION ==============================
message("Nautilus installation path: ${CMAKE_INSTALL_PREFIX}/usr/lib/nautilus/extensions-3.0")
install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/lib/nautilus/extensions-3.0)
