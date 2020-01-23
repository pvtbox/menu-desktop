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

#include "OverlayFactory.h"
#include "Overlay.h"

extern long dllReferenceCount;

OverlayFactory::OverlayFactory(int state)
    : _referenceCount(1), _state(state)
{
    InterlockedIncrement(&dllReferenceCount);
}

OverlayFactory::~OverlayFactory()
{
    InterlockedDecrement(&dllReferenceCount);
}

IFACEMETHODIMP OverlayFactory::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hResult = S_OK;

    if (IsEqualIID(IID_IUnknown, riid) ||
        IsEqualIID(IID_IClassFactory, riid))
    {
        *ppv = static_cast<IUnknown *>(this);
        AddRef();
    }
    else
    {
        hResult = E_NOINTERFACE;
        *ppv = NULL;
    }

    return hResult;
}

IFACEMETHODIMP_(ULONG) OverlayFactory::AddRef()
{
    return InterlockedIncrement(&_referenceCount);
}

IFACEMETHODIMP_(ULONG) OverlayFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&_referenceCount);

    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

IFACEMETHODIMP OverlayFactory::CreateInstance(
    IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hResult = CLASS_E_NOAGGREGATION;

    if (pUnkOuter != NULL) { return hResult; }

    hResult = E_OUTOFMEMORY;
    Overlay *lrOverlay = new (std::nothrow) Overlay(_state);
    if (!lrOverlay) { return hResult; }

    hResult = lrOverlay->QueryInterface(riid, ppv);
    lrOverlay->Release();

    return hResult;
}

IFACEMETHODIMP OverlayFactory::LockServer(BOOL fLock)
{
    if (fLock) {
        InterlockedIncrement(&dllReferenceCount);
    }
    else {
        InterlockedDecrement(&dllReferenceCount);
    }
    return S_OK;
}