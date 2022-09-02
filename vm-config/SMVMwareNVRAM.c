/*
 *  SMVMwareNVRAM.c
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <iconv.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/param.h>

#if __has_include(<os/lock.h>)
#  include <os/lock.h>
#  define HAS_LOCK 1
#endif

#include "SMVMwareNVRAM.h"

#include "SMBytesWritter.h"


/*
** Defines
*/
#pragma mark - Defines

// Helpers.
#define SMSetParseErrorPtr(Ptr, Nvram, BytesPtr, UserInfo, ...) ({																		\
	SMError			**__smerror = (Ptr);																								\
	SMVMwareNVRAM	*__nvram = (Nvram); 																								\
	uintptr_t		__start_bytes = (uintptr_t)__nvram->bytes; 																			\
	uintptr_t		__parse_bytes = (uintptr_t)(BytesPtr); 																				\
																																		\
	if (__smerror)																														\
		*__smerror = SMErrorCreate(SMVMwareNVRAMErrorDomain, 42, "parsing error @0x%lu - " UserInfo, (__parse_bytes - __start_bytes), ##__VA_ARGS__);	\
})

// Round up or down. Round should be a power of 2.
#define SMRoundUp(Value, Round)		(((Value) + ((typeof(Value))(Round) - 1)) & ~((typeof(Value))(Round) - 1))
#define SMRoundDown(Value, Round)	((Value) & ~((typeof(Value))(Round) - 1))

// File.
#define SMFileMagic		{ 'M', 'R', 'V', 'N' }
#define SMEFINVMagic	{ 'V', 'M', 'W', 'N', 'V', 'R', 'A', 'M' }


/*
** Types
*/
#pragma mark - Types

// API.
struct SMVMwareNVRAM
{
	char *path;
	
	char	*bytes;
	size_t	size;
	
	uint32_t unknown_value;
	
	SMVMwareNVRAMEntry 	**entries;
	size_t				entries_cnt;
};

struct SMVMwareNVRAMEntry
{
	// Type.
	SMVMwareNVRAMEntryType type;

	// Updated entry.
	bool updated;
	
	// Properties.
	char	name[4];
	char	cname[5];

	char	subname[4];
	char	csubname[5];
	
	// Variables.
	SMVMwareNVRAMEFIVariable	**vars;
	size_t						vars_cnt;
	
	// Original bytes.
	const void	*original_bytes;
	size_t	original_size;
	
	const void	*original_content_bytes;
	size_t		original_content_size;
	
	// Serialization.
	void	*serialized_bytes;
	size_t	serialized_size;
};

struct SMVMwareNVRAMEFIVariable
{
	SMVMwareNVRAMEntry *parent_entry;
	
	// Updated variable.
	bool updated;
	
	// Properties.
	efi_guid_t	guid;
	uint32_t	attributes;
	char		*utf8_name;

	// Original bytes.
	const void	*original_bytes;
	size_t		original_size;
	
	const void	*original_name_bytes;
	size_t		original_name_size;
	
	const void	*original_value_bytes;
	size_t		original_value_size;

	// Serialization.
	void		*serialized_bytes;
	size_t		serialized_size;
	
	// Updated bytes.
	void		*updated_name_bytes;
	size_t		updated_name_size;
	
	void		*updated_value_bytes;
	size_t		updated_value_size;
};

// File.
typedef struct
{
	uint8_t		name[4];
	uint8_t		subname[4];
	uint32_t	len;
} __attribute__((packed)) nvram_entry_t;

typedef struct
{
	efi_guid_t	guid;
	uint32_t	attributes;
	uint32_t	data_size;
	uint32_t	name_size;
} __attribute__((packed)) efi_var_t;


/*
** Globals
*/
#pragma mark - Globals

// Errors.
const char * SMVMwareNVRAMErrorDomain = "com.sourcemac.vmware-nvram.error";


/*
** Prototypes
*/
#pragma mark - Prototypes

// NVRAM.
// > Entries.
static void SMVMwareNVRAMAddEntry(SMVMwareNVRAM *nvram, SMVMwareNVRAMEntry *entry);


// Entries.
// > Instance.
static SMVMwareNVRAMEntry *	SMVMwareNVRAMEntryCreateFromBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, SMError **error);
static void					SMVMwareNVRAMEntryFree(SMVMwareNVRAMEntry *entry);

// > Serialization.
static const void *	SMVMwareNVRAMEntryGetSerializedBytes(SMVMwareNVRAMEntry *entry, size_t *size);
static void			SMVMwareNVRAMEntryMarkUpdated(SMVMwareNVRAMEntry *entry);

// > Variables.
static void SMVMwareNVRAMEntryAddVariableInternal(SMVMwareNVRAMEntry *entry, SMVMwareNVRAMEFIVariable *var);


// Variables.
// > Instance.
static SMVMwareNVRAMEFIVariable *	SMVMwareNVRAMEFIVariableCreate(efi_guid_t guid, uint32_t attributes, const char *utf8_name, const void *value, size_t value_size, SMError **error);
static SMVMwareNVRAMEFIVariable *	SMVMwareNVRAMEFIVariableCreateFromBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, SMError **error);
static void							SMVMwareNVRAMEFIVariableFree(SMVMwareNVRAMEFIVariable *var);

// > Serialization.
static const void *	SMVMwareNVRAMVariableGetSerializedBytes(SMVMwareNVRAMEFIVariable *entry, size_t *size);
static void SMVMwareNVRAMVariableMarkUpdated(SMVMwareNVRAMEFIVariable *variable);


// Helpers.
// File.
static bool SMFileWriteBytes(int fd, const void *bytes, size_t len, SMError **error);

// Bytes.
// > Read.
static bool SMReadBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, void *output, size_t output_size, SMError **error);
static bool SMReadMatchingBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, const void *match_bytes, size_t match_size, SMError **error);

// > Misc.
static bool			SMIsBufferAscii(const uint8_t *buffer, size_t size, const char *ascii);
static const char *	SMBytesDescription(const void *bytes, size_t size);

// Strings.
static char * SMStringUTF16ToUTF8(const void *utf16bytes, size_t len);
static void * SMStringUTF8ToUTF16(const char *utf8str, bool terminal_zero, size_t *len);


/*
** NVRAM
*/
#pragma mark - NVRAM

#pragma mark > Instance

SMVMwareNVRAM * SMVMwareNVRAMOpen(const char *nvram_file_path, SMError **error)
{
	SMVMwareNVRAM *result = calloc(1, sizeof(SMVMwareNVRAM));
	
	assert(result);
	
	// Copy path.
	result->path = strdup(nvram_file_path);
	
	assert(result->path);
	
	// Open the file.
	int fd = open(nvram_file_path, O_RDONLY);
	
	if (fd == -1)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, errno, "can't open the file (%d - %s)", errno, strerror(errno));
		goto fail;
	}
		
	// Stat the file.
	struct stat st;
	
	if (fstat(fd, &st) == -1)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, errno, "can't stat the file (%d - %s)", errno, strerror(errno));
		close(fd);
		goto fail;
	}
	
	// Check size.
	if (st.st_size == 0)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, 0, "empty file");
		close(fd);
		goto fail;
	}
	
	// Map the file.
	// > Forge flags.
	int	flags = MAP_PRIVATE | MAP_FILE;

#if defined(MAP_RESILIENT_MEDIA)
	flags |= MAP_RESILIENT_MEDIA;
#endif

	// > Map.
	void	*mbytes = mmap(NULL, st.st_size, PROT_READ, flags, fd, 0);
	int		err = errno;
	
	close(fd);
	
	if (mbytes == MAP_FAILED)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, errno, "can't map the file (%d - %s)", err, strerror(err));
		goto fail;
	}
	
	// Hold parameters.
	result->bytes = mbytes;
	result->size = st.st_size;
	
	// Parse content.
	const void	*bytes = mbytes;
	size_t		size = st.st_size;
	
	// > Read magic.
	uint8_t magic[] = SMFileMagic;
	
	if (!SMReadMatchingBytes(result, &bytes, &size, magic, sizeof(magic), error))
		goto fail;

	// Read unknown field (version ?).
	uint32_t unknown;

	if (!SMReadBytes(result, &bytes, &size, &unknown, sizeof(unknown), error))
		goto fail;

	result->unknown_value = unknown;
	
	// Read entries.
	while (size)
	{
		SMVMwareNVRAMEntry *entry = SMVMwareNVRAMEntryCreateFromBytes(result, &bytes, &size, error);
		
		if (!entry)
			goto fail;
		
		SMVMwareNVRAMAddEntry(result, entry);
	}
	
	// Return.
	return result;
	
fail:
	SMVMwareNVRAMFree(result);
	return NULL;
}

void SMVMwareNVRAMFree(SMVMwareNVRAM *nvram)
{
	if (!nvram)
		return;
	
	free(nvram->path);
	
	// Free entries.
	for (size_t i = 0; i < nvram->entries_cnt; i++)
		SMVMwareNVRAMEntryFree(nvram->entries[i]);
	
	free(nvram->entries);
	
	// Unmap bytes.
	if (nvram->bytes)
		munmap(nvram->bytes, nvram->size);
	
	// Free root.
	free(nvram);
}


#pragma mark > Properties

const char * SMVMwareNVRAMGetPath(SMVMwareNVRAM *nvram)
{
	return nvram->path;
}


#pragma mark > Serialization

bool SMVMwareNVRAMWriteToFile(SMVMwareNVRAM *nvram, const char *path, SMError **error)
{
	// Open file.
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	
	if (fd == -1)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, errno, "can't create the file (%d - %s)", errno, strerror(errno));
		goto fail;
	}
	
	// Write magic.
	uint8_t magic[] = SMFileMagic;
	
	if (!SMFileWriteBytes(fd, magic, sizeof(magic), error))
		goto fail;
	
	// Write unknown value.
	if (!SMFileWriteBytes(fd, &nvram->unknown_value, sizeof(nvram->unknown_value), error))
		goto fail;
	
	// Write entries.
	size_t entries_count = SMVMwareNVRAMEntriesCount(nvram);
	
	for (size_t i = 0; i < entries_count; i++)
	{
		SMVMwareNVRAMEntry *entry = SMVMwareNVRAMGetEntryAtIndex(nvram, i);
		
		size_t		bytes_size = 0;
		const void	*bytes = SMVMwareNVRAMEntryGetSerializedBytes(entry, &bytes_size);
		
		// Write.
		if (!SMFileWriteBytes(fd, bytes, bytes_size, error))
			goto fail;
	}
	
	// Close.
	close(fd);
	
	return true;
	
fail:
	if (fd >= 0)
		close(fd);
	
	unlink(path);
	
	return false;
}


#pragma mark > Entries

static void SMVMwareNVRAMAddEntry(SMVMwareNVRAM *nvram, SMVMwareNVRAMEntry *entry)
{
	nvram->entries = reallocf(nvram->entries, (nvram->entries_cnt + 1) * sizeof(*nvram->entries));
	
	assert(nvram->entries);
	
	nvram->entries[nvram->entries_cnt] = entry;
	nvram->entries_cnt++;
}

size_t SMVMwareNVRAMEntriesCount(SMVMwareNVRAM *nvram)
{
	return nvram->entries_cnt;
}

SMVMwareNVRAMEntry * SMVMwareNVRAMGetEntryAtIndex(SMVMwareNVRAM *nvram, size_t idx)
{
	assert(idx < nvram->entries_cnt);
	
	return nvram->entries[idx];
}


/*
** Entries
*/
#pragma mark - Entries

#pragma mark > Instance

static SMVMwareNVRAMEntry *	SMVMwareNVRAMEntryCreateFromBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, SMError **error)
{
	// Create instance.
	SMVMwareNVRAMEntry *entry = calloc(1, sizeof(SMVMwareNVRAMEntry));
	
	assert(entry);
	
	// Parse.
	const void *bytes_bck = *bytes;

	// > Read entry header.
	nvram_entry_t nvram_entry;
	
	if (!SMReadBytes(nvram, bytes, size, &nvram_entry, sizeof(nvram_entry), error))
		goto fail;

	// Check size.
	if (nvram_entry.len > *size)
	{
		SMSetParseErrorPtr(error, nvram, bytes, "entry too big (%u)", nvram_entry.len);
		goto fail;
	}
	
	// > Create entry.
	memcpy(entry->name, nvram_entry.name, sizeof(entry->name));
	memcpy(entry->subname, nvram_entry.subname, sizeof(entry->subname));
	
	memcpy(entry->cname, nvram_entry.name, sizeof(nvram_entry.name));
	memcpy(entry->csubname, nvram_entry.subname, sizeof(nvram_entry.subname));
	
	entry->original_bytes = bytes_bck;
	entry->original_size = sizeof(nvram_entry) + nvram_entry.len;
	
	entry->original_content_bytes = *bytes;
	entry->original_content_size = nvram_entry.len;

	// > Parse EFI NVRAM.
	if (SMIsBufferAscii(nvram_entry.name, sizeof(nvram_entry.name), "EFI_") && SMIsBufferAscii(nvram_entry.subname, sizeof(nvram_entry.subname), "NV"))
	{
		const void 	*innerBytes = *bytes;
		size_t		innerSize = *size;

		// > Mark type.
		entry->type = SMVMwareNVRAMEntryTypeEFIVariables;
		
		// > Read magic.
		uint8_t magic[] = SMEFINVMagic;
		
		if (!SMReadMatchingBytes(nvram, &innerBytes, &innerSize, magic, sizeof(magic), error))
			goto fail;
		
		// > Read zero.
		uint32_t zero = 0;
		
		if (!SMReadMatchingBytes(nvram, &innerBytes, &innerSize, &zero, sizeof(zero), error))
			goto fail;

		// > Read data size.
		uint32_t contentSize = 0;

		bytes_bck = innerBytes;
		
		if (!SMReadBytes(nvram, &innerBytes, &innerSize, &contentSize, sizeof(contentSize), error))
			goto fail;
		
		// > Check sizes.
		if (contentSize > nvram_entry.len)
		{
			SMSetParseErrorPtr(error, nvram, bytes_bck, "found an EFI_NV data too huge (%u > %u)", contentSize, nvram_entry.len);
			goto fail;
		}

		if (contentSize < sizeof(magic) + sizeof(zero))
		{
			SMSetParseErrorPtr(error, nvram, bytes_bck, "found an EFI_NV data too small (%u < %lu)", contentSize, sizeof(magic) + sizeof(zero));
			goto fail;
		}
		
		// > Skip headers size from content size.
		contentSize -= sizeof(magic) + sizeof(zero) + sizeof(contentSize);
		
		// > Parse variables.
		size_t lContentSize = contentSize;
		
		while (lContentSize)
		{
			SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMEFIVariableCreateFromBytes(nvram, &innerBytes, &lContentSize, error);
			
			if (!var)
				goto fail;
			
			SMVMwareNVRAMEntryAddVariableInternal(entry, var);
		}
	}
	else
		entry->type = SMVMwareNVRAMEntryTypeGeneric;
	
	// Consumed bytes.
	*bytes += nvram_entry.len;
	*size -= nvram_entry.len;
	
	// Return entry.
	return entry;
	
fail:
	SMVMwareNVRAMEntryFree(entry);
	return NULL;
}

static void SMVMwareNVRAMEntryFree(SMVMwareNVRAMEntry *entry)
{
	if (!entry)
		return;
	
	// Variables.
	for (size_t i = 0; i < entry->vars_cnt; i++)
		SMVMwareNVRAMEFIVariableFree(entry->vars[i]);
	
	free(entry->vars);
	
	// Serialization.
	free(entry->serialized_bytes);
	
	// Free root.
	free(entry);
}


#pragma mark > Serialization

static const void * SMVMwareNVRAMEntryGetSerializedBytes(SMVMwareNVRAMEntry *entry, size_t *size)
{
	// Return cached serialized bytes.
	if (entry->serialized_bytes)
	{
		if (size)
			*size = entry->serialized_size;
		
		return entry->serialized_bytes;
	}
	
	// Return original bytes if the entry wasn't updated.
	if (!entry->updated)
	{
		if (size)
			*size = entry->original_size;
		
		return entry->original_bytes;
	}
	
	// Serialize bytes & return them.
	SMBytesWritter writter = SMBytesWritterInit();
	
	// > Write header.
	off_t 			nvram_entry_offset = SMBytesWritterAppendSpace(&writter, sizeof(nvram_entry_t));
	nvram_entry_t	*hdr = SMBytesWritterPtrOff(nvram_entry_t, &writter, nvram_entry_offset);
	
	memcpy(hdr->name, entry->name, sizeof(hdr->name));
	memcpy(hdr->subname, entry->subname, sizeof(hdr->subname));
	hdr->len = 0;

	// > Write variables.
	size_t cnt = SMVMwareNVRAMEntryVariablesCount(entry);

	if (cnt > 0)
	{
		size_t data_size = 0;
		size_t content_size = 0;
		
		// > Write magic.
		uint8_t magic[] = SMEFINVMagic;
		
		SMBytesWritterAppendBytes(&writter, magic, sizeof(magic));
		
		// > Write zero.
		SMBytesWritterAppendRepeatedByte(&writter, 0, 4);
		
		// > Write size (zero for now).
		off_t data_size_offset = SMBytesWritterAppendRepeatedByte(&writter, 0, 4);
		
		// > Writes variables.
		for (size_t i = 0; i < cnt; i++)
		{
			SMVMwareNVRAMEFIVariable *variable = SMVMwareNVRAMEntryGetVariableAtIndex(entry, i);
			
			size_t 		var_size = 0;
			const void	*var_bytes = SMVMwareNVRAMVariableGetSerializedBytes(variable, &var_size);
			
			SMBytesWritterAppendBytes(&writter, var_bytes, var_size);
			
			data_size += var_size;
		}
		
		data_size += sizeof(magic) + 4 + 4; // magic + sizeof(zero) + sizeof(data_size).
		
		// > Compute & update content size. XXX: I'm not sure if 0x40000 is a maxium, or a "block" size. Consider it's a block size for now.
		nvram_entry_t *update_hdr = SMBytesWritterPtrOff(nvram_entry_t, &writter, nvram_entry_offset);

		content_size = SMRoundUp(data_size, 0x40000);
		update_hdr->len = (uint32_t)content_size;
		
		SMBytesWritterAppendRepeatedByte(&writter, 0xff, content_size - data_size);
		
		// > Update data size.
		uint32_t *data_size_ptr = SMBytesWritterPtrOff(uint32_t, &writter, data_size_offset);
		
		*data_size_ptr = (uint32_t)data_size;
	}
	
	// > Hold result.
	entry->serialized_bytes = writter.bytes;
	entry->serialized_size = writter.size;
	
	// Result.
	if (size)
		*size = writter.size;
	
	return writter.bytes;
}

static void SMVMwareNVRAMEntryMarkUpdated(SMVMwareNVRAMEntry *entry)
{
	entry->updated = true;
	
	if (entry->serialized_bytes)
	{
		free(entry->serialized_bytes);
		entry->serialized_bytes = NULL;
		
		entry->serialized_size = 0;
	}
}


#pragma mark > Properties

SMVMwareNVRAMEntryType SMVMwareNVRAMEntryGetType(SMVMwareNVRAMEntry *entry)
{
	return entry->type;
}

const char * SMVMwareNVRAMEntryGetName(SMVMwareNVRAMEntry *entry)
{
	return entry->cname;
}

bool SMVMwareNVRAMEntrySetName(SMVMwareNVRAMEntry *entry, const char *name, SMError **error)
{
	size_t len = strlen(name);
	
	if (len > 4)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "name is too long");
		return false;
	}
	
	memset(entry->name, 0, sizeof(entry->name));
	memset(entry->cname, 0, sizeof(entry->cname));
	
	memcpy(entry->name, name, len);
	memcpy(entry->cname, name, len);
	
	SMVMwareNVRAMEntryMarkUpdated(entry);
	
	return true;
}

const char * SMVMwareNVRAMEntryGetSubname(SMVMwareNVRAMEntry *entry)
{
	return entry->csubname;
}

bool SMVMwareNVRAMEntrySetSubname(SMVMwareNVRAMEntry *entry, const char *subname, SMError **error)
{
	size_t len = strlen(subname);
	
	if (len > 4)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "subname is too long");
		return false;
	}
	
	memset(entry->subname, 0, sizeof(entry->subname));
	memset(entry->csubname, 0, sizeof(entry->csubname));
	
	memcpy(entry->subname, subname, len);
	memcpy(entry->csubname, subname, len);
	
	SMVMwareNVRAMEntryMarkUpdated(entry);
	
	return true;
}

const void * SMVMwareNVRAMEntryGetContentBytes(SMVMwareNVRAMEntry *entry, size_t *size)
{
	if (size)
		*size = entry->original_content_size;
	
	return entry->original_content_bytes;
}


#pragma mark > Variables

SMVMwareNVRAMEFIVariable * SMVMwareNVRAMEntryAddVariable(SMVMwareNVRAMEntry *entry, efi_guid_t guid, uint32_t attributes, const char *utf8_name, const void *bytes, size_t size, SMError **error)
{
	// Create instance.
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMEFIVariableCreate(guid, attributes, utf8_name, bytes, size, error);

	if (!var)
		return NULL;

	// Add to entries.
	SMVMwareNVRAMEntryAddVariableInternal(entry, var);
	SMVMwareNVRAMEntryMarkUpdated(entry);

	// Return added entry.
	return var;
}

static void SMVMwareNVRAMEntryAddVariableInternal(SMVMwareNVRAMEntry *entry, SMVMwareNVRAMEFIVariable *var)
{
	// Append to array.
	entry->vars = reallocf(entry->vars, (entry->vars_cnt + 1) * sizeof(*entry->vars));
	
	assert(entry->vars);
	
	entry->vars[entry->vars_cnt] = var;
	entry->vars_cnt++;

	// Link the variable to us.
	var->parent_entry = entry;
}

size_t SMVMwareNVRAMEntryVariablesCount(SMVMwareNVRAMEntry *entry)
{
	return entry->vars_cnt;
}

SMVMwareNVRAMEFIVariable * SMVMwareNVRAMEntryGetVariableAtIndex(SMVMwareNVRAMEntry *entry, size_t idx)
{
	assert(idx < entry->vars_cnt);
	
	return entry->vars[idx];
}


/*
** Variables
*/
#pragma mark - Variables

#pragma mark > Instance

static SMVMwareNVRAMEFIVariable * SMVMwareNVRAMEFIVariableCreate(efi_guid_t guid, uint32_t attributes, const char *utf8_name, const void *value, size_t value_size, SMError **error)
{
	// Create instance.
	SMVMwareNVRAMEFIVariable *var = calloc(1, sizeof(SMVMwareNVRAMEFIVariable));

	assert(var);

	// Fill content.
	if (!SMVMwareNVRAMVariableSetUTF8Name(var, utf8_name, error))
		goto fail;

	SMVMwareNVRAMVariableSetAttributes(var, attributes);

	SMVMwareNVRAMVariableSetGUID(var, &guid);

	SMVMwareNVRAMVariableSetValue(var, value, value_size);

	return var;

fail:
	SMVMwareNVRAMEFIVariableFree(var);
	return NULL;
}

static SMVMwareNVRAMEFIVariable * SMVMwareNVRAMEFIVariableCreateFromBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, SMError **error)
{
	SMVMwareNVRAMEFIVariable *var = calloc(1, sizeof(SMVMwareNVRAMEFIVariable));
	
	assert(var);

	// Parse.
	const void *bytes_bck = *bytes;
	
	// > Read var header.
	efi_var_t efi_var;
	
	if (!SMReadBytes(nvram, bytes, size, &efi_var, sizeof(efi_var), error))
		goto fail;
	
	// > Check var data size.
	if (efi_var.data_size > *size)
	{
		SMSetParseErrorPtr(error, nvram, bytes_bck, "an EFI var is too huge (%u > %lu)", efi_var.data_size, *size);
		goto fail;
	}
	
	// > Check name data size.
	if (efi_var.name_size > efi_var.data_size)
	{
		SMSetParseErrorPtr(error, nvram, bytes_bck, "an EFI var name is too huge (%u > %u)", efi_var.name_size, efi_var.data_size);
		goto fail;
	}
	
	// Fill variable.
	memcpy(&var->guid, &efi_var.guid, sizeof(efi_guid_t));
	var->attributes = efi_var.attributes;
	
	var->original_bytes = bytes_bck;
	var->original_size = sizeof(efi_var) + efi_var.data_size;
	
	var->original_name_bytes = *bytes;
	var->original_name_size = efi_var.name_size;
	
	var->original_value_bytes = *bytes + efi_var.name_size;
	var->original_value_size = efi_var.data_size - efi_var.name_size;
	
	// Consumed bytes.
	*bytes += efi_var.data_size;
	*size -= efi_var.data_size;
	
	// Result.
	return var;
	
fail:
	SMVMwareNVRAMEFIVariableFree(var);
	return NULL;
}

static void SMVMwareNVRAMEFIVariableFree(SMVMwareNVRAMEFIVariable *var)
{
	if (!var)
		return;
	
	free(var->utf8_name);
	
	free(var->serialized_bytes);
	free(var->updated_name_bytes);
	free(var->updated_value_bytes);
	
	free(var);
}


#pragma mark > Serialization

static const void *	SMVMwareNVRAMVariableGetSerializedBytes(SMVMwareNVRAMEFIVariable *variable, size_t *size)
{
	// Return cached serialized bytes.
	if (variable->serialized_bytes)
	{
		if (size)
			*size = variable->serialized_size;
		
		return variable->serialized_bytes;
	}
	
	// Return original bytes if the variable wasn't updated.
	if (!variable->updated)
	{
		if (size)
			*size = variable->original_size;
		
		return variable->original_bytes;
	}
	
	// Serialize bytes & return them.
	SMBytesWritter writter = { 0 };
	
	// > Fetch content.
	size_t		name_size = 0;
	const void	*name_bytes = SMVMwareNVRAMVariableGetName(variable, &name_size);
	
	size_t		value_size = 0;
	const void	*value_bytes = SMVMwareNVRAMVariableGetValue(variable, &value_size);
	
	// > Header.
	off_t 		efi_var_offset = SMBytesWritterAppendSpace(&writter, sizeof(efi_var_t));
	efi_var_t	*efi_var = SMBytesWritterPtrOff(efi_var_t, &writter, efi_var_offset);
	
	memcpy(&efi_var->guid, &variable->guid, sizeof(efi_guid_t));
	efi_var->attributes = variable->attributes;
	efi_var->data_size = (uint32_t)name_size + (uint32_t)value_size;
	efi_var->name_size = (uint32_t)name_size;
	
	// > Content.
	SMBytesWritterAppendBytes(&writter, name_bytes, name_size);
	SMBytesWritterAppendBytes(&writter, value_bytes, value_size);

	// > Hold result.
	variable->serialized_bytes = writter.bytes;
	variable->serialized_size = writter.size;
	
	// Result.
	if (size)
		*size = writter.size;
	
	return writter.bytes;
}

static void SMVMwareNVRAMVariableMarkUpdated(SMVMwareNVRAMEFIVariable *variable)
{
	// Flag as updated.
	variable->updated = true;
	
	// Free serialized bytes.
	if (variable->serialized_bytes)
	{
		free(variable->serialized_bytes);
		variable->serialized_bytes = NULL;
		
		variable->serialized_size = 0;
	}
	
	// Mark entry as updated.
	if (variable->parent_entry)
		SMVMwareNVRAMEntryMarkUpdated(variable->parent_entry);
}


#pragma mark > Properties

efi_guid_t SMVMwareNVRAMVariableGetGUID(SMVMwareNVRAMEFIVariable *variable)
{
	return variable->guid;
}

void SMVMwareNVRAMVariableSetGUID(SMVMwareNVRAMEFIVariable *variable, const efi_guid_t *guid)
{
	memcpy(&variable->guid, guid, sizeof(efi_guid_t));
	SMVMwareNVRAMVariableMarkUpdated(variable);
}

uint32_t SMVMwareNVRAMVariableGetAttributes(SMVMwareNVRAMEFIVariable *variable)
{
	return variable->attributes;
}

void SMVMwareNVRAMVariableSetAttributes(SMVMwareNVRAMEFIVariable *variable, uint32_t attributes)
{
	variable->attributes = attributes;
	SMVMwareNVRAMVariableMarkUpdated(variable);
}

const void * SMVMwareNVRAMVariableGetName(SMVMwareNVRAMEFIVariable *variable, size_t *size)
{
	if (variable->updated_name_bytes)
	{
		if (size)
			*size = variable->updated_name_size;
		
		return variable->updated_name_bytes;
	}
	else
	{
		if (size)
			*size = variable->original_name_size;
		
		return variable->original_name_bytes;
	}
}

void SMVMwareNVRAMVariableSetName(SMVMwareNVRAMEFIVariable *variable, const void *name, size_t size)
{
	// Flush UTF-8 string.
	free(variable->utf8_name);
	variable->utf8_name = NULL;
	
	// Free previous name.
	if (variable->updated_name_bytes)
		free(variable->updated_name_bytes);
	
	// Copy new name.
	variable->updated_name_bytes = malloc(size);
	
	assert(variable->updated_name_bytes);
	
	memcpy(variable->updated_name_bytes, name, size);
	variable->updated_name_size = size;
	
	// Mark as updated.
	SMVMwareNVRAMVariableMarkUpdated(variable);
}

const void * SMVMwareNVRAMVariableGetValue(SMVMwareNVRAMEFIVariable *variable, size_t *size)
{
	if (variable->updated_value_bytes)
	{
		if (size)
			*size = variable->updated_value_size;
		
		return variable->updated_value_bytes;
	}
	else
	{
		if (size)
			*size = variable->original_value_size;
		
		return variable->original_value_bytes;
	}
}

void SMVMwareNVRAMVariableSetValue(SMVMwareNVRAMEFIVariable *variable, const void *bytes, size_t size)
{
	// Free previous value.
	if (variable->updated_value_bytes)
		free(variable->updated_value_bytes);
	
	// Copy new value.
	variable->updated_value_bytes = malloc(size);
	
	assert(variable->updated_value_bytes);
	
	memcpy(variable->updated_value_bytes, bytes, size);
	variable->updated_value_size = size;
	
	// Mark as updated.
	SMVMwareNVRAMVariableMarkUpdated(variable);
}

const char * SMVMwareNVRAMVariableGetUTF8Name(SMVMwareNVRAMEFIVariable *variable, SMError **error)
{
	if (variable->utf8_name)
		return variable->utf8_name;

	size_t		name_len = 0;
	const void	*name_bytes = SMVMwareNVRAMVariableGetName(variable, &name_len);
	
	variable->utf8_name = SMStringUTF16ToUTF8(name_bytes, name_len);
	
	if (!variable->utf8_name)
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "unable to convert UTF-16 to UTF-8");
	
	return variable->utf8_name;
}

bool SMVMwareNVRAMVariableSetUTF8Name(SMVMwareNVRAMEFIVariable *variable, const char *utf8name, SMError **error)
{
	// Convert to UTF-16.
	size_t	utf16_len = 0;
	void	*utf16_bytes = SMStringUTF8ToUTF16(utf8name, true, &utf16_len);
	
	if (!utf16_bytes)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "unable to convert UTF-8 to UTF1-6");
		return false;
	}
	
	// Store UTF-16 bversion.
	free(variable->updated_name_bytes);
	variable->updated_name_bytes = utf16_bytes;
	variable->updated_name_size = utf16_len;
	
	// Store UTF-8 version.
	free(variable->utf8_name);
	variable->utf8_name = strdup(utf8name);
	
	assert(variable->utf8_name);
	
	// Mark as updated.
	SMVMwareNVRAMVariableMarkUpdated(variable);

	return true;
}


/*
** GUID
*/
#pragma mark - GUID

bool SMVMwareNVRAMGUIDStringToGUID(const char *guid_str, efi_guid_t *guid, SMError **error)
{
	// Check len.
	if (strlen(guid_str) != SMVMwareGUIDStringSize)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "invalid string size");
		return false;
	}
	
	// Scan components.
	unsigned int	values[11];
	int				sresult = sscanf(guid_str, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], &values[6], &values[7], &values[8], &values[9], &values[10]);
	
	if (sresult != 11)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "invalid number of components");
		return false;
	}
	
	// Fill structure.
	efi_guid_t result;

	result.Data1 = values[0];
	
	if (values[1] > UINT16_MAX || values[2] > UINT16_MAX)
	{
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "invalid scanned word");
		return false;
	}
	
	result.Data2 = values[1];
	result.Data3 = values[2];
	
	for (size_t i = 0; i < 8; i++)
	{
		unsigned int val = values[i + 3];
		
		if (val > (unsigned int)UINT8_MAX)
		{
			SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, -1, "invalid scanned byte");
			return false;
		}
		
		result.Data4[i] = (uint8_t)val;
	}

	// Give baack result.
	memcpy(guid, &result, sizeof(efi_guid_t));

	return true;
}

void SMVMwareNVRAMGUIDToGUIDString(const efi_guid_t *guid, char *guid_str)
{
	snprintf(guid_str, SMVMwareGUIDStringSize + 1, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}


/*
** Helpers
*/
#pragma mark - Helpers

#pragma mark File

static bool SMFileWriteBytes(int fd, const void *bytes, size_t len, SMError **error)
{
	if (write(fd, bytes, len) != len)
	{
		int err_bck = errno;
		
		SMSetErrorPtr(error, SMVMwareNVRAMErrorDomain, errno, "can't write bytes %s (%d - %s)", SMBytesDescription(bytes, len), err_bck, strerror(err_bck));
		
		return false;
	}
	
	return true;
}


#pragma mark Bytes

#pragma mark > Read

static bool SMReadBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, void *output, size_t output_size, SMError **error)
{
	if (*size < output_size)
	{
		SMSetParseErrorPtr(error, nvram, *bytes, "need to read %lu bytes but only %lu bytes are available", output_size, *size);
		return false;
	}
	
	memcpy(output, *bytes, output_size);

	*bytes += output_size;
	*size -= output_size;

	return true;
}

static bool SMReadMatchingBytes(SMVMwareNVRAM *nvram, const void **bytes, size_t *size, const void *match_bytes, size_t match_size, SMError **error)
{
	// Check size remaining.
	if (*size < match_size)
	{
		SMSetParseErrorPtr(error, nvram, *bytes, "need to match %lu bytes (%s) but only %lu bytes are available", match_size, SMBytesDescription(match_bytes, match_size), *size);
		return false;
	}
	
	// Compare.
	if (memcmp(*bytes, match_bytes, match_size) != 0)
	{
		if (error)
		{
			char *desc_match = strdup(SMBytesDescription(match_bytes, match_size));
			char *desc_bytes = strdup(SMBytesDescription(*bytes, match_size));

			SMSetParseErrorPtr(error, nvram, *bytes, "expected %s bytes but got %s", desc_match, desc_bytes);
			
			free(desc_match);
			free(desc_bytes);
		}
		return false;
	}
	
	// Update pointers.
	*bytes += match_size;
	*size -= match_size;
	
	// Success.
	return true;
}


#pragma mark > Misc

static bool SMIsBufferAscii(const uint8_t *buffer, size_t size, const char *ascii)
{
	size_t ascii_len = strlen(ascii);

	if (ascii_len > size)
		return false;
	
	return (memcmp(buffer, ascii, ascii_len) == 0);
}

static const char * SMBytesDescription(const void *bytes, size_t size)
{
#define	max_size 5
#define	max_result_size ((max_size * 2) + ((max_size - 1)) + 3 + 2 + 1)

	static char buffer[max_result_size];

	if (size == 0)
		return "";

	size_t	final_size = MIN(size, max_size);
	char	*result = buffer;
	size_t	rsize = max_result_size;
	
	for (size_t i = 0; i < final_size; i++)
	{
		int sz = snprintf(result, rsize, "%02x", ((uint8_t *)bytes)[i]);
		
		if (sz > 0)
		{
			result += sz;
			rsize -= sz;
		}
		
		if (i + 1 != final_size)
		{
			*result = ' ';
			result++;
			rsize--;
		}
	}
	
	*result = '\0';
	
	if (size > max_size)
	{
		strlcpy(result, "...", rsize);
		
		result += 3;
		rsize -= 3;
		
		snprintf(result, rsize, "%02x", ((uint8_t *)bytes)[size - 1]);
	}

	return buffer;
}


#pragma mark Strings

static char * SMStringUTF16ToUTF8(const void *utf16bytes, size_t len)
{
	char *result = NULL;
	
	// Create converter.
#ifdef HAS_LOCK
	static os_unfair_lock	lock = OS_UNFAIR_LOCK_INIT;
	static iconv_t			conv = NULL;

	os_unfair_lock_lock(&lock);
	
#else
	iconv_t conv = NULL;
#endif
	
	if (!conv)
		conv = iconv_open("UTF-8", "UTF-16LE");
	
	if (!conv)
		goto finish;
	
	// Convert.
	size_t	strInputSize = len;
	char	*strInput = (char *)utf16bytes;

	size_t	strResultLenMax = strInputSize * 3 + 1;
	char	*strResultBuffer = malloc(strResultLenMax);
	char	*strResult = strResultBuffer;
	
	assert(strResultBuffer);

	if (iconv(conv, (char **)&strInput, &strInputSize, &strResult, &strResultLenMax) == (size_t)(-1))
		goto finish;
		
	// Add terminal zero.
	if (strResultLenMax < 1)
	{
		free(strResultBuffer);
		result = NULL;
		goto finish;
	}
	
	*strResult = 0;
	
	// Finish.
	result = strResultBuffer;
	
finish:
#ifdef HAS_LOCK
	iconv(conv, NULL, NULL, NULL, NULL);
	os_unfair_lock_unlock(&lock);
#else
	iconv_close(conv);
#endif
	
	return result;
}

static void * SMStringUTF8ToUTF16(const char *utf8str, bool terminal_zero, size_t *len)
{
	char *result = NULL;
	
	// Create converter.
#ifdef HAS_LOCK
	static os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
	static iconv_t conv = NULL;

	os_unfair_lock_lock(&lock);
	
#else
	iconv_t conv = NULL;
#endif
	
	if (!conv)
		conv = iconv_open("UTF-16LE", "UTF-8");
	
	if (!conv)
		goto finish;
	
	// Convert.
	size_t	strInputSize = strlen(utf8str);
	char	*strInput = (char *)utf8str;

	size_t	strResultLenMax = strInputSize * 3 + 2;
	char	*strResultBuffer = malloc(strResultLenMax);
	char	*strResult = strResultBuffer;
	
	assert(strResultBuffer);

	if (iconv(conv, (char **)&strInput, &strInputSize, &strResult, &strResultLenMax) == (size_t)(-1))
		goto finish;
	
	// Add terminal zero.
	if (terminal_zero)
	{
		if (strResultLenMax < 2)
		{
			free(strResultBuffer);
			result = NULL;
			goto finish;
		}
		
		strResult[0] = 0;
		strResult[1] = 0;

		strResult += 2;
	}
	
	// Finish.
	result = strResultBuffer;
	
	if (len)
		*len = (strResult - strResultBuffer);
	
finish:
#ifdef HAS_LOCK
	iconv(conv, NULL, NULL, NULL, NULL);
	os_unfair_lock_unlock(&lock);
#else
	iconv_close(conv);
#endif
	
	return result;
}
