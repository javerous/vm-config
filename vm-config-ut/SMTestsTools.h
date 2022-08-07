/*
 *  SMTestsTools.h
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

// C-String
#define XCTAssertEqualFreeableStrings(FreeableString, ReferenceString) ({	\
	char		*__freeable_str = (FreeableString);							\
	const char	*__ref_str = (ReferenceString);								\
	int			__result = strcmp(__freeable_str, __ref_str);				\
																			\
	XCTAssertEqual(__result, 0, "'%s' != '%s'", __freeable_str, __ref_str);	\
																			\
	free(__freeable_str);													\
})

#define XCTAssertEqualStrings(TestString, ReferenceString) ({				\
	const char	*__test_str = (TestString);									\
	const char	*__ref_str = (ReferenceString);								\
	int			__result = strcmp(__test_str, __ref_str);					\
																			\
	XCTAssertEqual(__result, 0, "'%s' != '%s'", __test_str, __ref_str);		\
})

// On-exit
static inline void _execBlock (__strong dispatch_block_t *block) {
	(*block)();
}

#define __concat_(A, B) A ## B
#define __concat(A, B) __concat_(A, B)

#define _onExit \
	__strong dispatch_block_t __concat(_exitBlock_, __LINE__) __attribute__((cleanup(_execBlock), unused)) = ^
