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
#include <cstring>
#include <libintl.h>
#include <stdexcept>

#include "../common/Consts.h"
#include "../common/PvtboxClientAPI.h"
#include "../common/MenuDescription.h"
#include "../common/gtk/FileInfo.h"
#include "../common/gtk/GStringGuard.h"
#include "../common/gtk/Utils.h"
#include "PvtboxNautilus.h"
#include "WSListener.h"


#define EXPORT __attribute__((visibility("default")))

void on_status_changed(std::string path, std::string status);


//=============================================================================
// Global variables
GType extensionType;
WSListener ws_listener(on_status_changed);

//=============================================================================
// Strings
const gchar* const extensionName = "PvtboxNautilusExtension";

const gchar* const activateMenuItemEventName = "activate";

const gchar* const filesDataKey = "pvtbox_extension_files";
const gchar* const actionDataKey = "pvtbox_extension_action";


//=============================================================================
//
template <typename T>
T getMenuItemData(NautilusMenuItem* item, const gchar* key) {
    return reinterpret_cast<T>(
        g_object_get_data(
            reinterpret_cast<GObject*>(item),
            key));
}


//=============================================================================
// Callback function to run the action associated with menu item
void runMenuItem(NautilusMenuItem* item, gpointer user_data) {
    GtkUtils::executeAction(getMenuItemData<GList*>(item, filesDataKey),
                            reinterpret_cast<GtkUtils::FileUriGetter>(nautilus_file_info_get_uri),
                            getMenuItemData<FileAction>(item, actionDataKey));
}


//=============================================================================
// Recursive function to build menu according the description
NautilusMenuItem* createMenuItem(const MenuItemDescription& menuItem,
                                 GList* files,
                                 const char* pathUtf8,
                                 NautilusMenuProvider* provider) {
    NautilusMenuItem* item = nautilus_menu_item_new(
        menuItem.name,
        gettext(menuItem.label),
        gettext(menuItem.tip),
        menuItem.iconName);

    if (menuItem.action) {
        g_object_set_data_full(
            reinterpret_cast<GObject*>(item), // GObject *object
            filesDataKey, // const gchar *key
            nautilus_file_info_list_copy(files), // gpointer data
            reinterpret_cast<GDestroyNotify>(nautilus_file_info_list_free)); // GDestroyNotify destroy

        g_object_set_data_full(
            reinterpret_cast<GObject*>(item), // GObject *object
            actionDataKey, // const gchar *key
            reinterpret_cast<gpointer>(menuItem.action), // gpointer data
            NULL); // GDestroyNotify destroy

        g_signal_connect(item, activateMenuItemEventName, G_CALLBACK(runMenuItem), provider);
    }

    std::vector<std::string> paths;
    GList *l;
    for (l = files; l != NULL; l = l->next)
    {
        GtkUtils::FileInfo file(nautilus_file_info_get_uri(
            reinterpret_cast<NautilusFileInfo*>(l->data)));
        if (!file.isLocal())
            return NULL;
        paths.push_back(file.fullPathUtf8());
    }

    const MenuDescription& subMenuDescription = getMenu(paths,
                                                        &menuItem);
    if (subMenuDescription.size && menuItem.hasChildren())
    {
        NautilusMenu* subMenu = nautilus_menu_new();
        nautilus_menu_item_set_submenu(item, subMenu);
        for (int i = 0; i < subMenuDescription.size; ++i) {
            nautilus_menu_append_item(
                subMenu,
                createMenuItem(
                    subMenuDescription.items[i],
                    files,
                    pathUtf8,
                    provider));
        }
    }

    return item;
}


//=============================================================================
// Callback function to build context menu exstesion for the speciifed files
GList* getMenuItems(NautilusMenuProvider* provider,
                    GtkWidget* window,
                    GList* files) {
    try {

        std::vector<std::string> paths;
        GList *l;
        for (l = files; l != NULL; l = l->next)
        {
            GtkUtils::FileInfo file(nautilus_file_info_get_uri(NAUTILUS_FILE_INFO(l->data)));
            if (!file.isLocal()) // Support only local files
                return NULL;
            paths.push_back(file.fullPathUtf8());
        }

        const MenuDescription& menuDescription = getMenu(paths);
        GList* result = NULL;
        for (int i = 0; i < menuDescription.size; ++i) {
            result = g_list_append(
                result,
                createMenuItem(
                    menuDescription.items[i],
                    files,
                    "",
                    provider));
        }

        return result;
    } catch (const std::exception& e) {
        g_print("pvtbox: %s\n", e.what());
    }

    return NULL;
}


//=============================================================================
// Callback function to update info for the speciifed files
static NautilusOperationResult updateFileInfo(
    NautilusInfoProvider* provider,
    NautilusFileInfo* file,
    G_GNUC_UNUSED GClosure* update_complete,
    G_GNUC_UNUSED NautilusOperationHandle** handle)
{
    GFile* g_file;
    gchar* path;
    gchar* uri_scheme;

    g_file = nautilus_file_info_get_location(file);
    if (!g_file)
    {
        return NAUTILUS_OPERATION_COMPLETE;
    }

    path = g_file_get_path(g_file);
    if (!path)
    {
        return NAUTILUS_OPERATION_COMPLETE;
    }

    uri_scheme = g_file_get_uri_scheme(g_file);
    if (g_ascii_strcasecmp(uri_scheme, "file") != 0)
    {
        g_free(uri_scheme);
        return NAUTILUS_OPERATION_COMPLETE;
    }
    g_free(uri_scheme);

    std::string status = ws_listener.getStatus(path);
    if ( status == "")
    {
        ws_listener.addPath(path);
    }
    else
    {
        on_status_changed(path, status);
    }

    g_free(path);

    return NAUTILUS_OPERATION_COMPLETE;
}


//=============================================================================
// nautilus extension interface
//=============================================================================

//=============================================================================
// Initializing menu provider interface
void menuProviderIfaceInit(gpointer gIface, gpointer ifaceData) {
    g_print("pvtbox-nautilus-menu: menuProviderIfaceInit\n");
    NautilusMenuProviderIface* const iface = static_cast<NautilusMenuProviderIface*>(gIface);
    iface->get_file_items = getMenuItems;
}

// Initializing info provider interface
void infoProviderIfaceInit(gpointer gIface, gpointer ifaceData) {
    g_print("pvtbox-nautilus-menu: infoProviderIfaceInit\n");
    NautilusInfoProviderIface* const iface = static_cast<NautilusInfoProviderIface*>(gIface);
    iface->update_file_info = updateFileInfo;
}


void infoProviderIfaceFinalize(gpointer gIface, gpointer ifaceData)
{
    g_print("pvtbox-nautilus-menu: infoProviderIfaceFinalize\n");
}


//=============================================================================
// Initialize this extesion instance
void instanceInit(GTypeInstance* instance, gpointer gClass) {
    g_print("pvtbox-nautilus-menu: instanceInit\n");
}

//=============================================================================
// Initialize this extenstion class
void classInit(gpointer gClass, gpointer classData) {
    g_print("pvtbox-nautilus-menu: classInit\n");
}

//=============================================================================
// Module initialization
void EXPORT nautilus_module_initialize(GTypeModule* module) {

    g_print("pvtbox-nautilus-menu: nautilus_module_initialize\n");

    static const GTypeInfo info = {
        sizeof(PvtboxExtensionClass), // GBaseInitFunc
        NULL, // GBaseInitFunc base_init
        NULL, // GBaseFinalizeFunc base_finalize
        classInit, // GClassInitFunc  class_init
        NULL, // class_finalize
        NULL, // class_data
        sizeof(PvtboxExtension), // instance_size
        0, // n_preallocs
        instanceInit, // GInstanceInitFunc instance_init
        // const GTypeValueTable *value_table;
    };

    static const GInterfaceInfo menu_provider_iface_info = {
        menuProviderIfaceInit, // GInterfaceInitFunc interface_init
        NULL, // interface_finalize
        NULL // interface_data
    };

    static const GInterfaceInfo info_provider_iface_info = {
        infoProviderIfaceInit, // GInterfaceInitFunc interface_init
        infoProviderIfaceFinalize, // interface_finalize
        NULL // interface_data
    };

    extensionType = g_type_module_register_type(
        module, // GTypeModule *module
        G_TYPE_OBJECT, // GType parent_type
        extensionName, // const gchar *type_name
        &info, // const GTypeInfo *type_info
        GTypeFlags(0)); // GTypeFlags flags

    g_type_module_add_interface(
        module, // GTypeModule *module
        extensionType, // GType instance_type
        NAUTILUS_TYPE_MENU_PROVIDER, // GType interface_type
        &menu_provider_iface_info); // const GInterfaceInfo *interface_info

    g_type_module_add_interface(
        module, // GTypeModule *module
        extensionType, // GType instance_type
        NAUTILUS_TYPE_INFO_PROVIDER, // GType interface_type
        &info_provider_iface_info); // const GInterfaceInfo *interface_info

    bindtextdomain(gettextDomain, "/usr/share/locale");
    textdomain(gettextDomain);
}

//=============================================================================
// Module shutdown
void EXPORT nautilus_module_shutdown(void) {
    g_print("pvtbox-nautilus-menu: nautilus_module_shutdown\n");
}

//=============================================================================
// Get list of extesion types
void EXPORT nautilus_module_list_types(const GType** types, int* num_types) {
    *types = &extensionType;
    *num_types = 1;
}


//=============================================================================
void on_status_changed(std::string path, std::string status)
{
    g_print("pvtbox-nautilus-menu: %s -- %s\n", status.c_str(), path.c_str());

    GFile* g_file = g_file_new_for_path(path.c_str());
    if (!g_file) return;

    NautilusFileInfo* file = nautilus_file_info_create(g_file);
    if (!file) return;

    //reset emblems
    nautilus_file_info_invalidate_extension_info(file);

    //set emblems
    std::string overlays = "";
    if (status == "synced")
        overlays = "pvtbox-synced";
    else if (status == "syncing")
        overlays = "pvtbox-syncing";
    else if (status ==  "paused")
        overlays = "pvtbox-paused";
    else if (status == "error")
        overlays = "pvtbox-error";
    else if (status == "online")
    {
        if ( nautilus_file_info_is_directory (file) )
            overlays = "pvtbox-online";
    }


    if (overlays != "")
        nautilus_file_info_add_emblem(file, overlays.c_str());

    g_object_unref(g_file);
}
