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

#include "Icons.h"

#include <Uxtheme.h>
#include <gdiplus.h>
#include <objidl.h>

#pragma comment(lib, "uxtheme.lib")

HINSTANCE instance;

//=============================================================================
//
void IconsCache::initialize(HINSTANCE hInstance) {
    instance = hInstance;
}

//=============================================================================
//
bool hasAlpha(__in Gdiplus::ARGB* pargb,
              SIZE& sizImage,
              int cxRow) {
    ULONG cxDelta = cxRow - sizImage.cx;

    for (ULONG y = sizImage.cy; y; --y) {
        for (ULONG x = sizImage.cx; x; --x) {
            if (*pargb++ & 0xFF000000) {
                return true;
            }
        }

        pargb += cxDelta;
    }

    return false;
}

//=============================================================================
//
HRESULT convertToPARGB32(HDC hdc,
                         __inout Gdiplus::ARGB* pargb,
                         HBITMAP hbmp,
                         SIZE& sizImage,
                         int cxRow) {
    BITMAPINFO bmi;
    SecureZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth = sizImage.cx;
    bmi.bmiHeader.biHeight = sizImage.cy;
    bmi.bmiHeader.biBitCount = 32;

    HANDLE hHeap = GetProcessHeap();

    void* pvBits = HeapAlloc(hHeap, 0, bmi.bmiHeader.biWidth * 4 * bmi.bmiHeader.biHeight);

    if (pvBits == 0) {
        return E_OUTOFMEMORY;
    }

    HRESULT hResult = E_UNEXPECTED;

    if (GetDIBits(hdc, hbmp, 0, bmi.bmiHeader.biHeight, pvBits, &bmi, DIB_RGB_COLORS) == bmi.bmiHeader.biHeight) {
        ULONG cxDelta = cxRow - bmi.bmiHeader.biWidth;
        Gdiplus::ARGB* pargbMask = static_cast<Gdiplus::ARGB*>(pvBits);

        for (ULONG y = bmi.bmiHeader.biHeight; y; --y) {
            for (ULONG x = bmi.bmiHeader.biWidth; x; --x) {
                if (*pargbMask++) {
                    // transparent pixel
                    *pargb++ = 0;
                } else {
                    // opaque pixel
                    *pargb++ |= 0xFF000000;
                }
            }
            pargb += cxDelta;
        }

        hResult = S_OK;
    }

    HeapFree(hHeap, 0, pvBits);

    return hResult;
}

//=============================================================================
//
HRESULT convertBufferToPARGB32(HPAINTBUFFER hPaintBuffer,
                               HDC hdc,
                               HICON hicon,
                               SIZE& sizIcon) {
    RGBQUAD* prgbQuad;
    int cxRow;
    HRESULT hResult = GetBufferedPaintBits(hPaintBuffer, &prgbQuad, &cxRow);

    if (SUCCEEDED(hResult)) {
        Gdiplus::ARGB* pargb = reinterpret_cast<Gdiplus::ARGB*>(prgbQuad);

        if (!hasAlpha(pargb, sizIcon, cxRow)) {
            ICONINFO info;

            if (GetIconInfo(hicon, &info)) {

                if (info.hbmMask) {
                    hResult = convertToPARGB32(hdc, pargb, info.hbmMask, sizIcon, cxRow);
                }

                DeleteObject(info.hbmColor);
                DeleteObject(info.hbmMask);
            }
        }
    }

    return hResult;
}

//=============================================================================
//
HRESULT create32BitHBITMAP(HDC hdc,
                           const SIZE* psize,
                           __deref_opt_out void** ppvBits,
                           __out HBITMAP* phBmp) {
    if (psize == 0)
        return E_INVALIDARG;

    if (phBmp == 0)
        return E_POINTER;

    *phBmp = NULL;

    BITMAPINFO bmi;
    SecureZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth = psize->cx;
    bmi.bmiHeader.biHeight = psize->cy;
    bmi.bmiHeader.biBitCount = 32;

    HDC hdcUsed = hdc ? hdc : GetDC(NULL);

    if (hdcUsed) {
        *phBmp = CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);

        if (hdc != hdcUsed) {
            ReleaseDC(NULL, hdcUsed);
        }
    }

    return (NULL == *phBmp) ? E_OUTOFMEMORY : S_OK;
}

//=============================================================================
//
HBITMAP iconToBitmapPARGB32(HICON hIcon) {
    if (!hIcon)
        return NULL;

    SIZE sizIcon;
    sizIcon.cx = GetSystemMetrics(SM_CXSMICON);
    sizIcon.cy = GetSystemMetrics(SM_CYSMICON);

    RECT rcIcon;
    SetRect(&rcIcon, 0, 0, sizIcon.cx, sizIcon.cy);
    HBITMAP hBmp = NULL;

    HDC hdcDest = CreateCompatibleDC(NULL);
    if (hdcDest) {
        if (SUCCEEDED(create32BitHBITMAP(hdcDest, &sizIcon, NULL, &hBmp))) {
            HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcDest, hBmp);
            if (hbmpOld) {
                BLENDFUNCTION bfAlpha = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
                BP_PAINTPARAMS paintParams = {0};
                paintParams.cbSize = sizeof(paintParams);
                paintParams.dwFlags = BPPF_ERASE;
                paintParams.pBlendFunction = &bfAlpha;

                HDC hdcBuffer;
                HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(hdcDest, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
                if (hPaintBuffer) {
                    if (DrawIconEx(hdcBuffer, 0, 0, hIcon, sizIcon.cx, sizIcon.cy, 0, NULL, DI_NORMAL)) {
                        // If icon did not have an alpha channel we need to convert buffer to PARGB
                        convertBufferToPARGB32(hPaintBuffer, hdcDest, hIcon, sizIcon);
                    }

                    // This will write the buffer contents to the destination bitmap
                    EndBufferedPaint(hPaintBuffer, TRUE);
                }

                SelectObject(hdcDest, hbmpOld);
            }
        }

        DeleteDC(hdcDest);
    }

    return hBmp;
}

//=============================================================================
//
HBITMAP IconsCache::getIcon(const char* iconName) {
    if (strcmp(iconName, "pvtbox") == 0) {
        static HBITMAP pvtboxIcon = 0;
        if (!pvtboxIcon) {
            HICON icon = LoadIcon(instance, MAKEINTRESOURCE(IDI_PVTBOX));
            pvtboxIcon = iconToBitmapPARGB32(icon);
        }

        return pvtboxIcon;
    }

    return NULL;
}
