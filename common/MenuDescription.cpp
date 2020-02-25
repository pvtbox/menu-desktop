/**
*  
*  Pvtbox. Fast and secure file transfer & sync directly across your devices. 
*  Copyright © 2020  Pb Private Cloud Solutions Ltd. 
*  
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*  
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*  
**/
#include "MenuDescription.h"
#include "PvtboxClientAPI.h"

#include <cstring>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <sys/stat.h>
#endif

#define gettext(text) text

#ifdef _WIN32
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

#ifdef _WIN32
static std::wstring utf82ws(const std::string& str) {
    if (str.empty())
        return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[size];
    // std::valarray<char> buf(size + 1);
    size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, size);
    if (!size)
        throw std::runtime_error("Failed to convert from UTF8");

    return wstr;
}
#endif

bool isRootDirectory(const std::string& pathUtf8, const std::string& syncFolderUtf8) {
    bool isRoot = pathUtf8.find(SEPARATOR, syncFolderUtf8.size() + 1) == std::string::npos;
#ifdef _WIN32
    std::wstring path = utf82ws(pathUtf8);
    return PathIsDirectory(path.data()) && isRoot;
#else
    struct stat buffer;
    return (stat(pathUtf8.data(), &buffer) == 0 && (buffer.st_mode & S_IFDIR) && isRoot);
#endif
}

const MenuItemDescription menu = { // Main menu item description
    "pvt_box", // name for GTK
    gettext("Pvtbox"), // label
    gettext("http://pvtbox.net"), // tip
    "pvtbox", // icon name
    NULL}; // action

const MenuItemDescription insideSubMenu[] = { // Submenu for synchronized folder
    {"copy_link", // name for GTK
     gettext("Get link"), // label
     gettext("Get link to public resource"), // tip
     NULL, // icon name
     PvtboxClientAPI::sharePath}, // action

    {"block_link", // name for GTK
     gettext("Remove link"), // label
     gettext("Remove link to this resource"), // tip
     NULL, // icon name
     PvtboxClientAPI::blockPath}, // action

    {"email_link", // name for GTK
     gettext("Send link to E-mail"), // label
     gettext("Send link to E-mail"), // tip
     NULL, // icon name
     PvtboxClientAPI::emailLink}, // action

    {"view_on_web", // name for GTK
     gettext("Show on site"), // label
     gettext("View this file on site"), // tip
     NULL, // icon name
     PvtboxClientAPI::openLink}}; // action

const MenuItemDescription insideSubMenuWithCollabs[] = { // Submenu for synchronized folder with collaborations
    {"collaboration_settings", // name for GTK
     gettext("Collaboration settings"), // label
     gettext("Manage collaboration settings"), // tip
     NULL, // icon name
     PvtboxClientAPI::collaborationSettings}, // action
     insideSubMenu[0],  insideSubMenu[1],  insideSubMenu[2],  insideSubMenu[3] };

const MenuItemDescription insideSubMenuNotShared[] = { // Submenu for synchronized folder not shared
    insideSubMenu[0],  insideSubMenu[2],  insideSubMenu[3] };

const MenuItemDescription insideSubMenuNotSharedWithCollabs[] = { // Submenu for synchronized folder not shared with collaborations
    insideSubMenuWithCollabs[0], insideSubMenuWithCollabs[1],  insideSubMenuWithCollabs[3],  insideSubMenuWithCollabs[4] };

const MenuItemDescription outsideSubMenu[] = { // Submenu for all other folders
    {"copy_to_sync_folder", // name for GTK
     gettext("Copy to Pvtbox sync folder"), // label
     gettext("Copy to Pvtbox sync folder"), // tip
     "pvtbox", // icon name
     PvtboxClientAPI::copyToSyncDir}}; // action

bool MenuItemDescription::hasChildren() const {
    return action == NULL;
}


MenuDescription getMenu(const std::vector<std::string>& selectedFilesPaths,
                        const MenuItemDescription* parent) {
    MenuDescription result = {NULL, 0};
    if (selectedFilesPaths.empty()) {
        return result;
    }
    std::string syncFolderUtf8 = PvtboxClientAPI::getSyncDir();

    std::string selectedFilePath = selectedFilesPaths[0];
    if (selectedFilePath == syncFolderUtf8) {
        if (selectedFilesPaths.size() == 1) {
            return result;
        } else {
            selectedFilePath = selectedFilesPaths[1];
        }
    }

    if (parent == NULL) {
        result.items = &menu;
        result.size = 1;
        if (syncFolderUtf8.empty() || selectedFilePath.find(syncFolderUtf8 + SEPARATOR) == std::string::npos) {
            result.items = outsideSubMenu;
            result.size = sizeof(outsideSubMenu) / sizeof(outsideSubMenu[0]);
        }
    } else if (parent == &menu) {
        std::string is_shared = PvtboxClientAPI::isShared(selectedFilesPaths);
        bool showCollab = selectedFilesPaths.size() == 1 && isRootDirectory(selectedFilePath, syncFolderUtf8);
        if (is_shared.size() != 0){
            if (showCollab) {
                result.items = insideSubMenuWithCollabs;
                result.size = sizeof(insideSubMenuWithCollabs) / sizeof(insideSubMenuWithCollabs[0]);
            }
            else{
                result.items = insideSubMenu;
                result.size = sizeof(insideSubMenu) / sizeof(insideSubMenu[0]);
            }
        }
        else{
            if (showCollab) {
                result.items = insideSubMenuNotSharedWithCollabs;
                result.size = sizeof(insideSubMenuNotSharedWithCollabs) / sizeof(insideSubMenuNotSharedWithCollabs[0]);
            }
            else {
                result.items = insideSubMenuNotShared;
                result.size = sizeof(insideSubMenuNotShared) / sizeof(insideSubMenuNotShared[0]);
            }
        }
    }

    return result;
}
