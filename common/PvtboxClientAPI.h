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

#include <stdexcept>
#include <string>
#include <vector>

namespace PvtboxClientAPI {
    // Set app start command line file path
    // path must not have trailing slash '/'
    #ifdef _WIN32
    void initialize(const wchar_t* path);
    #endif
 
    std::string getSyncDir();

    void copyToSyncDir(const std::vector<std::string>& selectedFilesPaths);

    void sharePath(const std::vector<std::string>& selectedFilesPaths);

    void blockPath(const std::vector<std::string>& selectedFilesPaths);

    void emailLink(const std::vector<std::string>& selectedFilesPaths);

    void openLink(const std::vector<std::string>& selectedFilesPaths);

    std::string isShared(const std::vector<std::string>& selectedFilesPaths);
}
