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

// This is the number that will end up in the version window of the DLLs.
// Increment this version before committing a new build if you are today's shell_integration build master.
#define EXT_BUILD_NUM 1

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define EXT_VERSION 1,0,0,EXT_BUILD_NUM
#define EXT_VERSION_STRING STRINGIZE(EXT_VERSION)
