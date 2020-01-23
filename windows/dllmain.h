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
class CShellContextMenuModule : public ATL::CAtlDllModuleT<CShellContextMenuModule> {
public:
    DECLARE_LIBID(LIBID_ShellContextMenuLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SHELLCONTEXTMENU, "{B39CAA84-9FBC-4EF3-BB5A-F56A3D8BCFE5}")
};

extern class CShellContextMenuModule _AtlModule;
