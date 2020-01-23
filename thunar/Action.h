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

typedef struct _PvtboxActionClass PvtboxActionClass;
typedef struct _PvtboxAction PvtboxAction;

#define PVTBOX_TYPE_ACTION (pvtbox_action_get_type())
#define PVTBOX_ACTION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PVTBOX_TYPE_ACTION, PvtboxAction))
#define PVTBOX_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), PVTBOX_TYPE_ACTION, PvtboxActionClass))
#define IS_PVTBOX_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), PVTBOX_TYPE_ACTION))
#define IS_PVTBOX_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PVTBOX_TYPE_ACTION))
#define PVTBOX_ACTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), PVTBOX_TYPE_ACTION, PvtboxActionClass))

GType pvtbox_action_get_type(void) G_GNUC_CONST G_GNUC_INTERNAL;
void pvtbox_action_register_type(ThunarxProviderPlugin*) G_GNUC_INTERNAL;
