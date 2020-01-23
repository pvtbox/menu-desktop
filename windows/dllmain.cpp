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
#include "ShellContextMenu.h"
#include "resource.h"
#include "stdafx.h"
#include "stdlib.h"
#include "string"
#include "windows.h"

#include "dllmain.h"

#include "../common/Consts.h"
#include "../common/PvtboxClientAPI.h"
#include "FileLogger.h"
#include "Icons.h"
#include <libintl.h>

CShellContextMenuModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(hInstance, path, MAX_PATH);
    PathRemoveFileSpecW(path);

    SetDllDirectoryW(path);

    SetThreadLocale(LOCALE_CUSTOM_UI_DEFAULT);

    FileLogger::initialize(path);
    IconsCache::initialize(hInstance);
    PvtboxClientAPI::initialize(path);

    PathAppendW(path, L"locale");

    hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved);
}
