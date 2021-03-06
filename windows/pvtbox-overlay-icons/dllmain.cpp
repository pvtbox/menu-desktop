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
#include "OverlayFactory.h"

HINSTANCE instanceHandle = NULL;

long dllReferenceCount = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        instanceHandle = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

HRESULT CreateFactory(REFIID riid, void **ppv, int state)
{
    HRESULT hResult = E_OUTOFMEMORY;

    OverlayFactory* overlayFactory = new OverlayFactory(state);

    if (overlayFactory) {
        hResult = overlayFactory->QueryInterface(riid, ppv);
        overlayFactory->Release();
    }
    return hResult;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hResult = CLASS_E_CLASSNOTAVAILABLE;
    GUID guid;

    hResult = CLSIDFromString(OVERLAY_GUID_ERROR, (LPCLSID)&guid);
    if (!SUCCEEDED(hResult)) { return hResult; }
    if (IsEqualCLSID(guid, rclsid)) { return CreateFactory(riid, ppv, State_Error); }

    hResult = CLSIDFromString(OVERLAY_GUID_SYNCED, (LPCLSID)&guid);
    if (!SUCCEEDED(hResult)) { return hResult; }
    if (IsEqualCLSID(guid, rclsid)) { return CreateFactory(riid, ppv, State_Synced); }

    hResult = CLSIDFromString(OVERLAY_GUID_SYNCING, (LPCLSID)&guid);
    if (!SUCCEEDED(hResult)) { return hResult; }
    if (IsEqualCLSID(guid, rclsid)) { return CreateFactory(riid, ppv, State_Syncing); }

    hResult = CLSIDFromString(OVERLAY_GUID_PAUSED, (LPCLSID)&guid);
    if (!SUCCEEDED(hResult)) { return hResult; }
    if (IsEqualCLSID(guid, rclsid)) { return CreateFactory(riid, ppv, State_Paused); }

    hResult = CLSIDFromString(OVERLAY_GUID_ONLINE, (LPCLSID)&guid);
    if (!SUCCEEDED(hResult)) { return hResult; }
    if (IsEqualCLSID(guid, rclsid)) { return CreateFactory(riid, ppv, State_Online); }

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void)
{
    return  S_OK;
    return dllReferenceCount > 0 ? S_FALSE : S_OK;
}

HRESULT RegisterCLSID(LPCOLESTR guidStr, PCWSTR overlayStr, PCWSTR szModule)
{
    HRESULT hResult = S_OK;

    GUID guid;
    hResult = CLSIDFromString(guidStr, (LPCLSID)&guid);

    if (hResult != S_OK) {
        return hResult;
    }

    hResult = OverlayRegistrationHandler::RegisterCOMObject(szModule, OVERLAY_GENERIC_NAME, guid);

    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = OverlayRegistrationHandler::RegisterShellEntries(guid, overlayStr);

    return hResult;
}

HRESULT UnregisterCLSID(LPCOLESTR guidStr, PCWSTR overlayStr)
{
    HRESULT hResult = S_OK;
    GUID guid;

    hResult = CLSIDFromString(guidStr, (LPCLSID)&guid);

    if (hResult != S_OK) {
        return hResult;
    }

    hResult = OverlayRegistrationHandler::UnregisterCOMObject(guid);

    if (!SUCCEEDED(hResult)) {
        return hResult;
    }

    hResult = OverlayRegistrationHandler::UnregisterShellEntries(overlayStr);

    return hResult;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hResult = S_OK;

    wchar_t szModule[MAX_PATH * 2];

    if (GetModuleFileName(instanceHandle, szModule, MAX_PATH * 2) == 0) {
        hResult = HRESULT_FROM_WIN32(GetLastError());
        return hResult;
    }

    hResult = RegisterCLSID(OVERLAY_GUID_ERROR, OVERLAY_NAME_ERROR, szModule);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = RegisterCLSID(OVERLAY_GUID_SYNCED, OVERLAY_NAME_SYNCED, szModule);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = RegisterCLSID(OVERLAY_GUID_SYNCING, OVERLAY_NAME_SYNCING, szModule);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = RegisterCLSID(OVERLAY_GUID_PAUSED, OVERLAY_NAME_PAUSED, szModule);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = RegisterCLSID(OVERLAY_GUID_ONLINE, OVERLAY_NAME_ONLINE, szModule);

    return hResult;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hResult = S_OK;

    wchar_t szModule[MAX_PATH * 2];

    if (GetModuleFileNameW(instanceHandle, szModule, MAX_PATH * 2) == 0)
    {
        hResult = HRESULT_FROM_WIN32(GetLastError());
        return hResult;
    }

    hResult = UnregisterCLSID(OVERLAY_GUID_ERROR, OVERLAY_NAME_ERROR);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = UnregisterCLSID(OVERLAY_GUID_SYNCED, OVERLAY_NAME_SYNCED);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = UnregisterCLSID(OVERLAY_GUID_SYNCING, OVERLAY_NAME_SYNCING);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = UnregisterCLSID(OVERLAY_GUID_PAUSED, OVERLAY_NAME_PAUSED);
    if (!SUCCEEDED(hResult)) { return hResult; }
    hResult = UnregisterCLSID(OVERLAY_GUID_ONLINE, OVERLAY_NAME_ONLINE);

    return hResult;
}
