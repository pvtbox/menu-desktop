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
#include "stdafx.h"

#include "dllmain.h"


using namespace ATL;

STDAPI DllCanUnloadNow(void) {
    return _AtlModule.DllCanUnloadNow();
}

_Check_return_ STDAPI DllGetClassObject(_In_ REFCLSID rclsid,
                                        _In_ REFIID riid,
                                        _Outptr_ LPVOID* ppv) {
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void) {
    ATL::AtlSetPerUserRegistration(true);
    HRESULT hr = _AtlModule.DllRegisterServer();
    return hr;
}

STDAPI DllUnregisterServer(void) {
    ATL::AtlSetPerUserRegistration(true);
    HRESULT hr = _AtlModule.DllUnregisterServer();
    return hr;
}

STDAPI DllInstall(BOOL bInstall, _In_opt_ LPCWSTR pszCmdLine) {
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = L"user";

    ATL::AtlSetPerUserRegistration(true);
    if (bInstall)
    {
        hr = DllRegisterServer();
        if (FAILED(hr))
        {
            DllUnregisterServer();
        }
    }
    else
    {
        hr = DllUnregisterServer();
    }

    return hr;
}


