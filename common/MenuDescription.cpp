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

#define gettext(text) text

#ifdef _WIN32
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

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

const MenuItemDescription insideSubMenuNotShared[] = { // Submenu for synchronized folder not shared
    insideSubMenu[0],  insideSubMenu[2] ,  insideSubMenu[3]};

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
        result.items = insideSubMenu;
        result.size = sizeof(insideSubMenu) / sizeof(insideSubMenu[0]);
        std::string is_shared = PvtboxClientAPI::isShared(selectedFilesPaths);
        if (is_shared.size() == 0) {
            result.items = insideSubMenuNotShared;
            result.size = sizeof(insideSubMenuNotShared) / sizeof(insideSubMenuNotShared[0]);
        }
    }

    return result;
}
