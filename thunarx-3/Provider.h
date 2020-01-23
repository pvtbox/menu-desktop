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

#include <thunarx/thunarx.h>

typedef struct _MenuProviderClass MenuProviderClass;
typedef struct _MenuProvider MenuProvider;

#define MENU_TYPE_PROVIDER (menu_provider_get_type())
#define MENU_PROVIDER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MENU_TYPE_PROVIDER, MenuProvider))
#define MENU_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MENU_TYPE_PROVIDER, MenuProviderClass))
#define MENU_IS_PROVIDER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MENU_TYPE_PROVIDER))
#define MENU_IS_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MENU_TYPE_PROVIDER))
#define MENU_PROVIDER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MENU_TYPE_PROVIDER, MenuProviderClass))

GType menu_provider_get_type(void) G_GNUC_CONST G_GNUC_INTERNAL;
void menu_provider_register_type(ThunarxProviderPlugin* plugin) G_GNUC_INTERNAL;
