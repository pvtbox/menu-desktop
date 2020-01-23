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
#include "Utils.h"
#include "FileInfo.h"

//=============================================================================
// Execute specified action for all files
void GtkUtils::executeAction(GList* files,
                             GtkUtils::FileUriGetter getFileURI,
                             FileAction action) {
    try {
        if (!action)
            return;

        std::vector<std::string> paths;

        for (; files != NULL; files = files->next) {
            FileInfo file(getFileURI(files->data));
            paths.push_back(file.fullPathUtf8());
        }

        action(paths);

    } catch (const std::exception& e) {
        g_print("pvtbox: %s\n", e.what());
    }
}

//=============================================================================
// Guard constructor
GtkUtils::FileList::FileList(GList* files,
                             FileListCopyer copyList,
                             FileListEraser eraseList)
    : files_(copyList(files)) {

    eraseList_ = eraseList;
}

//=============================================================================
// Guard destructor
GtkUtils::FileList::~FileList() {
    eraseList_(files_);
}

//=============================================================================
// Returns file list
GList* GtkUtils::FileList::files() const {
    return files_;
}
