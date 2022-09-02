/*
 *  SMCommandLineOptions.h
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
#include <stdio.h>

#include "SMError.h"


/*
** Types
*/
#pragma mark - Types

// API.
typedef struct SMCLOptions			SMCLOptions;
typedef struct SMCLOptionsVerb		SMCLOptionsVerb;

typedef struct SMCLOptionsResult	SMCLOptionsResult;

typedef enum
{
	SMCLValueTypeString,
	SMCLValueTypeUInt32,
	SMCLValueTypeInt32,
	SMCLValueTypeUInt64,
	SMCLValueTypeInt64,
} SMCLValueType;

typedef enum
{
	// Parse errors.
	SMCLErrorParseInvalidArgumentsArray = 1,// The argument array passed to parser is invalid.
	SMCLErrorParseMissingVerb,				// The verv in the argument array is missing.
	SMCLErrorParseUnknowVerb,				// The verb in the argument array is unknown.
	SMCLErrorParseUnexpectedArgument, 		// An argument in argument array which have invalid format.
	SMCLErrorParseUnexpectedExtraArgument,	// An argument in argument array that have good format, and may be valid, but that we can't match.
	SMCLErrorParseUnkownOption,				// An option in the argument array is unknown.
	SMCLErrorParseMissingOptionArgument,	// An option in the argument array require an argument, but none is available.
	SMCLErrorParseMissingParameter,			// A parameter was not optional, but not provided in the argument array.
	SMCLErrorParseInvalidOptionArgument,	// The argument of an option is expected to be in a specific format, but this format is invalid.
} SMCLError;


/*
** Globals
*/
#pragma mark - Globals

extern const char * SMCommanLineOptionsErrorDomain;


/*
** Functions
*/
#pragma mark - Functions

// > Instance.
SMCLOptions *	SMCLOptionsCreate(void);
void			SMCLOptionsFree(SMCLOptions *options);

// > Verbs.
SMCLOptionsVerb * SMCLOptionsAddVerb(SMCLOptions *options, uint64_t identifier, const char *name, const char *description);

// > Parameters.
void SMCLOptionsVerbAddValue(SMCLOptionsVerb *verb, uint64_t identifier, const char *name, const char *description);

void SMCLOptionsVerbAddOption(SMCLOptionsVerb *verb, uint64_t identifier, bool optional, const char *name, char short_name, const char *description);
void SMCLOptionsVerbAddOptionWithArgument(SMCLOptionsVerb *verb, uint64_t identifier, bool optional, const char *name, char short_name, SMCLValueType argument_type, const char *argument_name, const char *description);

// > Usage.
void SMCLOptionsPrintUsage(SMCLOptions *options, FILE *output);

// > Parsing.
SMCLOptionsResult * SMCLOptionsParse(SMCLOptions *options, int argc, const char * argv[], SMError **error);

uint64_t		SMCLOptionsResultVerbIdentifier(SMCLOptionsResult *result);

size_t			SMCLOptionsResultParametersCount(SMCLOptionsResult *result);
uint64_t  		SMCLOptionsResultParameterIdentifierAtIndex(SMCLOptionsResult *result, size_t idx);

SMCLValueType	SMCLOptionsResultParameterTypeAtIndex(SMCLOptionsResult *result, size_t idx);
const char *	SMCLOptionsResultParameterStringValueAtIndex(SMCLOptionsResult *result, size_t idx);
uint32_t		SMCLOptionsResultParameterUInt32ValueAtIndex(SMCLOptionsResult *result, size_t idx);
int32_t			SMCLOptionsResultParameterInt32ValueAtIndex(SMCLOptionsResult *result, size_t idx);
uint64_t		SMCLOptionsResultParameterUInt64ValueAtIndex(SMCLOptionsResult *result, size_t idx);
int64_t			SMCLOptionsResultParameterInt64ValueAtIndex(SMCLOptionsResult *result, size_t idx);

void SMCLOptionsResultFree(SMCLOptionsResult *result);
