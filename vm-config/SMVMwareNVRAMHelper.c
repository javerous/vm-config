/*
 *  SMVMwareNVRAMHelper.c
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

#include <string.h>

#include "SMVMwareNVRAMHelper.h"


/*
** Globals
*/
#pragma mark - Globals

static const efi_guid_t g_apple_nvram_variable_guid		= Apple_NVRAM_Variable_Guid;
static const efi_guid_t g_dhcpv6_service_binding_guid	= DHCPv6_Service_Binding_Guid;


/*
** Prototypes
*/
#pragma mark - Prototypes

static SMVMwareNVRAMEntry * 		SMVMwareNVRAMVariablesEntry(SMVMwareNVRAM *nvram, SMError **error);
static SMVMwareNVRAMEFIVariable *	SMVMwareNVRAMVariableForGUIDAndName(SMVMwareNVRAM *nvram, const efi_guid_t *guid, const char *name, SMError **error);


/*
** Functions
*/
#pragma mark - Functions

#pragma mark Interface

bool SMVMwareNVRAMGetAppleCSRActiveConfig(SMVMwareNVRAM *nvram, uint32_t *csr, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_apple_nvram_variable_guid, SMEFIAppleNVRAMVarCSRActiveConfigName, error);
	
	if (!var)
		return false;
	
	size_t		csr_size = 0;
	const void	*csr_bytes = SMVMwareNVRAMVariableGetValue(var, &csr_size);
	
	if (csr_size != sizeof(*csr))
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "csr value is not the right size (%lu)", csr_size);
		return false;
	}
	
	memcpy(csr, csr_bytes, sizeof(*csr));
	
	return true;
}

bool SMVMwareNVRAMSetAppleCSRActiveConfig(SMVMwareNVRAM *nvram, uint32_t csr, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_apple_nvram_variable_guid, SMEFIAppleNVRAMVarCSRActiveConfigName, error);
	
	if (var)
		SMVMwareNVRAMVariableSetValue(var, &csr, sizeof(csr));
	else
	{
		SMVMwareNVRAMEntry *entry = SMVMwareNVRAMVariablesEntry(nvram, error);

		if (!entry)
			return false;

		uint32_t attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
		
		return (SMVMwareNVRAMEntryAddVariable(entry, g_apple_nvram_variable_guid, attributes, SMEFIAppleNVRAMVarCSRActiveConfigName, &csr, sizeof(csr), error) != NULL);
	}
	
	return true;
}

bool SMVMwareNVRAMSetAppleCSRActivation(SMVMwareNVRAM *nvram, bool enable, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_apple_nvram_variable_guid, SMEFIAppleNVRAMVarCSRActiveConfigName, error);

	if (!var)
	{
		SMVMwareNVRAMEntry *entry = SMVMwareNVRAMVariablesEntry(nvram, error);

		if (!entry)
			return false;

		uint32_t attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
		uint32_t zero = 0;

		var = SMVMwareNVRAMEntryAddVariable(entry, g_apple_nvram_variable_guid, attributes, SMEFIAppleNVRAMVarCSRActiveConfigName, &zero, sizeof(zero), error);

		if (!var)
			return false;
	}
	
	size_t		csr_size = 0;
	const void	*csr_bytes = SMVMwareNVRAMVariableGetValue(var, &csr_size);
	
	if (csr_size != sizeof(uint32_t))
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "csr value is not the right size (%lu)", csr_size);
		return false;
	}
	
	const uint32_t	*csr_ptr = csr_bytes;
	uint32_t		csr = *csr_ptr;
	
	if (enable)
		csr &= ~CSR_STD_DISABLE_FLAGS | CSR_STD_ENABLE_FLAGS;
	else
		csr |= CSR_STD_DISABLE_FLAGS;
	
	SMVMwareNVRAMVariableSetValue(var, &csr, sizeof(csr));
	
	return true;
}

bool SMVMwareNVRAMGetApplePlatformUUID(SMVMwareNVRAM *nvram, uuid_t uuid, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_apple_nvram_variable_guid, SMEFIAppleNVRAMVarPlatformUUIDName, error);
	
	if (!var)
		return false;
	
	size_t		uuid_size = 0;
	const void	*uuid_bytes = SMVMwareNVRAMVariableGetValue(var, &uuid_size);
	
	if (uuid_size != sizeof(uuid_t))
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "plateform uuid is not the right size (%lu)", uuid_size);
		return false;
	}
	
	memcpy(uuid, uuid_bytes, sizeof(uuid_t));
	
	return true;
}

bool SMVMwareNVRAMSetApplePlatformUUID(SMVMwareNVRAM *nvram, uuid_t uuid, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_apple_nvram_variable_guid, SMEFIAppleNVRAMVarPlatformUUIDName, error);
	
	if (var)
		SMVMwareNVRAMVariableSetValue(var, uuid, sizeof(uuid_t));
	else
	{
		SMVMwareNVRAMEntry *entry = SMVMwareNVRAMVariablesEntry(nvram, error);

		if (!entry)
			return false;

		uint32_t attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

		return (SMVMwareNVRAMEntryAddVariable(entry, g_apple_nvram_variable_guid, attributes, SMEFIAppleNVRAMVarPlatformUUIDName, uuid, sizeof(uuid_t), error) != NULL);
	}

	return true;
}

bool SMVMwareNVRAMSetAppleMachineUUID(SMVMwareNVRAM *nvram, uuid_t uuid, SMError **error)
{
	// Set Platform UUID.
	if (!SMVMwareNVRAMSetApplePlatformUUID(nvram, uuid, error))
		return false;
	
	// Replace DHCPv6 ClientID (RFC 3315 DUID). It's okay if there is no such variable.
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_dhcpv6_service_binding_guid, SMEFIVarDHCPv6ClientIDName, error);

	if (!var)
		return true;

	// > On VMs I saw, the value is composed of 4 bytes + machine UUID.
	// > I have no idea what is the 4 bytes, it doesnt match DUID doc, and it doesn't seems to be a length, so it's perhaps some internal flags from VMware / Intel.
	// >
	// > We just reproduce the format there.
	uint8_t client_id[20] = { 0x12, 0x00, 0x00, 0x04 };
	
	memcpy(client_id + 4, uuid, sizeof(uuid_t));
	
	SMVMwareNVRAMVariableSetValue(var, client_id, sizeof(client_id));

	return true;
}

bool SMVMwareNVRAMSetBootArgs(SMVMwareNVRAM *nvram, const char *boot_args, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &g_apple_nvram_variable_guid, SMEFIAppleNVRAMVarBootArgsName, error);

	if (var)
		SMVMwareNVRAMVariableSetValue(var, boot_args, strlen(boot_args) + 1);
	else
	{
		SMVMwareNVRAMEntry *entry = SMVMwareNVRAMVariablesEntry(nvram, error);

		if (!entry)
			return false;

		uint32_t attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

		return (SMVMwareNVRAMEntryAddVariable(entry, g_apple_nvram_variable_guid, attributes, SMEFIAppleNVRAMVarBootArgsName, boot_args, strlen(boot_args) + 1, error) != NULL);
	}

	return true;
}


#pragma mark Helpers

static SMVMwareNVRAMEntry * SMVMwareNVRAMVariablesEntry(SMVMwareNVRAM *nvram, SMError **error)
{
	for (size_t i = 0; i < SMVMwareNVRAMEntriesCount(nvram); i++)
	{
		SMVMwareNVRAMEntry 		*entry = SMVMwareNVRAMGetEntryAtIndex(nvram, i);
		SMVMwareNVRAMEntryType	type = SMVMwareNVRAMEntryGetType(entry);

		if (type == SMVMwareNVRAMEntryTypeEFIVariables)
			return entry;
	}

	SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "EFI variables section not found");

	return NULL;
}


static SMVMwareNVRAMEFIVariable * SMVMwareNVRAMVariableForGUIDAndName(SMVMwareNVRAM *nvram, const efi_guid_t *guid, const char *name, SMError **error)
{
	SMVMwareNVRAMEntry *entry = SMVMwareNVRAMVariablesEntry(nvram, error);

	if (!entry)
		return NULL;

	// Search variable.
	for (size_t i = 0; i < SMVMwareNVRAMEntryVariablesCount(entry); i++)
	{
		SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMEntryGetVariableAtIndex(entry, i);

		// > Check GUID.
		efi_guid_t iguid = SMVMwareNVRAMVariableGetGUID(var);

		if (memcmp(&iguid, guid, sizeof(efi_guid_t)) != 0)
			continue;

		// > Check name.
		const char *iname = SMVMwareNVRAMVariableGetUTF8Name(var, NULL);

		if (!iname)
			continue;

		if (strcmp(iname, name) != 0)
			continue;

		return var;
	}

	SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "variable '%s' not found", name);
	
	return NULL;
}
