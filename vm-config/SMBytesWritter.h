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
#include <assert.h>

#include <sys/types.h>



/*
** Defines
*/
#pragma mark - Defines

#define SMBytesWritterInit() { 0 }

#define SMBytesWritterPtrOff(Type, Writter, Offset) ({	\
	void *__bytes = SMBytesWritterPtr(Writter);			\
	Type *__value = __bytes + (Offset);					\
														\
	__value;											\
})


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

#pragma mark > Helpers

static __attribute__((always_inline)) inline
void SMWritterReallocAppendSize(SMBytesWritter *writter, size_t size)
{
	if (writter->bytes_size < writter->size + size)
	{
		writter->bytes_size = writter->size + size + 10;
		writter->bytes = reallocf(writter->bytes, writter->bytes_size);

		assert(writter->bytes);
	}
}


#pragma mark > Instance

static __attribute__((always_inline)) inline
void SMBytesWritterFree(SMBytesWritter *writter)
{
	free(writter->bytes);
	writter->bytes = NULL;
}


#pragma mark > Properties

static __attribute__((always_inline)) inline
void * SMBytesWritterPtr(SMBytesWritter *writter)
{
	return writter->bytes;
}

static __attribute__((always_inline)) inline
size_t SMBytesWritterSize(SMBytesWritter *writter)
{
	return writter->size;
}


#pragma mark > Append

static __attribute__((always_inline)) inline
off_t SMBytesWritterAppendBytes(SMBytesWritter *writter, const void *bytes, size_t size)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, size);

	memcpy(writter->bytes + writter->size, bytes, size);
	writter->size += size;

	return result;
}

static __attribute__((always_inline)) inline
off_t SMBytesWritterAppendRepeatedByte(SMBytesWritter *writter, uint8_t byte, size_t count)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, count);

	memset(writter->bytes + writter->size, (int)byte, count);
	writter->size += count;

	return result;
}

static __attribute__((always_inline)) inline
off_t SMBytesWritterAppendByte(SMBytesWritter *writter, uint8_t byte)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, 1);

	*(((uint8_t *)writter->bytes) + writter->size) = byte;
	writter->size += 1;

	return result;
}

static __attribute__((always_inline)) inline
off_t SMBytesWritterAppendSpace(SMBytesWritter *writter, size_t size)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, size);

	writter->size += size;

	return result;
}
