/*
 *  SMStringHelper.c
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
#include <string.h>

#include "SMStringHelper.h"


/*
** Functionss
*/
#pragma mark - Functions

#pragma mark General

char * SMStringDuplicate(const void *str, size_t len)
{
	char *result = malloc(len + 1);
	
	memcpy(result, str, len);
	result[len] = 0;
	
	return result;
}

char * SMStringTrimCharacter(char *str, const char *chars, size_t chars_cnt, bool free_str)
{
#define is_char(c) ({ 											\
	bool __result = false;										\
																\
	for (size_t __i = 0; __i < chars_cnt && !__result; __i++)	\
		__result = ((c) == chars[__i]);							\
																\
	__result;													\
})
	
	// Trim beggining.
	while (*str && is_char(*str))
		str++;
	
	if (*str == 0)
	{
		if (free_str)
			free(str);
		
		return strdup("");
	}
	
	// Trim end.
	size_t		len = strlen(str);
	const char	*end = str + len;
	
	while (end != str && (*end == 0 || is_char(*end)))
		end--;
	
	if (end == str)
	{
		if (free_str)
			free(str);
		
		return strdup("");
	}
	
	// Forge result.
	size_t	flen = (end - str) + 1;
	char	*result = malloc(flen + 1);
	
	if (!result)
		return NULL;
	
	memcpy(result, str, flen);
	result[flen] = 0;
	
	if (free_str)
		free(str);
	
	return result;
}

char * SMStringReplaceString(char *str, const char *value, const char *replacement, bool free_str)
{
#define append_size(Size) ({						\
	if (result_buff_len < result_len + Size)		\
	{												\
		result_buff_len = result_len + Size + 10;	\
		result = realloc(result, result_buff_len);	\
													\
		if (!result)								\
			return NULL;							\
	}												\
})
	
	// Compute lens.
	char *str_bck = str;
	
	char 	*result = NULL;
	size_t	result_len = 0;
	size_t 	result_buff_len = 0;
	
	size_t	str_len = strlen(str);
	size_t	value_len = strlen(value);
	size_t	replacement_len = strlen(replacement);
	
	// Fast path.
	if (value_len == 0 || str_len < value_len)
	{
		result = strdup(str);
		
		if (free_str)
			free(str);
		
		return result;
	}
	
	// Compose result string.
	while (*str)
	{
		if (str_len >= value_len && memcmp(str, value, value_len) == 0)
		{
			append_size(replacement_len);
			memcpy(result + result_len, replacement, replacement_len);
			
			str += value_len;
			str_len -= value_len;
			
			result_len += replacement_len;
		}
		else
		{
			append_size(1);
			result[result_len] = *str;
			
			str += 1;
			str_len -= 1;
			
			result_len += 1;
		}
	}
	
	// Terminal zero.
	append_size(1);
	result[result_len] = 0;
	
	// Free.
	if (free_str)
		free(str_bck);
	
	// Result.
	return result;
}


#pragma mark Path

char * SMStringPathAppendComponent(const char *path, const char *component)
{
	// Compute sizes.
	size_t path_len = strlen(path);
	size_t comp_len = strlen(component);

	if (comp_len == 0)
		return strdup(path);
	
	// Allocate result.
	size_t	result_max_len = path_len + 1 + comp_len + 1;
	char	*result = malloc(result_max_len);
	size_t	i = 0;
	
	*result = 0;
	
	// Copy paths.
	while (path_len != 0 && path[path_len - 1] == '/')
		path_len--;
	
	memcpy(result + i, path, path_len);
	i += path_len;
	
	// Add intermediate '/'.
	result[i] = '/';
	i++;
	
	// Add components.
	while (*component && *component == '/')
	{
		component++;
		comp_len--;
	}
	
	memcpy(result + i, component, comp_len);
	i += comp_len;
	
	// Terminal zero.
	result[i] = 0;
	
	// Result.
	return result;
}

bool SMStringPathHasExtension(const char *path, const char *ext)
{
	size_t	path_len = strlen(path);
	size_t	ext_len = strlen(ext);
	
	if (ext_len == 0)
		return false;
	
	if (path_len == 0)
		return false;
	
	if (ext_len + 1 > path_len)
		return false;

	return (path[path_len - ext_len - 1] == '.') && (memcmp(path + path_len - ext_len, ext, ext_len) == 0);
}
