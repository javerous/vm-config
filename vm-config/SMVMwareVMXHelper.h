/*
 *  SMVMwareVMXHelper.h
 *
 *  Copyright 2022 Avérous Julien-Pierre
 *
 *  This file is part of vm-config.
 *
 *  vm-config is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vm-config is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vm-config.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <uuid/uuid.h>

#include "SMVMwareVMX.h"

#include "SMError.h"


/*
** Defines
*/
#pragma mark - Defines

#define SMVMwareVMXNVRAMFileKey		"nvram"

#define SMVMwareVMXUUIDBiosKey		"uuid.bios"
#define SMVMwareVMXUUIDLocationKey	"uuid.location"


/*
** Functions
*/
#pragma mark - Functions

bool SMVMwareVMXSetMachineUUID(SMVMwareVMX *vmx, uuid_t uuid, SMError **error);
