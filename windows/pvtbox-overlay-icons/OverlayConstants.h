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

#define OVERLAY_GUID_ERROR          L"{5F86DE92-D539-4F03-99EF-9D6E40312E63}"
#define OVERLAY_GUID_SYNCED         L"{5F86DE93-D539-4F03-99EF-9D6E40312E63}"
#define OVERLAY_GUID_SYNCING        L"{5F86DE94-D539-4F03-99EF-9D6E40312E63}"
#define OVERLAY_GUID_PAUSED         L"{5F86DE95-D539-4F03-99EF-9D6E40312E63}"
#define OVERLAY_GUID_ONLINE         L"{5F86DE96-D539-4F03-99EF-9D6E40312E63}"

#define OVERLAY_GENERIC_NAME        L"pvtbox overlay handler"

#define OVERLAY_NAME_ERROR          L"     pvtboxError"
#define OVERLAY_NAME_SYNCED         L"     pvtboxSynced"
#define OVERLAY_NAME_SYNCING        L"     pvtboxSyncing"
#define OVERLAY_NAME_PAUSED         L"     pvtboxPaused"
#define OVERLAY_NAME_ONLINE         L"     pvtboxOnline"

#define REGISTRY_OVERLAY_KEY        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers"
#define REGISTRY_CLSID              L"CLSID"
#define REGISTRY_IN_PROCESS         L"InprocServer32"
#define REGISTRY_THREADING          L"ThreadingModel"
#define REGISTRY_APARTMENT          L"Apartment"
#define REGISTRY_VERSION            L"Version"
#define REGISTRY_VERSION_NUMBER     L"1.0"

//Registry values for running
#define REGISTRY_ENABLE_OVERLAY     L"EnableOverlay"

#define GET_FILE_OVERLAY_ID     L"getFileIconId"

#define PORT                34001