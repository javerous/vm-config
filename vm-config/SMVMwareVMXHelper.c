/*
 *  SMVMwareVMXHelper.c
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

#include <stdio.h>

#include "SMVMwareVMXHelper.h"


/*
** Functions
*/
#pragma mark - Functions

#pragma mark Interface

bool SMVMwareVMXSetMachineUUID(SMVMwareVMX *vmx, uuid_t uuid, SMError **error)
{
	SMVMwareVMXEntry *uuid_bios_entry = SMVMwareVMXGetEntryForKey(vmx, SMVMwareVMXUUIDBiosKey);
	SMVMwareVMXEntry *uuid_location_entry = SMVMwareVMXGetEntryForKey(vmx, SMVMwareVMXUUIDLocationKey);

	// For now, just return. Consider to actually add the keys in the future.
	if (!uuid_bios_entry && !uuid_location_entry)
		return true;
	
	// Format UUID value for VMX.
	char	result[47 + 1] = { 0 };
	uint8_t *v = uuid;
	
	snprintf(result, sizeof(result), "%02x %02x %02x %02x %02x %02x %02x %02x-%02x %02x %02x %02x %02x %02x %02x %02x",
			 v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
	
	// Replace values.
	if (uuid_bios_entry)
	{
		if (!SMVMwareVMXEntrySetValue(uuid_bios_entry, result, error))
			return false;
	}
	
	if (uuid_location_entry)
	{
		if (!SMVMwareVMXEntrySetValue(uuid_location_entry, result, error))
			return false;
	}
	
	return true;
}
