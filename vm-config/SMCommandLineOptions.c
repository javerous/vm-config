/*
 *  SMCommandLineOptions.c
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
#include <assert.h>
#include <ctype.h>
#include <sys/param.h>

#include "SMCommandLineOptions.h"


/*
** Defines
*/
#pragma mark - Defines

#define SMCLUsageVerbIdentation 				2
#define SMCLUsageParametersIdentation			4
#define SMCLUsageDescriptionMinimalAlignement	4
#define SMCLUsageParameterDescriptionOffset		2


/*
** Types
*/
#pragma mark - Types

typedef enum
{
	SMCLOptionsParameterTypeValue,
	SMCLOptionsParameterTypeOption,
} SMCLOptionsParameterType;

typedef enum
{
	SMCLOptionsArgumentTypeInvalid,
	SMCLOptionsArgumentTypeValue,
	SMCLOptionsArgumentTypeLongName,
	SMCLOptionsArgumentTypeShortName
} SMCLOptionsArgumentType;

typedef struct
{
	SMCLOptionsParameterType type;
	
	uint64_t identifier;
	
	bool optional;
	
	char	*name;
	char	short_name;
	
	bool						has_argument;
	SMCLParameterArgumentType	argument_type;
	char						*argument_name;
	
	char *description;
	
	bool handled;
} SMCLOptionsParameter;

struct SMCLOptions
{
	SMCLOptionsVerb **verbs;
	size_t			verbs_count;
};

struct SMCLOptionsVerb
{
	uint64_t identifier;
	
	char *name;
	char *description;
	
	SMCLOptionsParameter *parameters;
	size_t				parameters_count;
	
};

struct SMCLOptionsResult
{
	uint64_t verb_identifier;
	
	struct {
		uint64_t identifier;
		char *value;
	} *parameters;
	size_t parameters_count;
};


/*
** Globals
*/
#pragma mark - Globals

// Errors.
const char * SMCommanLineOptionsErrorDomain = "com.sourcemac.command-line-options.error";


/*
** Prototypes
*/
#pragma mark - Prototypes

// Parameters.
static SMCLOptionsParameter * SMCLVerbAddParameter(SMCLOptionsVerb *verb);

// Usage.
static char * SMCLFormatParameterName(SMCLOptionsParameter *parameter);

// Parsing.
static SMCLOptionsArgumentType SMCLOptionsSenseArgument(const char *argument, const char **content);


/*
** Instance
*/
#pragma mark - Instance

SMCLOptions * SMCLOptionsCreate(void)
{
	SMCLOptions *result = calloc(1, sizeof(SMCLOptions));
	
	assert(result);
	
	return result;
}

void SMCLOptionsFree(SMCLOptions *options)
{
	for (size_t i = 0; i < options->verbs_count; i++)
	{
		SMCLOptionsVerb *verb = options->verbs[i];
		
		for (size_t j = 0; j < verb->parameters_count; j++)
		{
			SMCLOptionsParameter *parameter = &verb->parameters[j];
			
			free(parameter->name);
			free(parameter->description);
			free(parameter->argument_name);
		}
		
		free(verb->name);
		free(verb->description);
		free(verb->parameters);
		free(verb);
	}
	
	free(options->verbs);
	
	free(options);
}


/*
** Verbs
*/
#pragma mark - Verbs

SMCLOptionsVerb * SMCLOptionsAddVerb(SMCLOptions *options, uint64_t identifier, const char *name, const char *description)
{
	SMCLOptionsVerb *result = calloc(1, sizeof(SMCLOptionsVerb));
	
	options->verbs = reallocf(options->verbs, (options->verbs_count + 1) * sizeof(SMCLOptionsVerb));
	
	assert(options->verbs);
	
	options->verbs[options->verbs_count++] = result;
		
	result->identifier = identifier;
	result->name = strdup(name);
	result->description = strdup(description);
	
	return result;
}


/*
** Parameters
*/
#pragma mark - Parameters

#pragma mark > Interface

void SMCLOptionsVerbAddValue(SMCLOptionsVerb *verb, uint64_t identifier, const char *name, const char *description)
{
	SMCLOptionsParameter *parameter = SMCLVerbAddParameter(verb);
	
	parameter->type = SMCLOptionsParameterTypeValue;
	
	parameter->identifier = identifier;
	parameter->name = strdup(name);
	parameter->description =strdup(description);
}

void SMCLOptionsVerbAddOption(SMCLOptionsVerb *verb, uint64_t identifier, bool optional, const char *name, char short_name, const char *description)
{
	SMCLOptionsParameter *parameter = SMCLVerbAddParameter(verb);
	
	parameter->type = SMCLOptionsParameterTypeOption;
	
	parameter->identifier = identifier;
	parameter->optional = optional;
	parameter->name = strdup(name);
	parameter->short_name = short_name;
	parameter->has_argument = false;
	parameter->description =strdup(description);
}

void SMCLOptionsVerbAddOptionWithArgument(SMCLOptionsVerb *verb, uint64_t identifier, bool optional, const char *name, char short_name, SMCLParameterArgumentType argument_type, const char *argument_name, const char *description)
{
	SMCLOptionsParameter *parameter = SMCLVerbAddParameter(verb);
	
	parameter->type = SMCLOptionsParameterTypeOption;
	
	parameter->identifier = identifier;
	parameter->optional = optional;
	parameter->name = strdup(name);
	parameter->short_name = short_name;
	parameter->has_argument = true;
	parameter->argument_type = argument_type;
	parameter->argument_name = (argument_name ? strdup(argument_name) : NULL);
	parameter->description =strdup(description);
}


#pragma mark > Helpers

static SMCLOptionsParameter * SMCLVerbAddParameter(SMCLOptionsVerb *verb)
{
	SMCLOptionsParameter *result;
	
	verb->parameters = reallocf(verb->parameters, (verb->parameters_count + 1) * sizeof(SMCLOptionsParameter));
	
	assert(verb->parameters);
	
	result = &verb->parameters[verb->parameters_count];
	verb->parameters_count++;
	
	memset(result, 0, sizeof(SMCLOptionsParameter));
	
	return result;
}


/*
** Usage
*/
#pragma mark - Usage

#pragma mark > Interface

void SMCLOptionsPrintUsage(SMCLOptions *options, FILE *output)
{
	fprintf(output, "Usage: %s <verb> <options>, where <verb> and <options> are as follows:\n", getprogname());
	fprintf(output, "\n");
	
	// Compute max left size
	size_t max_left_size = 0;

	for (size_t i = 0; i < options->verbs_count; i++)
	{
		SMCLOptionsVerb *verb = options->verbs[i];
		
		max_left_size = MAX(max_left_size, SMCLUsageVerbIdentation + strlen(verb->name));
		
		for (size_t j = 0; j < verb->parameters_count; j++)
		{
			SMCLOptionsParameter *parameter = &verb->parameters[j];
			
			char *formatted_param = SMCLFormatParameterName(parameter);
			
			max_left_size = MAX(max_left_size, SMCLUsageParametersIdentation + strlen(formatted_param));
			
			free(formatted_param);
		}
	}
	
	max_left_size += SMCLUsageDescriptionMinimalAlignement;
	
	// Print.
	for (size_t i = 0; i < options->verbs_count; i++)
	{
		SMCLOptionsVerb	*verb = options->verbs[i];
		size_t			right_verb_padding_size = max_left_size - (SMCLUsageVerbIdentation + strlen(verb->name));
		char 			*left_verb_padding = malloc(SMCLUsageVerbIdentation + 1);
		char 			*right_verb_padding = malloc(right_verb_padding_size + 1);
		
		assert(left_verb_padding);
		assert(right_verb_padding);
		
		memset(left_verb_padding, ' ', SMCLUsageVerbIdentation);
		memset(right_verb_padding, ' ', right_verb_padding_size);
		left_verb_padding[SMCLUsageVerbIdentation] = 0;
		right_verb_padding[right_verb_padding_size] = 0;
		
		fprintf(output, "%s%s%s%s\n", left_verb_padding, verb->name, right_verb_padding, verb->description);
		
		free(left_verb_padding);
		free(right_verb_padding);

		for (size_t j = 0; j < verb->parameters_count; j++)
		{
			SMCLOptionsParameter	*parameter = &verb->parameters[j];
			char 					*formatted_param = SMCLFormatParameterName(parameter);
			size_t					right_param_padding_size = max_left_size - (SMCLUsageParametersIdentation + strlen(formatted_param)) + SMCLUsageParameterDescriptionOffset;
			char 					*left_param_padding = malloc(SMCLUsageParametersIdentation + 1);
			char 					*right_param_padding = malloc(right_param_padding_size + 1);
			
			assert(left_param_padding);
			assert(right_param_padding);
			
			memset(left_param_padding, ' ', SMCLUsageParametersIdentation);
			memset(right_param_padding, ' ', right_param_padding_size);
			
			left_param_padding[SMCLUsageParametersIdentation] = 0;
			right_param_padding[right_param_padding_size] = 0;

			fprintf(output, "%s%s%s%s\n", left_param_padding, formatted_param, right_param_padding, parameter->description);
						
			free(formatted_param);
			free(left_param_padding);
			free(right_param_padding);
		}
		
		if (i + 1 < options->verbs_count)
			fputc('\n', output);
	}
}


#pragma mark > Helpers

static char * SMCLFormatParameterName(SMCLOptionsParameter *parameter)
{
	char *result = NULL;
	
	switch (parameter->type)
	{
		case SMCLOptionsParameterTypeValue:
		{
			asprintf(&result, "%s", parameter->name);
			break;
		}
			
		case SMCLOptionsParameterTypeOption:
		{
			// > Forge argument name part.
			char *argument_name = NULL;

			if (parameter->has_argument)
			{
				const char *default_argument_name = NULL;

				switch (parameter->argument_type)
				{
					case SMCLParameterArgumentTypeString:
						default_argument_name = "string";
						break;
				}
				
				asprintf(&argument_name, " <%s>", parameter->argument_name ?: default_argument_name);
			}
			
			// > Forge optional decoration.
			const char *optional_open = "";
			const char *optional_close = "";

			if (parameter->optional)
			{
				optional_open = "[";
				optional_close = "]";
			}
			
			// > Forge whole argument.
			if (isprint(parameter->short_name))
				asprintf(&result, "%s-%c, --%s%s%s", optional_open, parameter->short_name, parameter->name, argument_name ?: "", optional_close);
			else
				asprintf(&result, "%s--%s%s%s", optional_open, parameter->name, argument_name ?: "", optional_close);
			
			free(argument_name);
			
			break;
		}
	}
	
	return result;
}


/*
** Parsing
*/
#pragma mark - Parsing

SMCLOptionsResult * SMCLOptionsParse(SMCLOptions *options, int argc, const char * argv[], SMError **error)
{
	SMCLOptionsResult		*result = calloc(1, sizeof(SMCLOptionsResult));
	SMCLOptionsParameter	*parameters = NULL;
	
	// Check input.
	if (argc <= 0 || !argv)
	{
		SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseInvalidArgumentsArray, "invalid argument array passed");
		goto fail;
	}
	
	if (argc == 1)
	{
		SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseMissingVerb, "missing verb");
		goto fail;
	}
	
	// Search verb.
	const char		*verb_str = argv[1];
	SMCLOptionsVerb	*verb = NULL;
	
	for (size_t i = 0; i < options->verbs_count; i++)
	{
		if (strcmp(verb_str, options->verbs[i]->name) == 0)
		{
			verb = options->verbs[i];
			break;
		}
	}
	
	if (!verb)
	{
		SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseUnknowVerb, "unknow verb '%s'", verb_str);
		goto fail;
	}
	
	result->verb_identifier = verb->identifier;
	
	// Create modifiable copy of parameters.
	parameters = malloc(verb->parameters_count * sizeof(SMCLOptionsParameter));
	assert(parameters);
	memcpy(parameters, verb->parameters, verb->parameters_count * sizeof(SMCLOptionsParameter));
	
	// Handle arguments.
	size_t param_idx = 0;

	for (int arg_idx = 2; arg_idx < argc; arg_idx++)
	{
		const char	*arg = argv[arg_idx];
		const char	*param_value = NULL;
		uint64_t	param_idenfier = 0;
		
		// > Sense type of argument.
		const char 				*arg_content = NULL;
		SMCLOptionsArgumentType	arg_type = SMCLOptionsSenseArgument(arg, &arg_content);
				
		switch (arg_type)
		{
			// > Invalid argument: fail.
			case SMCLOptionsArgumentTypeInvalid:
			{
				SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseUnexpectedArgument, "unexpected argument '%s'", arg);
				goto fail;
			}
			
			// > Value: valid if we still have a value to match, after pre-optional paramaters.
			case SMCLOptionsArgumentTypeValue:
			{
				// > Skip optionals.
				for (; param_idx < verb->parameters_count && parameters[param_idx].optional; param_idx++)
					;
				
				// > Check we still have parameters to handle.
				if (param_idx >= verb->parameters_count)
				{
					SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseUnexpectedExtraArgument, "unexpected extra argument value '%s'", arg);
					goto fail;
				}

				// > Handle only if we are stopped on a value parameter. This will generate an error when we check if all non-option parameters have been handled.
				if (parameters[param_idx].type != SMCLOptionsParameterTypeValue)
					continue;
				
				// > Handle parameter.
				param_value = arg;
				param_idenfier = parameters[param_idx].identifier;
				
				// > Mark parameter as handled.
				parameters[param_idx].handled = true;
				
				break;
			}
			
			// > Option: valid between 2 values (or globally valid if there is no values).
			case SMCLOptionsArgumentTypeLongName:
			case SMCLOptionsArgumentTypeShortName:
			{
				// > Check we still have parameters to handle.
				if (param_idx >= verb->parameters_count)
				{
					SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseUnexpectedExtraArgument, "unexpected extra option '%s'", arg);
					goto fail;
				}

				// > Search matching parameter.
				SMCLOptionsParameter *match_parameter = NULL;

				for (size_t i = param_idx; i < verb->parameters_count; i++)
				{
					// > Stop on first value: options are valid between values parameter, to enforce values ordering relatively to parameters.
					if (parameters[i].type == SMCLOptionsParameterTypeValue)
						break;
					
					// > Match option name.
					if ((arg_type == SMCLOptionsArgumentTypeShortName && *arg_content != parameters[i].short_name))
						continue;
					
					if ((arg_type == SMCLOptionsArgumentTypeLongName && strcmp(arg_content, parameters[i].name) != 0))
						continue;
					
					match_parameter = &parameters[i];
					
					break;
				}
				
				// > Didn't find anything: fail.
				if (!match_parameter)
				{
					SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseUnkownOption, "unknow option '%s'", arg);
					goto fail;
				}
				
				// > Handle option argument.
				if (match_parameter->has_argument)
				{
					if (arg_idx + 1 >= argc)
					{
						SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseMissingOptionArgument, "missing argument for option '%s'", arg);
						goto fail;
					}
					
					// > Handle argument type.
					const char *argument_str = argv[arg_idx + 1];
										
					arg_idx++;

					switch (match_parameter->argument_type)
					{
						case SMCLParameterArgumentTypeString:
							param_value = argument_str;
							break;
					}
				}
				
				// > Handle parameter.
				param_idenfier = match_parameter->identifier;

				// > Mark parameter as handled.
				match_parameter->handled = true;
				
				break;
			}
		}
		
		// Everything valid: add entry to result.
		result->parameters = reallocf(result->parameters, (result->parameters_count + 1) * sizeof(*result->parameters));
		
		assert(result->parameters);
		
		result->parameters[result->parameters_count].identifier = param_idenfier;
		result->parameters[result->parameters_count].value = param_value ? strdup(param_value) : NULL;
		result->parameters_count++;
		
		// > Skip all handled parameters, so we don't re-handle multiple time same parameters, like values which are not named.
		for (; param_idx < verb->parameters_count && parameters[param_idx].handled; param_idx++)
			;
	}
	
	// Check we handled everything we should.
	for (size_t i = param_idx; i < verb->parameters_count; i++)
	{
		if (parameters[i].optional == false && parameters[i].handled == false)
		{
			switch (parameters[i].type)
			{
				case SMCLOptionsParameterTypeValue:
					SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseMissingParameter, "missing value '%s'", parameters[i].name);
					break;
					
				case SMCLOptionsParameterTypeOption:
					SMSetErrorPtr(error, SMCommanLineOptionsErrorDomain, SMCLErrorParseMissingParameter, "missing option '%s'", parameters[i].name);
					break;
			}
			
			goto fail;
		}
	}

	// Success.
	return result;
		
fail:
	free(parameters);
	SMCLOptionsResultFree(result);
	return NULL;
}

uint64_t SMCLOptionsResultVerbIdentifier(SMCLOptionsResult *result)
{
	return result->verb_identifier;
}

size_t SMCLOptionsResultParametersCount(SMCLOptionsResult *result)
{
	return result->parameters_count;
}

uint64_t SMCLOptionsResultParameterIdentifierAtIndex(SMCLOptionsResult *result, size_t idx)
{
	return result->parameters[idx].identifier;
}

const char * SMCLOptionsResultParameterValueAtIndex(SMCLOptionsResult *result, size_t idx)
{
	return result->parameters[idx].value;
}

void SMCLOptionsResultFree(SMCLOptionsResult *result)
{
	if (!result)
		return;
	
	for (size_t i = 0; i < result->parameters_count; i++)
		free(result->parameters[i].value);
	
	free(result->parameters);
	free(result);
}


#pragma mark > Helpers

static SMCLOptionsArgumentType SMCLOptionsSenseArgument(const char *argument, const char **content)
{
	size_t len = strlen(argument);
	
	if (len > 0 && argument[0] == '-')
	{
		if (len == 2 && isprint(argument[1]) && argument[1] != '-')
		{
			if (content)
				*content = &argument[1];
			
			return SMCLOptionsArgumentTypeShortName;
		}
		else if (len > 2 && argument[1] == '-')
		{
			if (content)
				*content = &argument[2];
			
			return SMCLOptionsArgumentTypeLongName;
		}
		else
			return SMCLOptionsArgumentTypeInvalid;
	}
	
	if (content)
		*content = argument;
	
	return SMCLOptionsArgumentTypeValue;
}
