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
#include "stdafx.h"

#include "OverlayRegistrationHandler.h"

#include <iostream>
#include <fstream>

using namespace std;

HRESULT OverlayRegistrationHandler::RegisterShellEntries(const CLSID& clsid, PCWSTR friendlyName)
{
    HRESULT hResult;
    HKEY shellOverlayKey = NULL;
    // the key may not exist yet
    hResult = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_OVERLAY_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &shellOverlayKey, NULL));
    if (!SUCCEEDED(hResult)) {
        hResult = RegCreateKey(HKEY_LOCAL_MACHINE, REGISTRY_OVERLAY_KEY, &shellOverlayKey);
        if (!SUCCEEDED(hResult)) {
            return hResult;
        }
    }

    HKEY syncExOverlayKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegCreateKeyEx(shellOverlayKey, friendlyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &syncExOverlayKey, NULL));

    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    wchar_t stringCLSID[MAX_PATH];
    StringFromGUID2(clsid, stringCLSID, MAX_PATH);
    LPCTSTR value = stringCLSID;
    hResult = RegSetValueEx(syncExOverlayKey, NULL, 0, REG_SZ, (LPBYTE)value, (DWORD)((wcslen(value) + 1) * sizeof(TCHAR)));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    return hResult;
}

HRESULT OverlayRegistrationHandler::UnregisterShellEntries(PCWSTR friendlyName)
{
    HRESULT hResult;
    HKEY shellOverlayKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_OVERLAY_KEY, 0, KEY_WRITE, &shellOverlayKey));

    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    HKEY syncExOverlayKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegDeleteKey(shellOverlayKey, friendlyName));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    return hResult;
}

HRESULT OverlayRegistrationHandler::RegisterCOMObject(PCWSTR modulePath, PCWSTR friendlyName, const CLSID& clsid)
{
    if (modulePath == NULL) {
        return E_FAIL;
    }

    wchar_t stringCLSID[MAX_PATH];
    StringFromGUID2(clsid, stringCLSID, MAX_PATH);
    HRESULT hResult;
    HKEY hKey = NULL;

    hResult = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CLASSES_ROOT, REGISTRY_CLSID, 0, KEY_WRITE, &hKey));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    HKEY clsidKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, stringCLSID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &clsidKey, NULL));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = HRESULT_FROM_WIN32(RegSetValue(clsidKey, NULL, REG_SZ, friendlyName, (DWORD)wcslen(friendlyName)));

    HKEY inprocessKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegCreateKeyEx(clsidKey, REGISTRY_IN_PROCESS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &inprocessKey, NULL));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = HRESULT_FROM_WIN32(RegSetValueEx(inprocessKey, NULL, NULL, REG_EXPAND_SZ, (const BYTE*)L"\%userprofile\%\\AppData\\Local\\Pvtbox\\pvtbox-overlays.dll", 109));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = HRESULT_FROM_WIN32(RegSetValueEx(inprocessKey, REGISTRY_THREADING, 0, REG_SZ, (LPBYTE)REGISTRY_APARTMENT, (DWORD)((wcslen(REGISTRY_APARTMENT) + 1) * sizeof(TCHAR))));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = HRESULT_FROM_WIN32(RegSetValueEx(inprocessKey, REGISTRY_VERSION, 0, REG_SZ, (LPBYTE)REGISTRY_VERSION_NUMBER, (DWORD)(wcslen(REGISTRY_VERSION_NUMBER) + 1) * sizeof(TCHAR)));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    return S_OK;
}

HRESULT OverlayRegistrationHandler::UnregisterCOMObject(const CLSID& clsid)
{
    wchar_t stringCLSID[MAX_PATH];

    StringFromGUID2(clsid, stringCLSID, MAX_PATH);
    HRESULT hResult;
    HKEY hKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CLASSES_ROOT, REGISTRY_CLSID, 0, DELETE, &hKey));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    HKEY clsidKey = NULL;
    hResult = HRESULT_FROM_WIN32(RegOpenKeyEx(hKey, stringCLSID, 0, DELETE, &clsidKey));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = HRESULT_FROM_WIN32(RegDeleteKey(clsidKey, REGISTRY_IN_PROCESS));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = HRESULT_FROM_WIN32(RegDeleteKey(hKey, stringCLSID));
    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    return S_OK;
}