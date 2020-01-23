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
#include "GStringGuard.h"

namespace GtkUtils {
    //=============================================================================
    // Guard to get file information from GTK
    struct FileInfo {

        // Construct the FIleInfo from uri string.
        // ATTENTION: this string will be freed by g_free by this class
        FileInfo(char* uri);

        // Return true if this is a local file
        bool isLocal();

        // Return UTF8 encoded string containing full path to file
        const char* fullPathUtf8();

    private:
        GStringGuard uri_;
        GStringGuard fullPathUtf8_;
    };
}
