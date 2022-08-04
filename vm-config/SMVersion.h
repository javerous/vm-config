/*
 *  SMVersion.h
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
** Defines
*/
#pragma mark - Defines

#define SMVersionFromComponents(MajorVersion, MinorVersion, PatchVersion) (SMVersion){ .major_version = (MajorVersion), .minor_version = (MinorVersion), .patch_version = (PatchVersion) }

#define SMVersionIsGreaterOrEqual(V1, V2) ({							\
	SMVersion __v1 = (V1);												\
	SMVersion __v2 = (V2);												\
																		\
	SMVersionIsGreater(__v1, __v2) || SMVersionIsEqual(__v1, __v2);		\
})

#define SMVersionIsLess(V1, V2)			({ !SMVersionIsGreaterOrEqual(V1, V2); )}
#define SMVersionIsLessOrEqual(V1, V2)	({ !SMVersionIsGreater(V1, V2 ); })


/*
** Types
*/
#pragma mark - Types

typedef struct
{
	unsigned major_version;
	unsigned minor_version;
	unsigned patch_version;
} SMVersion;


/*
** Globals
*/
#pragma mark - Globals

extern const SMVersion SMVersionInvalid;

extern const char * SMVersionErrorDomain;


/*
** Functions
*/
#pragma mark - Function

// Instance.
SMVersion SMVersionFromString(const char *version_str, SMError **error);

// Compare.
bool SMVersionIsGreater(SMVersion v1, SMVersion v2);	// True if v1 > v2.
bool SMVersionIsEqual(SMVersion v1, SMVersion v2);		// True if v1 == v2.
