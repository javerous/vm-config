/*
 *  SMVMwareNVRAM.h
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

#include "SMError.h"


/*
** Types
*/
#pragma mark - Types

// API.
typedef struct SMVMwareNVRAM 			SMVMwareNVRAM;
typedef struct SMVMwareNVRAMEntry		SMVMwareNVRAMEntry;
typedef struct SMVMwareNVRAMEFIVariable	SMVMwareNVRAMEFIVariable;

// EFI.
typedef struct {
	uint32_t  Data1;
	uint16_t  Data2;
	uint16_t  Data3;
	uint8_t   Data4[8];
} efi_guid_t;


/*
** Defines
*/
#pragma mark - Defines

// EFI.
#define EFI_VARIABLE_NON_VOLATILE							0x0000000000000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS						0x0000000000000002
#define EFI_VARIABLE_RUNTIME_ACCESS     					0x0000000000000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD					0x0000000000000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS				0x0000000000000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS	0x0000000000000020
#define EFI_VARIABLE_APPEND_WRITE							0x0000000000000040

// GUID.
#define SMVMwareGUIDStringSize 36


/*
** Types
*/
#pragma mark - Types

typedef enum
{
	SMVMwareNVRAMEntryTypeGeneric,
	SMVMwareNVRAMEntryTypeEFIVariables
} SMVMwareNVRAMEntryType;


/*
** Globals
*/
#pragma mark - Globals

extern const char * SMVMwareNVRAMErrorDomain;


/*
** Functions
*/
#pragma mark - Functions

// NVRAM.
// > Instance.
SMVMwareNVRAM *	SMVMwareNVRAMOpen(const char *nvram_file_path, SMError **error);
void			SMVMwareNVRAMFree(SMVMwareNVRAM *nvram);

// > Properties.
const char * SMVMwareNVRAMGetPath(SMVMwareNVRAM *nvram);

// > Serialization.
bool SMVMwareNVRAMWriteToFile(SMVMwareNVRAM *nvram, const char *path, SMError **error);

// > Entries.
size_t					SMVMwareNVRAMEntriesCount(SMVMwareNVRAM *nvram);
SMVMwareNVRAMEntry *	SMVMwareNVRAMGetEntryAtIndex(SMVMwareNVRAM *nvram, size_t idx);


// Entries.
// > Properties.
SMVMwareNVRAMEntryType	SMVMwareNVRAMEntryGetType(SMVMwareNVRAMEntry *entry);

const char *	SMVMwareNVRAMEntryGetName(SMVMwareNVRAMEntry *entry);
bool			SMVMwareNVRAMEntrySetName(SMVMwareNVRAMEntry *entry, const char *name, SMError **error);

const char *	SMVMwareNVRAMEntryGetSubname(SMVMwareNVRAMEntry *entry);
bool			SMVMwareNVRAMEntrySetSubname(SMVMwareNVRAMEntry *entry, const char *subname, SMError **error);

const void *	SMVMwareNVRAMEntryGetContentBytes(SMVMwareNVRAMEntry *entry, size_t *size);

// > Variables.
SMVMwareNVRAMEFIVariable *	SMVMwareNVRAMEntryAddVariable(SMVMwareNVRAMEntry *entry, efi_guid_t guid, uint32_t attributes, const char *utf8_name, const void *bytes, size_t size, SMError **error);

size_t						SMVMwareNVRAMEntryVariablesCount(SMVMwareNVRAMEntry *entry);
SMVMwareNVRAMEFIVariable *	SMVMwareNVRAMEntryGetVariableAtIndex(SMVMwareNVRAMEntry *entry, size_t idx);


// Variables.
efi_guid_t		SMVMwareNVRAMVariableGetGUID(SMVMwareNVRAMEFIVariable *variable);
void			SMVMwareNVRAMVariableSetGUID(SMVMwareNVRAMEFIVariable *variable, const efi_guid_t *guid);

uint32_t		SMVMwareNVRAMVariableGetAttributes(SMVMwareNVRAMEFIVariable *variable);
void			SMVMwareNVRAMVariableSetAttributes(SMVMwareNVRAMEFIVariable *variable, uint32_t attributes);

const void *	SMVMwareNVRAMVariableGetName(SMVMwareNVRAMEFIVariable *variable, size_t *size);
void			SMVMwareNVRAMVariableSetName(SMVMwareNVRAMEFIVariable *variable, const void *name, size_t size);

const void *	SMVMwareNVRAMVariableGetValue(SMVMwareNVRAMEFIVariable *variable, size_t *len);
void			SMVMwareNVRAMVariableSetValue(SMVMwareNVRAMEFIVariable *variable, const void *bytes, size_t size);

const char *	SMVMwareNVRAMVariableGetUTF8Name(SMVMwareNVRAMEFIVariable *variable, SMError **error);
bool			SMVMwareNVRAMVariableSetUTF8Name(SMVMwareNVRAMEFIVariable *variable, const char *utf8name, SMError **error);


// GUID.
bool SMVMwareNVRAMGUIDStringToGUID(const char *guid_str, efi_guid_t *guid, SMError **error);
void SMVMwareNVRAMGUIDToGUIDString(const efi_guid_t *guid, char *guid_str);
