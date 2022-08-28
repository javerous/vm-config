/*
 *  SMVMwareVMX.h
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

#include "SMError.h"


/*
** Types
*/
#pragma mark - Types

// API.
typedef struct SMVMwareVMX		SMVMwareVMX;
typedef struct SMVMwareVMXEntry	SMVMwareVMXEntry;

typedef enum
{
	SMVMwareVMXEntryTypeEmpty,
	SMVMwareVMXEntryTypeComment,
	SMVMwareVMXEntryTypeKeyValue,
} SMVMwareVMXEntryType;


/*
** Globals
*/
#pragma mark - Globals

extern const char * SMVMwareVMXErrorDomain;


/*
** Functions
*/
#pragma mark - Functions

// VMX.
// > Instance.
SMVMwareVMX *	SMVMwareVMXOpen(const char *vmx_file_path, SMError **error);
void			SMVMwareVMXFree(SMVMwareVMX *vmx);

// > Properties.
const char *	SMVMwareVMXGetPath(SMVMwareVMX *vmx);

// > Serialization.
bool SMVMwareVMXWriteToFile(SMVMwareVMX *vmx, const char *path, SMError **error);

// > Entries.
SMVMwareVMXEntry *	SMVMwareVMXAddEntryKeyValue(SMVMwareVMX *vmx, const char *key, const char *value, SMError **error);

size_t				SMVMwareVMXEntriesCount(SMVMwareVMX *vmx);
SMVMwareVMXEntry *	SMVMwareVMXGetEntryAtIndex(SMVMwareVMX *vmx, size_t idx);

SMVMwareVMXEntry *	SMVMwareVMXGetEntryForKey(SMVMwareVMX *vmx, const char *key);


// Entry.
// > Type.
SMVMwareVMXEntryType SMVMwareVMXEntryGetType(SMVMwareVMXEntry *entry);

// > Comment.
const char *	SMVMwareVMXEntryGetComment(SMVMwareVMXEntry *entry, SMError **error);
bool			SMVMwareVMXEntrySetComment(SMVMwareVMXEntry *entry, const char *comment, SMError **error);

// > Key-Value.
const char *	SMVMwareVMXEntryGetKey(SMVMwareVMXEntry *entry, SMError **error);
bool			SMVMwareVMXEntrySetKey(SMVMwareVMXEntry *entry, const char *key, SMError **error);

const char *	SMVMwareVMXEntryGetValue(SMVMwareVMXEntry *entry, SMError **error);
bool			SMVMwareVMXEntrySetValue(SMVMwareVMXEntry *entry, const char *value, SMError **error);
