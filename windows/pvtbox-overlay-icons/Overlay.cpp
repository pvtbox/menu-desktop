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

#include "Overlay.h"

#include "OverlayFactory.h"
#include "StringUtil.h"

#include "UtilConstants.h"
#include "RemotePathChecker.h"

#include "resource.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>

using namespace std;

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE instanceHandle;

#define IDM_DISPLAY 0  
#define IDB_OK 101

static const int icon_indeces[] = {1, 2, 0, 3, 4};

namespace {

    unique_ptr<PathChecker> s_instance;

    PathChecker *getGlobalChecker()
    {
        // On Vista we'll run into issue #2680 if we try to create the thread+pipe connection
        // on any DllGetClassObject of our registered classes.
        // Work around the issue by creating the static RemotePathChecker only once actually needed.
        static once_flag s_onceFlag;
        call_once(s_onceFlag, [] { s_instance.reset(new PathChecker); });

        return s_instance.get();
    }

}
Overlay::Overlay(int state)
    : _referenceCount(1)
    , _state(state)
{
}

Overlay::~Overlay(void)
{
}


IFACEMETHODIMP_(ULONG) Overlay::AddRef()
{
    return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP Overlay::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IShellIconOverlayIdentifier, riid))
    {
        *ppv = static_cast<IShellIconOverlayIdentifier *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        *ppv = NULL;
    }

    if (*ppv)
    {
        AddRef();
    }

    return hr;
}

IFACEMETHODIMP_(ULONG) Overlay::Release()
{
    ULONG cRef = InterlockedDecrement(&_referenceCount);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

IFACEMETHODIMP Overlay::GetPriority(int *pPriority)
{
    // this defines which handler has precedence, so
    // we order this in terms of likelyhood
    switch (_state) {
    case State_Synced:
        *pPriority = 1; break;
    case State_Online:
        *pPriority = 2; break;
    case State_Syncing:
        *pPriority = 3; break;
    case State_Paused:
        *pPriority = 4; break;
    case State_Error:
        *pPriority = 5; break;
    default:
        *pPriority = 6; break;
    }

    return S_OK;
}

IFACEMETHODIMP Overlay::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
    // log_f << "overlay log " << "IsMemberOf  start" << " state " << _state << std::endl;
    PathChecker* checker = getGlobalChecker();
    std::wstring sync_dir = checker->SyncDirectory();
    
    size_t pathLength = wcslen(pwszPath);
    // log_f << "overlay log " << "IsMemberOf " << StringUtil::toUtf8(pwszPath, pathLength) << std::endl;

    bool watched = sync_dir.empty() || StringUtil::isContainedIn(pwszPath, pathLength, sync_dir);
    if (!watched) {
        return MAKE_HRESULT(S_FALSE, 0, 0);
    }

    int state = 0;
    if (!checker->IsPathMonitored(pwszPath, &state)) {
        return MAKE_HRESULT(S_FALSE, 0, 0);
    }
    return MAKE_HRESULT(state == _state ? S_OK : S_FALSE, 0, 0);
}

IFACEMETHODIMP Overlay::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags)
{
    *pIndex = 0;
    *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
    *pIndex = icon_indeces[_state];

    if (GetModuleFileName(instanceHandle, pwszIconFile, cchMax) == 0) {
        HRESULT hResult = HRESULT_FROM_WIN32(GetLastError());
        wcerr << L"IsOK? " << (hResult == S_OK) << L" with path " << pwszIconFile << L", index " << *pIndex << endl;
        return hResult;
    }

    return S_OK;
}