/*
 *  SMError.c
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "SMError.h"


/*
** Types
*/
#pragma mark - Types

struct SMError
{
	char	*domain;
	int		code;
	char	*user_info;
};


/*
** Functions
*/
#pragma mark - Functions

#pragma mark Instance

SMError * SMErrorCreate(const char *domain, int code, const char *user_info, ...)
{
	va_list ap;
	SMError *result = malloc(sizeof(SMError));
	
	assert(result);
	
	result->domain = strdup(domain);
	result->code = code;
	
	va_start(ap, user_info);
	{
		vasprintf(&result->user_info, user_info, ap);
	}
	va_end(ap);

	return result;
}

void SMErrorFree(SMError *error)
{
	if (!error)
		return;
	
	free(error->domain);
	free(error->user_info);
	free(error);
}


#pragma mark Properties

const char * SMErrorGetDomain(SMError *error)
{
	return error->domain;
}

int SMErrorGetCode(SMError *error)
{
	return error->code;
}

const char * SMErrorGetUserInfo(SMError *error)
{
	return error->user_info;
}
