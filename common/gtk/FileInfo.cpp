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
#include "FileInfo.h"
#include <cstring>
#include <stdexcept>

//=============================================================================
// Construct the FIleInfo from uri string.
// ATTENTION: this string will be freed by g_free by this class
GtkUtils::FileInfo::FileInfo(char* uri)
    : uri_(uri) {

    if (uri == NULL)
        throw std::invalid_argument("FileInfo::FileInfo: URI must be specified, but NULL is passed");
}

//=============================================================================
// Return true if this is a local file
bool GtkUtils::FileInfo::isLocal() {
    GStringGuard scheme = g_uri_parse_scheme(uri_);
    return std::strcmp(scheme, "file") == 0;
}

//=============================================================================
// Return UTF8 encoded string containing full path to file
const char* GtkUtils::FileInfo::fullPathUtf8() {
    if (!fullPathUtf8_) {
        GStringGuard fn = g_filename_from_uri(uri_, NULL, NULL);
        gsize dummy;
        fullPathUtf8_ = g_filename_to_utf8(fn, -1, NULL, &dummy, NULL);
    }

    return fullPathUtf8_;
}
