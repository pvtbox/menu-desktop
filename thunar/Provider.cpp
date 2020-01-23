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
extern "C" {
#include "Provider.h"
}

#include "../common/MenuDescription.h"
#include "../common/gtk/FileInfo.h"
#include "CreateMenuItem.h"

//=============================================================================
// Callback function to build context menu exstesion for the speciifed files
GList* getFileMenuItems(ThunarxMenuProvider* provider,
                        GtkWidget* window,
                        GList* files) {
    if (!files)
        return NULL;

    try {
        std::vector<std::string> paths;
        GList *l;
        for (l = files; l != NULL; l = l->next)
        {
            GtkUtils::FileInfo file(thunarx_file_info_get_uri(
                reinterpret_cast<ThunarxFileInfo*>(l->data)));
            if (!file.isLocal())
                return NULL;
            paths.push_back(file.fullPathUtf8());
        }

        const MenuDescription& menu = getMenu(paths);
        std::tr1::shared_ptr<GtkUtils::FileList> fileList(
            new GtkUtils::FileList(files,
                                   thunarx_file_info_list_copy,
                                   thunarx_file_info_list_free));

        GList* result = NULL;
        for (int i = 0; i < menu.size; ++i) {
            const MenuItemDescription& item = menu.items[i];
            GtkAction* action = createMenuItem(item,
                                               fileList);
            result = g_list_append(result, action);
        }

        return result;
    } catch (const std::exception& e) {
        g_print("pvtbox: %s\n", e.what());
    }

    return NULL;
}

extern "C" {
//=============================================================================
//
static void menuProviderInit(ThunarxMenuProviderIface* iface) {
    iface->get_file_actions = getFileMenuItems;
}

//=============================================================================
// GObject
struct _MenuProviderClass {
    GObjectClass __parent__;
};

struct _MenuProvider {
    GObject __parent__;
};

//=============================================================================
// Thunarx
THUNARX_DEFINE_TYPE_EXTENDED(MenuProvider,
                             menu_provider,
                             G_TYPE_OBJECT,
                             GTypeFlags(0),
                             THUNARX_IMPLEMENT_INTERFACE(THUNARX_TYPE_MENU_PROVIDER,
                                                         menuProviderInit));

//=============================================================================
//
static void menu_provider_init(MenuProvider* menu_provider) {
}

//=============================================================================
//
static void menuProviderFinalize(GObject* object) {
    // MenuProvider* menu_provider = MENU_PROVIDER(object);
    (*G_OBJECT_CLASS(menu_provider_parent_class)->finalize)(object);
}

//=============================================================================
//
static void menu_provider_class_init(MenuProviderClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = menuProviderFinalize;
}
}
