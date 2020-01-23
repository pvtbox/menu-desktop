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
#include "Action.h"
}

#include "../common/PvtboxClientAPI.h"
#include "../common/gtk/FileInfo.h"
#include "CreateMenuItem.h"
#include <libintl.h>

const char* const activateMenuItemEventName = "activate";

extern "C" {
//=============================================================================
// The action class
struct _PvtboxActionClass {
    GtkActionClass __parent__;
};

//=============================================================================
// The action
struct _PvtboxAction {
    GtkAction __parent__;
    std::tr1::shared_ptr<GtkUtils::FileList> fileList;
    const MenuItemDescription* menuItem;
};
}

//=============================================================================
// Execute action
void execute_action(GtkAction* item,
                    PvtboxAction* dlAction) {
    GtkUtils::executeAction(dlAction->fileList->files(),
                            reinterpret_cast<GtkUtils::FileUriGetter>(thunarx_file_info_get_uri),
                            dlAction->menuItem->action);
}

//=============================================================================
//
GtkAction* createMenuItem(const MenuItemDescription& menuItem,
                          std::tr1::shared_ptr<GtkUtils::FileList> fileList) {
#define PARAM(name, value) name, value
    GtkAction* const action = reinterpret_cast<GtkAction*>(
        g_object_new(PVTBOX_TYPE_ACTION,
                     PARAM("hide-if-empty", FALSE),
                     PARAM("name", menuItem.name),
                     PARAM("label", gettext(menuItem.label)),
                     PARAM("tooltip", gettext(menuItem.tip)),
#if !GTK_CHECK_VERSION(2, 9, 0)
                     PARAM("stock-id", menuItem.iconName),
#else
                     PARAM("icon-name", menuItem.iconName),
#endif
                     NULL));
#undef PARAM

    PvtboxAction* pvtboxAction = PVTBOX_ACTION(action);
    pvtboxAction->fileList = fileList;
    pvtboxAction->menuItem = &menuItem;

    if (menuItem.action) {
        g_signal_connect_after(action,
                               activateMenuItemEventName,
                               G_CALLBACK(execute_action),
                               pvtboxAction);
    }

    return action;
}

extern "C" {
//=============================================================================
// Register action type
THUNARX_DEFINE_TYPE_EXTENDED(PvtboxAction, pvtbox_action, GTK_TYPE_ACTION, GTypeFlags(0), {})
}

//=============================================================================
// Build submenu
GtkWidget* create_submenu(GtkAction* action) {
    GtkWidget* const item = GTK_ACTION_CLASS(pvtbox_action_parent_class)->create_menu_item(action);
    try {
        PvtboxAction* dlAction = PVTBOX_ACTION(action);
        GList* const files = dlAction->fileList->files();
        if (!dlAction->menuItem->hasChildren() || !files)
            return item;

        GtkWidget* const menu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);

        std::vector<std::string> paths;
        GList *l;
        for (l = files; l != NULL; l = l->next)
        {
            GtkUtils::FileInfo file(thunarx_file_info_get_uri(
                reinterpret_cast<ThunarxFileInfo*>(l->data)));
            paths.push_back(file.fullPathUtf8());
        }

        const MenuDescription& subMenu = getMenu(paths,
                                                 dlAction->menuItem); // const MenuItemDescription* parent

        for (int i = 0; i < subMenu.size; ++i) {
            const MenuItemDescription& item = subMenu.items[i];
            GtkAction* const subAction = createMenuItem(item,
                                                        dlAction->fileList);
            GtkWidget* subItem = gtk_action_create_menu_item(subAction);
            gtk_widget_set_tooltip_text(subItem, gettext(dlAction->menuItem->tip));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), subItem);
        }
    } catch (const std::exception& e) {
        g_print("pvtbox: %s\n", e.what());
    }

    return item;
}

//=============================================================================
// Initialize action
void pvtbox_action_init(PvtboxAction* self) {
    self->menuItem = NULL;
}

//=============================================================================
// Finalize action
void pvtbox_action_finalize(GObject* object) {
    PVTBOX_ACTION(object)->fileList.reset();
    G_OBJECT_CLASS(pvtbox_action_parent_class)->finalize(object);
}

//=============================================================================
// Set action property
void pvtbox_action_set_property(GObject* object,
                                    guint property_id,
                                    const GValue* value,
                                    GParamSpec* pspec) {
}

//=============================================================================
// Initialize action class
void pvtbox_action_class_init(PvtboxActionClass* klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GtkActionClass* gtkaction_class = GTK_ACTION_CLASS(klass);

    gobject_class->finalize = pvtbox_action_finalize;
    gobject_class->set_property = pvtbox_action_set_property;

    gtkaction_class->create_menu_item = create_submenu;
}
