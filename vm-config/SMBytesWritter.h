/*
 *  SMBytesWritter.h
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


#include <stdlib.h>
#include <sys/types.h>


/*
** Defines
*/
#pragma mark - Defines

#define SMBytesWritterInit() { 0 }

#define SMBytesWritterPtrOff(Type, Writter, Offset) ({	\
	Type * __value = (Writter)->bytes + (Offset);		\
														\
	__value;											\
})

#define SMBytesWritterPtr(Writter)	({ (Writter)->bytes; })
#define SMBytesWritterSize(Writter)	({ (Writter)->size; })
#define SMBytesWritterFree(Writter)	({ free((Writter)->bytes); })


/*
** Types
*/
#pragma mark - Types

typedef struct SMBytesWritter
{
	void	*bytes;
	size_t	bytes_size;

	size_t	size;
} SMBytesWritter;


/*
** Functions
*/
#pragma mark - Functions

off_t SMBytesWritterAppendBytes(SMBytesWritter *writter, const void *bytes, size_t size);
off_t SMBytesWritterAppendRepeatedByte(SMBytesWritter *writter, uint8_t byte, size_t count);
off_t SMBytesWritterAppendByte(SMBytesWritter *writter, uint8_t byte);
off_t SMBytesWritterAppendSpace(SMBytesWritter *writter, size_t size);
