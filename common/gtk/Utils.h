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
#pragma once

#include "../MenuDescription.h"
#include <gtk/gtk.h>

namespace GtkUtils {
    typedef char* (*FileUriGetter)(void*);
    typedef GList* (*FileListCopyer)(GList*);
    typedef void (*FileListEraser)(GList*);

    // Execute specified action for all files
    void executeAction(GList* files,
                       FileUriGetter getFileURI,
                       FileAction action);

    // File list guard class
    struct FileList {
        FileList(GList* files,
                 FileListCopyer copyList,
                 FileListEraser eraseList);
        ~FileList();

        GList* files() const;

    private:
        GList* const files_;
        FileListEraser eraseList_;
    };
}
