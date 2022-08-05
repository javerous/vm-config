/*
 *  SMError.h
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


/*
** Types
*/
#pragma mark - Types

typedef struct SMError SMError;


/*
** Defines
*/
#pragma mark - Defines

#ifndef __printflike
# define __printflike(...)
#endif

#define SMSetErrorPtr(Ptr, Domain, Code, UserInfo, ...) ({					\
	SMError **__smerror = (Ptr);											\
																			\
	if (__smerror)															\
		*__smerror = SMErrorCreate(Domain, Code, UserInfo, ##__VA_ARGS__);	\
})


/*
** Functions
*/
#pragma mark - Functions

// Instance.
SMError *	SMErrorCreate(const char *domain, int code, const char *user_info, ...) __printflike(3, 4);
void		SMErrorFree(SMError *error);

// Properties.
const char *	SMErrorGetDomain(SMError *error);

int				SMErrorGetCode(SMError *error);

const char *	SMErrorGetUserInfo(SMError *error);
const char *	SMErrorGetSentencizedUserInfo(SMError *error);
