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
#include <ctype.h>
#include <string.h>

#include "SMVMwareVMXHelper.h"

#include "SMBytesWritter.h"


/*
** Functions
*/
#pragma mark - Functions

#pragma mark Machine UUID

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


#pragma mark Version

SMVersion SMVMwareVMXExtractMacOSVersion(SMVMwareVMX *vmx)
{
	// Define guest-os prefixes to macOS versions.
	static struct {
		const char *guest_os_prefix;
		size_t		guest_os_prefix_len;
		SMVersion	version;
	} darwin_to_version[] = {
		{ "darwin1-",	8,	SMVersionFromComponents(10, 0, 0) },
		{ "darwin5-",	8,	SMVersionFromComponents(10, 1, 0) },
		{ "darwin6-",	8,	SMVersionFromComponents(10, 2, 0) },
		{ "darwin7-",	8,	SMVersionFromComponents(10, 3, 0) },
		{ "darwin8-",	8,	SMVersionFromComponents(10, 4, 0) },
		{ "darwin9-",	8,	SMVersionFromComponents(10, 5, 0) },
		{ "darwin10-",	9,	SMVersionFromComponents(10, 6, 0) },
		{ "darwin11-",	9,	SMVersionFromComponents(10, 7, 0) },
		{ "darwin12-",	9,	SMVersionFromComponents(10, 8, 0) },
		{ "darwin13-",	9,	SMVersionFromComponents(10, 9, 0) },
		{ "darwin14-",	9,	SMVersionFromComponents(10, 10, 0) },
		{ "darwin15-",	9,	SMVersionFromComponents(10, 11, 0) },
		{ "darwin16-",	9,	SMVersionFromComponents(10, 12, 0) },
		{ "darwin17-",	9,	SMVersionFromComponents(10, 13, 0) },
		{ "darwin18-",	9,	SMVersionFromComponents(10, 14, 0) },
		{ "darwin19-",	9,	SMVersionFromComponents(10, 15, 0) },
		{ "darwin20-",	9,	SMVersionFromComponents(11, 0, 0) },
		{ "darwin21-",	9,	SMVersionFromComponents(12, 0, 0) },
		{ "darwin22-",	9,	SMVersionFromComponents(13, 0, 0) },
	};

	// Search for detailed data.
	const char *detailed_data = NULL;

	// > Try with Guest OS.
	SMVMwareVMXEntry *guest_os_ddata_entry = SMVMwareVMXGetEntryForKey(vmx, SMVMwareVMXGuestOSDetailedDataKey);

	if (guest_os_ddata_entry)
		detailed_data = SMVMwareVMXEntryGetValue(guest_os_ddata_entry, NULL);

	// > Try with Guest Info.
	if (!detailed_data)
	{
		SMVMwareVMXEntry *guest_info_entry = SMVMwareVMXGetEntryForKey(vmx, SMVMwareVMXGuestInfoDetailedDataKey);

		if (guest_info_entry)
			detailed_data = SMVMwareVMXEntryGetValue(guest_info_entry, NULL);
	}

	// > Parse detailed data.
	if (detailed_data)
	{
		SMDetailedField *fields = SMDetailedFieldsFromString(detailed_data);

		if (fields)
		{
			bool found_macos = false;
			char *version = NULL;

			// > Extract version & validate it's macOS.
			for (size_t i = 0; fields[i].key && fields[i].value; i++)
			{
				if (!found_macos && strcasecmp(fields[i].key, "distroName") == 0 && strcasecmp(fields[i].value, "Mac OS X") == 0)
					found_macos = true;
				else if (!found_macos && strcasecmp(fields[i].key, "familyName") == 0 && strcasecmp(fields[i].value, "Darwin") == 0)
					found_macos = true;
				else if (!version && strcasecmp(fields[i].key, "distroVersion") == 0)
					version = fields[i].value;

				if (fields[i].value != version)
					free(fields[i].value);

				free(fields[i].key);
			}

			free(fields);

			// > Try to parse the version.
			if (found_macos && version)
			{
				// > As it.
				SMVersion macos_version = SMVersionFromString(version, NULL);

				if (!SMVersionIsEqual(macos_version, SMVersionInvalid))
				{
					free(version);
					return macos_version;
				}

				// > With extra 0.
				char ext_version[20] = { 0 };

				snprintf(ext_version, sizeof(ext_version), "%s.0", version);

				macos_version = SMVersionFromString(ext_version, NULL);

				if (!SMVersionIsEqual(macos_version, SMVersionInvalid))
				{
					free(version);
					return macos_version;
				}
			}

			free(version);
		}
	}

	// > Try with Guest OS.
	SMVMwareVMXEntry *guest_os_entry = SMVMwareVMXGetEntryForKey(vmx, SMVMwareVMXGuestOSKey);

	if (guest_os_entry)
	{
		const char *guest_os = SMVMwareVMXEntryGetValue(guest_os_entry, NULL);

		for (size_t i = 0; i < sizeof(darwin_to_version) / sizeof(darwin_to_version[0]); i++)
		{
			if (strncmp(guest_os, darwin_to_version[i].guest_os_prefix, darwin_to_version[i].guest_os_prefix_len) == 0)
				return darwin_to_version[i].version;
		}
	}

	// > Fallback.
	return SMVersionInvalid;
}


#pragma mark Helper

SMDetailedField * SMDetailedFieldsFromString(const char *detailed_data)
{
	SMBytesWritter fields_writter = SMBytesWritterInit();

	// Parse fields.
	const char *str = detailed_data;

	while (*str)
	{
		// > Skip blank chars.
		while (*str && isblank(*str))
			str++;

		if (*str == 0)
			goto done;

		// > Extract key.
		SMBytesWritter key_writter = SMBytesWritterInit();

		while (*str && !isblank(*str) && *str != '=')
		{
			SMBytesWritterAppendByte(&key_writter, *str);
			str++;
		}

		if (*str == 0)
		{
			SMBytesWritterFree(&key_writter);
			goto done;
		}

		SMBytesWritterAppendByte(&key_writter, 0);

		// > Skip separator.
		while (*str && *str != '\'')
			   str++;

		if (*str == 0)
		{
			SMBytesWritterFree(&key_writter);
			goto done;
		}

		str++;

		// > Skip blank chars.
		while (*str && isblank(*str))
			str++;

		if (*str == 0)
			goto done;

		// > Extract value.
		SMBytesWritter value_writter = SMBytesWritterInit();

		bool last_escaped = false;
		bool value_extracting = true;

		for (; *str && value_extracting; str++)
		{
			if (last_escaped)
			{
				SMBytesWritterAppendByte(&value_writter, *str);
				last_escaped = false;
			}
			else
			{
				if (*str == '\\')
					last_escaped = true;
				else if (*str == '\'')
					value_extracting = false;
				else
					SMBytesWritterAppendByte(&value_writter, *str);
			}
		}

		if (*str == 0 && value_extracting)
		{
			SMBytesWritterFree(&key_writter);
			SMBytesWritterFree(&value_writter);

			goto done;
		}

		SMBytesWritterAppendByte(&value_writter, 0);

		// > Append field.
		off_t 			field_offset = SMBytesWritterAppendSpace(&fields_writter, sizeof(SMDetailedField));
		SMDetailedField	*field = SMBytesWritterPtrOff(SMDetailedField, &fields_writter, field_offset);

		field->key = SMBytesWritterPtr(&key_writter);
		field->value = SMBytesWritterPtr(&value_writter);
	}

done:
	// Append terminal empty field.
	SMBytesWritterAppendRepeatedByte(&fields_writter, 0, sizeof(SMDetailedField));

	// Return.
	return SMBytesWritterPtr(&fields_writter);
}

void SMDetailedFieldsFree(SMDetailedField *fields)
{
	for (size_t i = 0; ; i++)
	{
		if (!fields[i].key || !fields[i].value)
			break;
	
		free(fields[i].key);
		free(fields[i].value);
	}
	
	free(fields);
}
