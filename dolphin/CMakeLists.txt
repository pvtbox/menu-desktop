cmake_minimum_required(VERSION 3.0.2)
include(../template.cmake)
include(../fixInstallPrefix.cmake)

# ============================= SET PATHS =======================================
option(QT5_PREFIX_PATH "Path to QT5 modules" "")
option(KF5_PREFIX_PATH "Path to KF5 modules" "")

if (QT5_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_PREFIX_PATH})
endif()

if (KF5_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${KF5_PREFIX_PATH})
endif()

message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")


# =========================== PROJECT ===========================================
set(libraryName pvtbox-dolphin-menu)
project(${libraryName})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
find_package(KF5KIO REQUIRED)
find_package(KF5I18n REQUIRED)

third_party_target(jsoncpp)
third_party_target(nanomsg)

#set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=c++11")

find_package(ECM ${ECM_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)

set(PvtboxDolphin_SRCS
    ../common/MenuDescription.cpp
    ../common/PvtboxClientAPI.cpp
    PvtboxDolphin.cpp
)

kcoreaddons_add_plugin(${libraryName} SOURCES ${PvtboxDolphin_SRCS} INSTALL_NAMESPACE "kf5/kfileitemaction")
kcoreaddons_desktop_to_json(${libraryName} ${libraryName}.desktop)

target_link_libraries(${libraryName}
    KF5::KIOWidgets
    KF5::I18n
    KF5::XmlGui
)

target_third_party_dependencies(${libraryName} jsoncpp libjsoncpp.a)
target_third_party_dependencies(${libraryName} nanomsg libnanomsg.so)


# ============================== OVERLAY PLUGINS ==============================
set(overlayPluginName pvtbox-dolphin-overlay)

set(PvtboxOverlayPlugin_SRCS
    ../common/ws_client.cpp
    PvtboxOverlayPlugin.cpp
)

kcoreaddons_add_plugin(${overlayPluginName} SOURCES ${PvtboxOverlayPlugin_SRCS} INSTALL_NAMESPACE "kf5/overlayicon")
#kcoreaddons_desktop_to_json(${overlayPluginName} ${overlayPluginName}.desktop)

target_link_libraries(${overlayPluginName}
    KF5::KIOWidgets
)

target_third_party_dependencies(${overlayPluginName} jsoncpp libjsoncpp.a)
target_third_party_dependencies(${overlayPluginName} nanomsg libnanomsg.so)


# ================================ INSTALLATION ===============================

message("Dolphin installation service dir: ${CMAKE_INSTALL_PREFIX}/usr/${KDE_INSTALL_KSERVICES5DIR}")
message("Dolphin installation plugin dir: ${CMAKE_INSTALL_PREFIX}/usr/${KDE_INSTALL_QTPLUGINDIR}")

install(FILES ${libraryName}.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS ${libraryName} DESTINATION ${CMAKE_INSTALL_PREFIX}/usr/${KDE_INSTALL_QTPLUGINDIR})
