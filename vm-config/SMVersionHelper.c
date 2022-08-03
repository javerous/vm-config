/*
 *  SMVersionHelper.c
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

#include <ctype.h>

#include "SMVersionHelper.h"


/*
** Globals
*/
#pragma mark - GLobals

const SMVersion SMVersionInvalid = { .major_version = (unsigned)-1, .minor_version = (unsigned)-1, .patch_version = (unsigned)-1 };

const char * SMVersionErrorDomain = "com.sourcemac.version";


/*
** Interface
*/
#pragma mark - Interface

#pragma mark Parse

SMVersion SMVersionFromString(const char *version_str, SMError **error)
{
	// Major.
	unsigned	major = 0;
	bool 		major_found = false;

	while (isdigit(*version_str))
	{
		major *= 10;
		major += *version_str - '0';
		version_str++;
		major_found = true;
	}
	
	if (*version_str != '.' || !major_found)
	{
		SMSetErrorPtr(error, SMVersionErrorDomain, -1, "invalid major field in version string");
		return SMVersionInvalid;
	}
	
	version_str++;
	
	// Minor.
	unsigned	minor = 0;
	bool		minor_found = false;

	while (isdigit(*version_str))
	{
		minor *= 10;
		minor += *version_str - '0';
		version_str++;
		minor_found = true;
	}
	
	if (*version_str != '.' || !minor_found)
	{
		SMSetErrorPtr(error, SMVersionErrorDomain, -1, "invalid minor field in version string");
		return SMVersionInvalid;
	}
	
	version_str++;
	
	// Patch.
	unsigned 	patch = 0;
	bool 		patch_found = false;

	while (isdigit(*version_str))
	{
		patch *= 10;
		patch += *version_str - '0';
		version_str++;
		patch_found = true;
	}
	
	if (*version_str != 0 || !patch_found)
	{
		SMSetErrorPtr(error, SMVersionErrorDomain, -1, "invalid path field in version string");
		return SMVersionInvalid;
	}
	
	// Result.
	return SMVersionFromComponents(major, minor, patch);
}

SMVersion SMVersionFromComponents(unsigned major_version, unsigned minor_version, unsigned patch_version)
{
	return (SMVersion){ .major_version = major_version, .minor_version = minor_version, .patch_version = patch_version };
}


#pragma mark Compare

bool SMVersionIsGreater(SMVersion v1, SMVersion v2)
{
	// Major.
	if (v1.major_version > v2.major_version)
		return true;
	
	if (v1.major_version < v2.major_version)
		return false;
	
	// Minor.
	if (v1.minor_version > v2.minor_version)
		return true;
	
	if (v1.minor_version < v2.minor_version)
		return false;
	
	// Patch.
	return (v1.patch_version > v2.patch_version);
}

bool SMVersionIsEqual(SMVersion v1, SMVersion v2)
{
	return (v1.major_version == v2.major_version) && (v1.minor_version == v2.minor_version) && (v1.patch_version == v2.patch_version);
}
