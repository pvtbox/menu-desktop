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

#include "../common/MenuDescription.h"
#include "FileLogger.h"
#include "Icons.h"
#include "ShellExt.h"

#include <Strsafe.h>
#include <Windows.h>

#include <functional>
#include <unordered_map>
#include <valarray>
#include <vector>

#include <json/json.h>
#include <libintl.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

//=============================================================================
// Debug output message.
inline void logError(const char* message) {
    FileLogger::write(message);
}

//=============================================================================
// Exception type extend std::runtime_error with logical error message
struct Exception : std::runtime_error {
    Exception(const char* logicalError,
              const char* details)
        : logicalError(logicalError),
          std::runtime_error(details) {
    }

    const char* logicalError;
};

//=============================================================================
// Typical exception handling
template <typename ResultType>
ResultType HandleExceptoins(const ResultType& errorResult,
                            std::function<ResultType()> action) {
    try {
        return action();
    } catch (const Exception& e) {
        logError(e.logicalError);
        logError(e.what());
    } catch (const std::exception& e) {
        logError(e.what());
    } catch (...) {
        logError("Unknown error!");
    }

    return errorResult;
}

//=============================================================================
// Raise last error for the thread
void RaiseLastError(const char* logicalError) {
    const int msgSize = 0xFFF;
    char errorMessage[msgSize] = {0};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   LOCALE_CUSTOM_UI_DEFAULT,
                   errorMessage,
                   msgSize - 1,
                   NULL);
    throw Exception(logicalError, errorMessage);
}

//=============================================================================
// Convert WideChar string to specified codepage
std::string WideCharTo(UINT CodePage, const std::wstring& wstr) {
    if (wstr.empty())
        return std::string();

    int size = WideCharToMultiByte(CodePage, 0, wstr.c_str(), int(wstr.size()), 0, 0, NULL, NULL);
    std::valarray<char> buf(size + 1);
    size = WideCharToMultiByte(CodePage, 0, wstr.c_str(), int(wstr.size()), &buf[0], size, NULL, NULL);
    if (!size)
        RaiseLastError("Can't convert from WideChar");

    buf[size] = '\0';
    return std::string(&buf[0]);
}

//=============================================================================
// Convert UTF-8 encoded string to specified codepage
std::string Utf8To(UINT CodePage, const std::string& utf8str) {
    if (utf8str.empty())
        return utf8str;

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), int(utf8str.size()), 0, 0);
    std::valarray<wchar_t> buf(size + 1);
    size = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), int(utf8str.size()), &buf[0], size);
    if (!size)
        RaiseLastError("Can't convert from UTF-8 to WideChar");

    buf[size] = L'\0';
    return WideCharTo(CodePage, &buf[0]);
}

std::wstring utf8ws2(const std::string& str) {
    if (str.empty())
        return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[size];
    // std::valarray<char> buf(size + 1);
    size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, size);
    if (!size)
        throw std::runtime_error("Failed to convert from UTF8");

    return wstr;
}

//=============================================================================
// Constructor
CShellExt::CShellExt() {
}

//=============================================================================
// Guard class to correctly release global lock and medium
struct DataObjectGuard {
    DataObjectGuard(LPDATAOBJECT pDataObj)
        : hDrop_(0),
          stg_({TYMED_HGLOBAL}) {

        FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        if (FAILED(pDataObj->GetData(&fmt, &stg_)))
            throw std::runtime_error("Failed to get data about selected files.");

        hDrop_ = (HDROP) GlobalLock(stg_.hGlobal);
    }

    ~DataObjectGuard() {
        if (hDrop_) {
            GlobalUnlock(stg_.hGlobal);
            ReleaseStgMedium(&stg_);
        }
    }

    HDROP hDrop() {
        return hDrop_;
    }

private:
    STGMEDIUM stg_;
    HDROP hDrop_;
};

HRESULT CShellExt::Initialize(LPCITEMIDLIST pidlFolder,
                              LPDATAOBJECT pDataObj,
                              HKEY hProgID) {
    FileLogger::write("CShellExt::Initialize");
    return HandleExceptoins<HRESULT>(E_INVALIDARG, [=]() {
        DataObjectGuard dataObject(pDataObj);
        FileLogger::write("CShellExt::Initialize callback");
        m_selectedFilesPaths.clear();
        // Sanity check – make sure there is at least one filename.
        m_selectedFilesCount = DragQueryFile(dataObject.hDrop(), 0xFFFFFFFF, NULL, 0);
        FileLogger::write((std::string("CShellExt::Initialize callback selected files count: ") + std::to_string(m_selectedFilesCount)).c_str());
        if (m_selectedFilesCount == 0)
            throw std::runtime_error("No selected files");

        for (int i = 0; i < m_selectedFilesCount; ++i) {
            const int size = DragQueryFileW(dataObject.hDrop(), i, NULL, 0);
            std::valarray<wchar_t> buf(size + 1);
            if (size != DragQueryFileW(dataObject.hDrop(), i, &buf[0], size + 1))
                throw std::runtime_error("Can't get file path");
            buf[size] = L'\0';

            std::string path = WideCharTo(CP_UTF8, &buf[0]);
            FileLogger::write((std::string("CShellExt::Initialize callback selected path: ") + path).c_str());
            m_selectedFilesPaths.push_back(path);
        }
        return S_OK;
    });
}

//=============================================================================
// Map menu items to unique id and back
typedef std::unordered_map<const MenuItemDescription*, WORD> Commands;
typedef std::vector<const MenuItemDescription*> MenuItems;

Commands commands;
MenuItems menuItems;
WORD getCommandIdByMenuItemDescription(const MenuItemDescription& menuItemDescription) { // Must be as fast as possible
    Commands::iterator command = commands.find(&menuItemDescription);
    if (command != commands.end())
        return command->second;

    menuItems.push_back(&menuItemDescription);
    commands[&menuItemDescription] = WORD(menuItems.size() - 1);
    return WORD(menuItems.size() - 1);
}

const MenuItemDescription* getMenuItemDescriptionByCommand(WORD command) {
    return menuItems[command];
}

//=============================================================================
// Recursively build Pvtbox context menu
void buildMenu(const std::vector<std::string>& selectedFilesPaths,
               const HMENU hMenu,
               const UINT uidFirstCmd,
               const MenuItemDescription* parent = NULL) {
    FileLogger::write("ShellExt::buildMenu");
    MenuDescription menuDescription = getMenu(selectedFilesPaths,
                                                     parent);
    for (int i = 0; i < menuDescription.size; ++i) {
        const MenuItemDescription& menuItem = menuDescription.items[i];
        MENUITEMINFO menuItemInfo = {sizeof(MENUITEMINFO)};
        const WORD command = getCommandIdByMenuItemDescription(menuItem);
        menuItemInfo.fMask = MIIM_STRING | MIIM_ID;
        std::wstring text = utf8ws2(gettext(menuItem.label));
        menuItemInfo.dwTypeData = &text[0];
        menuItemInfo.wID = uidFirstCmd + command;

        if (menuItem.hasChildren()) {
            menuItemInfo.fMask |= MIIM_SUBMENU;
            HMENU hSubmenu = CreatePopupMenu();
            buildMenu(selectedFilesPaths,
                      hSubmenu,
                      uidFirstCmd,
                      &menuItem);
            menuItemInfo.hSubMenu = hSubmenu;
        }

        if (menuItem.iconName) {
            if (HBITMAP icon = IconsCache::getIcon(menuItem.iconName)) {
                menuItemInfo.fMask |= MIIM_BITMAP;
                menuItemInfo.hbmpItem = icon;
            }
        }

        InsertMenuItem(hMenu, command, true, &menuItemInfo);
    }
}


//=============================================================================

HRESULT CShellExt::QueryContextMenu(HMENU hmenu,
                                    UINT uMenuIndex,
                                    UINT uidFirstCmd,
                                    UINT uidLastCmd,
                                    UINT uFlags) {
    HRESULT errorResult = MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

    return HandleExceptoins<HRESULT>(errorResult, [=]() {
        // If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
        if (uFlags & (CMF_DEFAULTONLY | CMF_NOVERBS | CMF_VERBSONLY))
            return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

        // skip Windows shortcuts
        std::vector<std::string>::iterator it = m_selectedFilesPaths.begin();
        while(it != m_selectedFilesPaths.end()) {
            std::string path = *it;
            if (path.length() > 4 && strcmp(".lnk", path.substr(path.length() - 4, 4).c_str()) == 0) {
                it = m_selectedFilesPaths.erase(it);
            } else {
                it++;
            }
        }
        if (m_selectedFilesPaths.empty()) {
            return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
        }

        buildMenu(m_selectedFilesPaths,
                  hmenu,
                  uidFirstCmd);

        return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, menuItems.size());
    });
}

//=============================================================================
// Return some command details.
// In current implementation only tooltips are defined here
HRESULT CShellExt::GetCommandString(UINT_PTR idCmd,
                                    UINT uFlags,
                                    UINT* pwReserved,
                                    LPSTR pszName,
                                    UINT cchMax) {
    return HandleExceptoins<HRESULT>(E_INVALIDARG, [=]() {
        USES_CONVERSION;

        if (!(uFlags & GCS_HELPTEXT)) // Only tooltips required for us
            return E_INVALIDARG;

        // Check idCmd
        if (menuItems.size() <= idCmd)
            return E_INVALIDARG;

        MenuItemDescription menuItemDescription = *getMenuItemDescriptionByCommand(WORD(idCmd));
        const char* const tooltipUtf8 = gettext(menuItemDescription.tip);

        if (uFlags & GCS_UNICODE) {
            wchar_t* const wName = reinterpret_cast<wchar_t*>(pszName);
            const int size = cchMax * sizeof(pszName[0]) / sizeof(wName);
            MultiByteToWideChar(CP_THREAD_ACP, 0, tooltipUtf8, -1, wName, size);
        } else {
            std::string tooltip = Utf8To(CP_THREAD_ACP, tooltipUtf8);
            StringCchCopyA(pszName, cchMax, &tooltip[0]);
        }

        return S_OK;
    });
}

//=============================================================================
// Run menu item handler
HRESULT CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo) {
    return HandleExceptoins<HRESULT>(E_INVALIDARG, [=]() {
        // If lpVerb really points to a string, ignore this function call and bail out.
        if (0 != HIWORD(pCmdInfo->lpVerb))
            return E_INVALIDARG;

        const WORD iCmd = LOWORD(pCmdInfo->lpVerb);
        const MenuItemDescription* menuItemDescription = getMenuItemDescriptionByCommand(iCmd);
        if (menuItemDescription->action)
            menuItemDescription->action(m_selectedFilesPaths);

        return S_OK;
    });
}
