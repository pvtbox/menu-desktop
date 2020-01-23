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
#include "resource.h"


#include "ShellContextMenu.h"
#include <comdef.h>
#include <shlobj.h>
#include <vector>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "error"
#endif

using namespace ATL;


// CShellExt

class ATL_NO_VTABLE CShellExt : public CComObjectRootEx<CComSingleThreadModel>,
                                public CComCoClass<CShellExt, &CLSID_ShellExt>,
                                public IDispatchImpl<IShellExt, &IID_IShellExt, &LIBID_ShellContextMenuLib, /*wMajor =*/1, /*wMinor =*/0>,
                                public IShellExtInit,
                                public IContextMenu {
public:
    CShellExt();

    DECLARE_REGISTRY_RESOURCEID(IDR_SHELLEXT)

    BEGIN_COM_MAP(CShellExt)
    COM_INTERFACE_ENTRY(IShellExt)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() {
        return S_OK;
    }

    void FinalRelease() {
    }

protected:
    UINT m_selectedFilesCount;
    std::vector<std::string> m_selectedFilesPaths;

public:
    // IShellExtInit
    STDMETHOD(Initialize)
    (LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IContextMenu interface implementation
    STDMETHOD(GetCommandString)
    (UINT_PTR, UINT, UINT*, LPSTR, UINT);

    STDMETHOD(InvokeCommand)
    (LPCMINVOKECOMMANDINFO);

    STDMETHOD(QueryContextMenu)
    (HMENU, UINT, UINT, UINT, UINT);
};

OBJECT_ENTRY_AUTO(__uuidof(ShellExt), CShellExt)
