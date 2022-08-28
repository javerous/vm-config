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

// Test asserts.
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

#define XCTAssertContainString(WholeString, WholeSize, PartString) ({					\
	const char	*__whole_str = (WholeString);											\
	size_t 		__whole_size = (WholeSize);												\
	const char	*__part_str = (PartString);												\
	char		*__result = strnstr(__whole_str, __part_str, __whole_size);				\
																						\
	XCTAssertNotEqual(__result, NULL, "'%s' not in '%.20s...'", __part_str, __whole_str);	\
})

#define XCTAssertNotContainString(WholeString, WholeSize, PartString) ({				\
	const char	*__whole_str = (WholeString);											\
	size_t 		__whole_size = (WholeSize);												\
	const char	*__part_str = (PartString);												\
	char		*__result = strnstr(__whole_str, __part_str, __whole_size);				\
																						\
	XCTAssertEqual(__result, NULL, "'%s' in '%.20s...'", __part_str, __result);			\
})

// Boilerplate.
#define SMDeclareDefaultFiles 								\
	char	**bout = (char **)calloc(1, sizeof(void *));	\
	char	**berr = (char **)calloc(1, sizeof(void *));	\
	size_t	sout = 0;										\
	size_t	serr = 0;										\
	FILE	*fout = open_memstream(bout, &sout);			\
	FILE	*ferr = open_memstream(berr, &serr);			\
															\
	_onExit { 												\
		fclose(fout);										\
		fclose(ferr);										\
		free(*bout); 										\
		free(*berr);										\
		free(bout); 										\
		free(berr); 										\
	}

// On-exit
static inline void _execBlock (__strong dispatch_block_t *block) {
	(*block)();
}

#define __concat_(A, B) A ## B
#define __concat(A, B) __concat_(A, B)

#define _onExit \
	__strong dispatch_block_t __concat(_exitBlock_, __LINE__) __attribute__((cleanup(_execBlock), unused)) = ^


// Paths.
static __attribute__((always_inline)) inline
NSString * SMGenerateTemporaryTestPath(void)
{
	return [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"com.sourcemac.ut-file-%@", [NSUUID UUID].UUIDString]];
}
