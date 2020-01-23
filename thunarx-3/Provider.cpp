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

static void execAction(ThunarxMenuItem* menu_item, gpointer item_action);

const gchar* const ACTIVATE_EVENT_NAME = "activate";
const gchar* const FILES_PROPERTY_NAME = "files";


//=============================================================================
// Callback function to build context menu exstesion for the speciifed files
GList* getFileMenuItems(ThunarxMenuProvider* provider,
                        GtkWidget* window,
                        GList* files) {
    if (!files)
        return NULL;

    if (files->next != NULL)
        return NULL;

    try
    {
        GList* result = NULL;

        GtkUtils::FileInfo file(thunarx_file_info_get_uri(
            reinterpret_cast<ThunarxFileInfo*>(files->data)));
        if (!file.isLocal())
            return NULL;

        /* get menu description */
        const MenuDescription& menuDescription = getMenu(
            true, file.fullPathUtf8());
        if (!menuDescription.size)
            return NULL;

        gchar* file_name = thunarx_file_info_get_name(
            reinterpret_cast<ThunarxFileInfo*>(files->data));

        const MenuItemDescription& item = menuDescription.items[0];
        ThunarxMenuItem* menuItem = thunarx_menu_item_new(
            item.name,
            item.label,
            item.tip,
            item.iconName
        );

        if (item.action != NULL)
        {
            /* Link files to menuItem */
            g_object_set_data_full(
                reinterpret_cast<GObject*>(menuItem),
                FILES_PROPERTY_NAME,
                thunarx_file_info_list_copy(files),
                reinterpret_cast<GDestroyNotify>(thunarx_file_info_list_free));

            g_signal_connect(menuItem, ACTIVATE_EVENT_NAME,
                             G_CALLBACK(execAction), (gpointer*) item.action);
        }

        result = g_list_append(result, menuItem);

        /* Get submenu description */
        const MenuDescription& subMenuDescription = getMenu(
            true, file.fullPathUtf8(), &item);
        if (subMenuDescription.size)
        {
            ThunarxMenu* subMenu = thunarx_menu_new();
            thunarx_menu_item_set_menu(menuItem, subMenu);

            for (int i = 0; i < subMenuDescription.size; ++i)
            {
                const MenuItemDescription& subitem = subMenuDescription.items[i];
                ThunarxMenuItem* subMenuItem = thunarx_menu_item_new(
                    subitem.name,
                    subitem.label,
                    subitem.tip,
                    subitem.iconName
                );

                if (subitem.action != NULL)
                {
                    /* Link files to subMenuItem */
                    g_object_set_data_full(
                        reinterpret_cast<GObject*>(subMenuItem),
                        FILES_PROPERTY_NAME,
                        thunarx_file_info_list_copy(files),
                        reinterpret_cast<GDestroyNotify>(thunarx_file_info_list_free));

                    g_signal_connect(subMenuItem, ACTIVATE_EVENT_NAME,
                                     G_CALLBACK(execAction), (gpointer*) subitem.action);
                }

                thunarx_menu_append_item(subMenu, subMenuItem);
            }
        }

        return result;
    }
    catch (const std::exception& e)
    {
        g_print("pvtbox: %s\n", e.what());
    }

    return NULL;
}

static void execAction(ThunarxMenuItem* menu_item, gpointer item_action)
{
    /* Extract files data */
    GList* files = reinterpret_cast<GList*>(
        g_object_get_data(reinterpret_cast<GObject*>(menu_item), FILES_PROPERTY_NAME));
    GtkUtils::FileInfo file(thunarx_file_info_get_uri(
            reinterpret_cast<ThunarxFileInfo*>(files->data)));

    FileAction action = reinterpret_cast<FileAction>(item_action);
    action(file.fullPathUtf8());
}

extern "C" {
//=============================================================================
//
static void menuProviderInit(ThunarxMenuProviderIface* iface) {
    iface->get_file_menu_items = getFileMenuItems;
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
