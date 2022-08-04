/*
 *  SMBytesWritter.c
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
#include <assert.h>
#include <string.h>

#include "SMBytesWritter.h"


/*
** Inlines
*/
#pragma mark - Inlines

static __attribute__((always_inline))
void SMWritterReallocAppendSize(SMBytesWritter *writter, size_t size)
{
	if (writter->bytes_size < writter->size + size)
	{
		writter->bytes_size = writter->size + size + 10;
		writter->bytes = reallocf(writter->bytes, writter->bytes_size);

		assert(writter->bytes);
	}
}


/*
** Functions
*/
#pragma mark - Functions

off_t SMWriteAppendBytes(SMBytesWritter *writter, const void *bytes, size_t size)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, size);

	memcpy(writter->bytes + writter->size, bytes, size);
	writter->size += size;

	return result;
}

off_t SMWriteAppendRepeatedByte(SMBytesWritter *writter, uint8_t byte, size_t count)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, count);

	memset(writter->bytes + writter->size, (int)byte, count);
	writter->size += count;

	return result;
}

off_t SMWriteAppendByte(SMBytesWritter *writter, uint8_t byte)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, 1);

	*(((uint8_t *)writter->bytes) + writter->size) = byte;
	writter->size += 1;

	return result;
}

off_t SMWriteAppendSpace(SMBytesWritter *writter, size_t size)
{
	off_t result = writter->size;

	SMWritterReallocAppendSize(writter, size);

	writter->size += size;

	return result;
}
