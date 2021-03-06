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

#ifndef OVERLAY_H
#define OVERLAY_H

#pragma once

class Overlay : public IShellIconOverlayIdentifier

{
public:
    Overlay(int state);

    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags);
    IFACEMETHODIMP GetPriority(int *pPriority);
    IFACEMETHODIMP IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib);
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) Release();

protected:
    ~Overlay();

private:
    long _referenceCount;
    int _state;
};

#endif
