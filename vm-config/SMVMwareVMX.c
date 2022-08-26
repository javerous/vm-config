/*
 *  SMVMwareVMX.c
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
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include <sys/errno.h>

#include "SMVMwareVMX.h"

#include "SMStringHelper.h"
#include "SMBytesWritter.h"


/*
** Types
*/
#pragma mark - Types

struct SMVMwareVMX
{
	char *path;
	
	SMVMwareVMXEntry	**entries;
	size_t				entries_cnt;
};

struct SMVMwareVMXEntry
{
	SMVMwareVMXEntryType type;
	
	// Updated entry.
	bool updated;
	
	// Original bytes.
	char *original_line;
	
	char *original_key;
	char *original_value;
	char *original_comment;
	
	// Serialization.
	char *serialized_line;
	
	// Updated fields.
	char *updated_key;
	char *updated_value;
	char *updated_comment;
};


/*
** Globals
*/
#pragma mark - Globals

// Errors.
const char * SMVMwareVMXErrorDomain = "com.sourcemac.vmware-vmx.error";

static const char gBlanckCharacters[] = { ' ', '\t' };
static const char gBlanckAndNewlineCharacters[] = { ' ', '\t', '\n' };



/*
** Prototypes
*/
#pragma mark - Prototypes

// VMX.
// > Entries.
static void SMVMwareVMXAddEntry(SMVMwareVMX *vmx, SMVMwareVMXEntry *entry);

// Entry.
// > Instance.
static SMVMwareVMXEntry * 	SMVMwareVMXEntryCreateKeyValue(const char *key, const char *value, SMError **error);
static SMVMwareVMXEntry *	SMVMwareVMXEntryCreateFromLine(const char *line, size_t line_idx, SMError **error);
static void					SMVMwareVMXEntryFree(SMVMwareVMXEntry *entry);

// > Serialization.
static const char *	SMVMwareVMXEntryGetSerializedLine(SMVMwareVMXEntry *entry);
static void			SMVMwareVMXEntryMarkUpdated(SMVMwareVMXEntry *entry);

// Helpers.
// > File.
static bool SMFileWriteBytes(int fd, const void *bytes, size_t len, SMError **error);


								   
/*
** VMX
*/
#pragma mark - VMX

#pragma mark > Instance

SMVMwareVMX * SMVMwareVMXOpen(const char *vmx_file_path, SMError **error)
{
	SMVMwareVMX *result = calloc(1, sizeof(SMVMwareVMX));
	
	assert(result);
	
	// Copy path.
	result->path = strdup(vmx_file_path);
	
	assert(result->path);
		
	// Open the file.
	FILE *file = fopen(vmx_file_path, "r");
	
	if (!file)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, errno, "can't open the file (%d - %s)", errno, strerror(errno));
		goto fail;
	}
	
	// Read lines.
	char	*line = NULL;
	size_t	linecap = 0;
	size_t	line_idx = 0;
	
	while (1)
	{
		// > Read line.
		ssize_t linelen = getline(&line, &linecap, file);
		
		if (linelen == -1)
		{
			if (!feof(file))
			{
				SMSetErrorPtr(error, SMVMwareVMXErrorDomain, errno, "can't read line (%d - %s)", errno, strerror(errno));
				goto fail;
			}
			
			break;
		}
				
		// > Create entry.
		char 				*clean_line = SMStringTrimCharacter(line, gBlanckAndNewlineCharacters, sizeof(gBlanckAndNewlineCharacters), false);
		SMVMwareVMXEntry	*entry = SMVMwareVMXEntryCreateFromLine(clean_line, line_idx, error);
		
		if (!entry)
		{
			free(line);
			goto fail;
		}
		
		// > Add entry.
		SMVMwareVMXAddEntry(result, entry);
		
		// > Update line index.
		line_idx++;
	}

	free(line);
			
	// Return.
	return result;
	
fail:
	SMVMwareVMXFree(result);
	return NULL;
}

void SMVMwareVMXFree(SMVMwareVMX *vmx)
{
	if (!vmx)
		return;

	// Path.
	free(vmx->path);

	// Entries.
	for (size_t i = 0; i < vmx->entries_cnt; i++)
		SMVMwareVMXEntryFree(vmx->entries[i]);
	
	free(vmx->entries);
	
	// Root.
	free(vmx);
}


#pragma mark > Properties

const char * SMVMwareVMXGetPath(SMVMwareVMX *vmx)
{
	return vmx->path;
}


#pragma mark > Serialization

bool SMVMwareVMXWriteToFile(SMVMwareVMX *vmx, const char *path, SMError **error)
{
	// Open file.
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	
	if (fd == -1)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, errno, "can't create the file (%d - %s)", errno, strerror(errno));
		goto fail;
	}
	
	// Write entries.
	size_t entries_count = SMVMwareVMXEntriesCount(vmx);
	
	for (size_t i = 0; i < entries_count; i++)
	{
		SMVMwareVMXEntry	*entry = SMVMwareVMXGetEntryAtIndex(vmx, i);
		const char			*line = SMVMwareVMXEntryGetSerializedLine(entry);
		
		// Write line.
		if (!SMFileWriteBytes(fd, line, strlen(line), error))
			goto fail;
		
		// Write new line.
		static uint8_t new_line[] = { '\n' };
			
		if (!SMFileWriteBytes(fd, new_line, sizeof(new_line), error))
			goto fail;
	}
	
	return true;
	
fail:
	if (fd >= 0)
		close(fd);
	
	unlink(path);
	
	return false;
}


#pragma mark > Entries

static void SMVMwareVMXAddEntry(SMVMwareVMX *vmx, SMVMwareVMXEntry *entry)
{
	vmx->entries = reallocf(vmx->entries, (vmx->entries_cnt + 1) * sizeof(*vmx->entries));
	
	assert(vmx->entries);
	
	vmx->entries[vmx->entries_cnt] = entry;
	vmx->entries_cnt++;
}

size_t SMVMwareVMXEntriesCount(SMVMwareVMX *vmx)
{
	return vmx->entries_cnt;
}

SMVMwareVMXEntry * SMVMwareVMXGetEntryAtIndex(SMVMwareVMX *vmx, size_t idx)
{
	assert(idx < vmx->entries_cnt);
	
	return vmx->entries[idx];
}

SMVMwareVMXEntry * SMVMwareVMXGetEntryForKey(SMVMwareVMX *vmx, const char *key)
{
	// XXX Implement an hash table to speed up queries. Don't forget to update it from children if a key is changed, in this case.
	
	size_t entries_count = SMVMwareVMXEntriesCount(vmx);
	
	for (size_t i = 0; i < entries_count; i++)
	{
		SMVMwareVMXEntry		*entry = SMVMwareVMXGetEntryAtIndex(vmx, i);
		SMVMwareVMXEntryType	entry_type = SMVMwareVMXEntryGetType(entry);
		
		if (entry_type != SMVMwareVMXEntryTypeKeyValue)
			continue;
		
		const char *ekey = SMVMwareVMXEntryGetKey(entry, NULL);
		
		if (!ekey)
			continue;
		
		if (strcmp(ekey, key) == 0)
			return entry;
	}
	
	return NULL;
}

SMVMwareVMXEntry * SMVMwareNVRAMEntryAddKeyValue(SMVMwareVMX *vmx, const char *key, const char *value, SMError **error)
{
	// Create instance.
	SMVMwareVMXEntry *entry = SMVMwareVMXEntryCreateKeyValue(key, value, error);
	
	if (!entry)
		return NULL;
	
	// Add to entries.
	SMVMwareVMXAddEntry(vmx, entry);
	SMVMwareVMXEntryMarkUpdated(entry);
	
	// Return added entry.
	return entry;
}


/*
** Entry
*/
#pragma mark - Entry

#pragma mark > Instance

static SMVMwareVMXEntry * SMVMwareVMXEntryCreateKeyValue(const char *key, const char *value, SMError **error)
{
	SMVMwareVMXEntry *result = calloc(1, sizeof(SMVMwareVMXEntry));
	
	assert(result);

	result->type = SMVMwareVMXEntryTypeKeyValue;
	
	if (!SMVMwareVMXEntrySetKey(result, key, error))
		goto fail;
	
	if (!SMVMwareVMXEntrySetValue(result, value, error))
		goto fail;
	
	return result;
	
fail:
	SMVMwareVMXEntryFree(result);
	return NULL;
}

static SMVMwareVMXEntry * SMVMwareVMXEntryCreateFromLine(const char *line, size_t line_idx, SMError **error)
{
	SMVMwareVMXEntry *result = calloc(1, sizeof(SMVMwareVMXEntry));
	
	assert(result);
	
	// Copy original line.
	result->original_line = strdup(line);
	assert(result->original_line);
	
	// Parse line.
	if (*line == 0)
	{
		result->type = SMVMwareVMXEntryTypeEmpty;
	}
	else if (*line == '#')
	{
		result->type = SMVMwareVMXEntryTypeComment;

		line++;
		
		result->original_comment = SMStringTrimCharacter((char *)line, gBlanckCharacters, sizeof(gBlanckCharacters), false);
	}
	else
	{
		result->type = SMVMwareVMXEntryTypeKeyValue;
		
		// Extract key.
		SMBytesWritter key_writter = SMBytesWritterInit();
		
		// > Extract until we find a character which can terminate a key.
		while (*line && !isblank(*line) && *line != '=')
		{
			SMBytesWritterAppendByte(&key_writter, *line);
			line++;
		}
		
		// > Check we are not end-of-line.
		if (*line == 0)
		{
			SMBytesWritterFree(&key_writter);
			SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "unexpected end-of-line when parsing key at line %lu", line_idx + 1);
			
			goto fail;
		}
		
		// > Add terminal 0.
		SMBytesWritterAppendByte(&key_writter, 0);
		
		
		// Search key-value separator.
		// > Skip potential white characters between end of key, and key-value separator.
		while (*line && isblank(*line))
			line++;
		
		// > Check we are not end-of-line.
		if (*line == 0)
		{
			SMBytesWritterFree(&key_writter);
			SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "unexpected end-of-line when searching key-value separator at line %lu", line_idx + 1);
			
			goto fail;
		}
		
		// > Check we are stopped at key-value separator.
		if (*line != '=')
		{
			SMBytesWritterFree(&key_writter);
			SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "unexpected character '%c' when searching key-value separator at line %lu", *line, line_idx + 1);
			
			goto fail;
		}
		
		// > Skip the key-value separator.
		line++;
		
		
		// Search value.
		// > Skip potential white characters between key-value separator and value.
		while (*line && isblank(*line))
			line++;
		
		// > Check we are not end-of-line.
		if (*line == 0)
		{
			SMBytesWritterFree(&key_writter);
			SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "unexpected end-of-line when searching value at line %lu", line_idx + 1);
			
			goto fail;
		}
		
		// > Check we have value delimiter.
		if (*line != '"')
		{
			SMBytesWritterFree(&key_writter);
			SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "unexpected character '%c' when searching value at line %lu", *line, line_idx + 1);
			
			goto fail;
		}
		
		// > Skip value delimiter.
		line++;
		
		
		// Extract value.
		SMBytesWritter value_writter = SMBytesWritterInit();
		
		bool last_escaped = false;
		bool value_extracting = true;
		
		for (; *line && value_extracting; line++)
		{
			if (last_escaped)
			{
				SMBytesWritterAppendByte(&value_writter, *line);
				last_escaped = false;
			}
			else
			{
				if (*line == '\\')
					last_escaped = true;
				else if (*line == '"')
					value_extracting = false;
				else
					SMBytesWritterAppendByte(&value_writter, *line);
			}
		}
		
		if (*line == 0 && value_extracting)
		{
			SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "unexpected end-of-line when parsing value at line %lu", line_idx + 1);
			
			SMBytesWritterFree(&key_writter);
			SMBytesWritterFree(&value_writter);
			
			goto fail;
		}
		
		SMBytesWritterAppendByte(&value_writter, 0);
		
		
		// Store result.
		result->original_key = SMBytesWritterPtr(&key_writter);
		result->original_value = SMBytesWritterPtr(&value_writter);
	}
	
	return result;
	
fail:
	SMVMwareVMXEntryFree(result);
	return NULL;
}

static void SMVMwareVMXEntryFree(SMVMwareVMXEntry *entry)
{
	if (!entry)
		return;
	
	free(entry->original_line);
	free(entry->original_key);
	free(entry->original_value);
	free(entry->original_comment);
	
	free(entry->serialized_line);
	
	free(entry->updated_key);
	free(entry->updated_value);
	free(entry->updated_comment);

	free(entry);
}


#pragma mark > Serialization

static const char *	SMVMwareVMXEntryGetSerializedLine(SMVMwareVMXEntry *entry)
{
	// Return cached serialized bytes.
	if (entry->serialized_line)
		return entry->serialized_line;
	
	// Return original line if the entry wasn't updated.
	if (!entry->updated)
		return entry->original_line;
	
	// Serialize line.
	char *line = NULL;
	
	switch (SMVMwareVMXEntryGetType(entry))
	{
		case SMVMwareVMXEntryTypeEmpty:
		{
			line = strdup("");
			break;
		}
			
		case SMVMwareVMXEntryTypeComment:
		{
			asprintf(&line, "# %s", SMVMwareVMXEntryGetComment(entry, NULL));
			break;
		}
			
		case SMVMwareVMXEntryTypeKeyValue:
		{
			const char *key = SMVMwareVMXEntryGetKey(entry, NULL);
			const char *value = SMVMwareVMXEntryGetValue(entry, NULL);
			
			char *fixed_value = SMStringReplaceString((char *)value, "\"", "\\\"", false);
			
			asprintf(&line, "%s = \"%s\"", key, fixed_value);
			
			free(fixed_value);

			break;
		}
	}
	
	assert(line);
	
	entry->serialized_line = line;
	
	return line;
}

static void SMVMwareVMXEntryMarkUpdated(SMVMwareVMXEntry *entry)
{
	// Flag as updated.
	entry->updated = true;
	
	// Free serialized bytes.
	if (entry->serialized_line)
	{
		free(entry->serialized_line);
		entry->serialized_line = NULL;
	}
}


#pragma mark > Type

SMVMwareVMXEntryType SMVMwareVMXEntryGetType(SMVMwareVMXEntry *entry)
{
	return entry->type;
}


#pragma mark > Comment

const char * SMVMwareVMXEntryGetComment(SMVMwareVMXEntry *entry, SMError **error)
{
	// Check type.
	if (entry->type != SMVMwareVMXEntryTypeComment)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "invalid type");
		return NULL;
	}
	
	// Return comment.
	if (entry->updated_comment)
		return entry->updated_comment;
	else
		return entry->original_comment;
}

bool SMVMwareVMXEntrySetComment(SMVMwareVMXEntry *entry, const char *comment, SMError **error)
{
	// Check type.
	if (entry->type != SMVMwareVMXEntryTypeComment)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "invalid type");
		return false;
	}
	
	// Update comment.
	free(entry->updated_comment);
	entry->updated_comment = strdup(comment);
	
	assert(entry->updated_comment);
	
	// Mark as updated.
	SMVMwareVMXEntryMarkUpdated(entry);
	
	return true;
}


#pragma mark > Key-Value

const char * SMVMwareVMXEntryGetKey(SMVMwareVMXEntry *entry, SMError **error)
{
	// Check type.
	if (entry->type != SMVMwareVMXEntryTypeKeyValue)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "invalid type");
		return NULL;
	}
	
	// Return key.
	if (entry->updated_key)
		return entry->updated_key;
	else
		return entry->original_key;
}

bool SMVMwareVMXEntrySetKey(SMVMwareVMXEntry *entry, const char *key, SMError **error)
{
	// FIXME We should validate key (I guess it can't contain chars like = or ").
	
	// Check type.
	if (entry->type != SMVMwareVMXEntryTypeKeyValue)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "invalid type");
		return false;
	}
	
	// Update key.
	free(entry->updated_key);
	entry->updated_key = strdup(key);
	
	assert(entry->updated_key);
	
	// Mark as updated.
	SMVMwareVMXEntryMarkUpdated(entry);
	
	return true;
}


const char * SMVMwareVMXEntryGetValue(SMVMwareVMXEntry *entry, SMError **error)
{
	// Check type.
	if (entry->type != SMVMwareVMXEntryTypeKeyValue)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "invalid type");
		return NULL;
	}
	
	// Return value.
	if (entry->updated_value)
		return entry->updated_value;
	else
		return entry->original_value;
}

bool SMVMwareVMXEntrySetValue(SMVMwareVMXEntry *entry, const char *value, SMError **error)
{
	// Check type.
	if (entry->type != SMVMwareVMXEntryTypeKeyValue)
	{
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, -1, "invalid type");
		return false;
	}
	
	// Update value.
	free(entry->updated_value);
	entry->updated_value = strdup(value);
	
	assert(entry->updated_value);
	
	// Mark as updated.
	SMVMwareVMXEntryMarkUpdated(entry);
	
	return true;
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
		
		SMSetErrorPtr(error, SMVMwareVMXErrorDomain, errno, "can't write bytes (%d - %s)", err_bck, strerror(err_bck));
		
		return false;
	}
	
	return true;
}
