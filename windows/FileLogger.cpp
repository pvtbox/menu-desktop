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
#include "FileLogger.h"
#include "stdafx.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <string>

//=============================================================================
// Clobal variable to store log file path
//wchar_t* filePath;
std::wstring filePath;

//=============================================================================
// Set log file path
void FileLogger::initialize(const wchar_t* path) {
#ifdef ENABLE_LOGGING
    filePath.assign(path);
    filePath += L"\\pvtbox-menu.log";
#endif
}

//=============================================================================
// Guard for the file
struct FileGuard {
    FileGuard(const std::wstring& filePath)
        : file_(filePath.c_str(), std::ios_base::out | std::ios_base::app) {
    }

    ~FileGuard() {
        file_.close();
    }

    void write(const char* message) {
#ifdef ENABLE_LOGGING
        std::time_t now = std::time(0);

#pragma warning(push)
#pragma warning(disable : 4996)
        file_ << std::put_time(std::localtime(&now), "%c %Z") << ": " << message << std::endl;
#pragma warning(pop)
#endif
    }

private:
    std::ofstream file_;
};

//=============================================================================
// Write message
void FileLogger::write(const char* message) {
#ifdef ENABLE_LOGGING
    try {
        if (filePath.empty())
            return;

        FileGuard f(filePath);
        f.write(message);
    } catch (...) {
        // Do nothing
        // Suppress all exceptions which can happens during the logging
    }
#endif
}
