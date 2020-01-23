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
#include "../common/Consts.h"
#include "Action.h"
#include "Provider.h"
#include <libintl.h>

//=============================================================================
// Static variables
static GType type_list[2];

//=============================================================================
// Exported functions
G_MODULE_EXPORT void thunar_extension_initialize(ThunarxProviderPlugin* plugin) {
    bindtextdomain(gettextDomain, "/usr/share/locale");
    textdomain(gettextDomain);

    menu_provider_register_type(plugin);
    pvtbox_action_register_type(plugin);
    type_list[0] = MENU_TYPE_PROVIDER;
    type_list[1] = PVTBOX_TYPE_ACTION;
}

G_MODULE_EXPORT void thunar_extension_shutdown(void) {
}

G_MODULE_EXPORT void thunar_extension_list_types(const GType** types,
                                                 gint* n_types) {
    *types = type_list;
    *n_types = G_N_ELEMENTS(type_list);
}
