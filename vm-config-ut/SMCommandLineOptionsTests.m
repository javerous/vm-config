/*
 *  SMCommandLineOptionsTests.m
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

#import <XCTest/XCTest.h>

#import "SMCommandLineOptions.h"

#import "SMTestsTools.h"
#import "SMTestCase.h"


/*
** Defines
*/
#pragma mark - Defines

#define XCTAssertSuccess(Result, Error) ({	\
	XCTAssertNotEqual(Result, NULL);		\
	XCTAssertEqual(Error, NULL);			\
})

#define XCTAssertFailure(Result, Error, ErrorCode) ({										\
	SMError	*__error = (Error);																\
	int		__test_error_code = (ErrorCode);												\
																							\
	XCTAssertEqual(Result, NULL);															\
	XCTAssertNotEqual(__error, NULL);														\
																							\
	if (__error)																			\
	{																						\
		XCTAssertEqualStrings(SMErrorGetDomain(__error), SMCommanLineOptionsErrorDomain);	\
		XCTAssertEqual(SMErrorGetCode(__error), __test_error_code);							\
	}																						\
})


/*
** Types
*/
#pragma mark - Types

typedef struct
{
	uint64_t	verb_identifier;

	uint64_t	identifier;
	
	SMCLValueType value_type;
	union {
		char		*str;
		uint64_t	uint64;
		int64_t		int64;
		uint32_t	uint32;
		int32_t		int32;
	} value;
} SMCLParsedParameterTest;


/*
** SMCommandLineOptionsTests
*/
#pragma mark - SMCommandLineOptionsTests

@interface SMCommandLineOptionsTests : SMTestCase

@end

@implementation SMCommandLineOptionsTests

#pragma mark - Setup

- (void)setUp
{
	[super setUp];
	
	self.continueAfterFailure = NO;
}


#pragma mark - Tests

- (void)testUsage
{
	SMDeclareDefaultFiles;
	
	// Get standard options.
	SMCLOptions *options = [self standardOptions];
	
	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Generate usage.
	SMCLOptionsPrintUsage(options, fout);
	fflush(fout);
		
	// Check if it seems valid.
	XCTAssertGreaterThan(sout, 50);

	// > Verb-1.
	XCTAssertContainString(*bout, sout, "verb1");
	XCTAssertContainString(*bout, sout, "#token-verb1");

	// > Verb-2.
	XCTAssertContainString(*bout, sout, "verb2");
	XCTAssertContainString(*bout, sout, "#token-verb2");

	XCTAssertContainString(*bout, sout, "v2-value1");
	XCTAssertContainString(*bout, sout, "#token-v2-value1");

	XCTAssertContainString(*bout, sout, "--v2-mandatory-nvalue");
	XCTAssertContainString(*bout, sout, "#token-v2-option1");

	XCTAssertContainString(*bout, sout, "--v2-mandatory-value");
	XCTAssertContainString(*bout, sout, "-a");
	XCTAssertContainString(*bout, sout, "<v2-arg-name>");
	XCTAssertContainString(*bout, sout, "#token-v2-option2");

	XCTAssertContainString(*bout, sout, "--v2-optional-value");
	XCTAssertContainString(*bout, sout, "<string");
	XCTAssertContainString(*bout, sout, "#token-v2-option4");

	// > Verb-3.
	XCTAssertContainString(*bout, sout, "verb3");
	XCTAssertContainString(*bout, sout, "#token-verb3");

	XCTAssertContainString(*bout, sout, "--v3-pre-option");
	XCTAssertContainString(*bout, sout, "#token-v3-option1");

	XCTAssertContainString(*bout, sout, "v3-value1");
	XCTAssertContainString(*bout, sout, "#token-v3-value1");

	XCTAssertContainString(*bout, sout, "v3-value2");
	XCTAssertContainString(*bout, sout, "#token-v3-value2");

	XCTAssertContainString(*bout, sout, "--v3-post-option-1");
	XCTAssertContainString(*bout, sout, "#token-v3-option2");

	XCTAssertContainString(*bout, sout, "--v3-post-option-2");
	XCTAssertContainString(*bout, sout, "#token-v3-option3");

	XCTAssertContainString(*bout, sout, "v3-value3");
	XCTAssertContainString(*bout, sout, "#token-v3-value3");

	// > Verb-4.
	XCTAssertContainString(*bout, sout, "verb4");
	XCTAssertContainString(*bout, sout, "#token-verb4");

	XCTAssertContainString(*bout, sout, "v4-value1");
	XCTAssertContainString(*bout, sout, "#token-v4-value1");

	// > Verb-5.
	XCTAssertContainString(*bout, sout, "verb5");
	XCTAssertContainString(*bout, sout, "#token-verb5");

	XCTAssertContainString(*bout, sout, "v4-value1");
	XCTAssertContainString(*bout, sout, "#token-v5-option1");
	XCTAssertContainString(*bout, sout, "#token-v5-option2");
}

- (void)testErrorInvalidArgumentsArray
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, 0, NULL, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseInvalidArgumentsArray);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorNoVerb
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingVerb);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorUnknowVerb
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"unknow"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnknowVerb);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorUnexpectedArgument
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb5",
		"-v5-option2"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnexpectedArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorUnexpectedExtraArgument
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb1",
		"--v5-option2"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnexpectedExtraArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorUnkownOption1
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb5",
		"--v5-option3"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnkownOption);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorUnkownOption2
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"the-value",
		"--v2-mandatory-nvalue",
		"-b",
		"hello"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnkownOption);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorMissingOptionArgument
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"<v2-value1>",
		"--v2-mandatory-value"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingOptionArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorMissingNonOptional
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"<v2-value1>",
		"--v2-mandatory-value"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingOptionArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testErrorMissingParameter
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];
	
	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb5",
		"--v5-option2"
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);
	
	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingParameter);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testVerbNoParameters
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb1"
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	XCTAssertEqual(SMCLOptionsResultVerbIdentifier(result), 1);

	// Validate result.
	[self validateResult:result testParameters:NULL count:0];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse1
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"the-value",
		"--v2-mandatory-nvalue",
		"--v2-mandatory-value",
		"an-argument",
		
	};

	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 2, .identifier = 0, .value_type = SMCLValueTypeString, .value.str = "the-value" },
		{ .verb_identifier = 2, .identifier = 1, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 2, .identifier = 2, .value_type = SMCLValueTypeString, .value.str = "an-argument" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse2
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"the-value",
		"--v2-mandatory-nvalue",
		"-a",
		"an-argument",
		"--v2-optional-nvalue",
		"--v2-optional-value",
		"optional-argument"
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 2, .identifier = 0, .value_type = SMCLValueTypeString, .value.str = "the-value" },
		{ .verb_identifier = 2, .identifier = 1, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 2, .identifier = 2, .value_type = SMCLValueTypeString, .value.str = "an-argument" },
		{ .verb_identifier = 2, .identifier = 3, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 2, .identifier = 4, .value_type = SMCLValueTypeString, .value.str = "optional-argument" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse3
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"the-value",
		"-a",
		"an-argument",
		"--v2-mandatory-nvalue",
		"--v2-optional-value",
		"optional-argument",
		"--v2-optional-nvalue",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 2, .identifier = 0, .value_type = SMCLValueTypeString, .value.str = "the-value" },
		{ .verb_identifier = 2, .identifier = 2, .value_type = SMCLValueTypeString, .value.str = "an-argument" },
		{ .verb_identifier = 2, .identifier = 1, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 2, .identifier = 4, .value_type = SMCLValueTypeString, .value.str = "optional-argument" },
		{ .verb_identifier = 2, .identifier = 3, .value_type = SMCLValueTypeString, .value.str = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse4
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"the-value",
		"--v2-mandatory-value",
		"an-argument",
		"--v2-mandatory-nvalue",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 2, .identifier = 0, .value_type = SMCLValueTypeString, .value.str = "the-value" },
		{ .verb_identifier = 2, .identifier = 2, .value_type = SMCLValueTypeString, .value.str = "an-argument" },
		{ .verb_identifier = 2, .identifier = 1, .value_type = SMCLValueTypeString, .value.str = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse5
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"--v3-pre-option",
		"value-1",
		"value-2",
		"--v3-post-option-1",
		"--v3-post-option-2",
		"value-3",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 3, .identifier = 5, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 6, .value_type = SMCLValueTypeString, .value.str = "value-1" },
		{ .verb_identifier = 3, .identifier = 7, .value_type = SMCLValueTypeString, .value.str = "value-2" },
		{ .verb_identifier = 3, .identifier = 8, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 9, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 10, .value_type = SMCLValueTypeString, .value.str = "value-3" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse6
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"--v3-pre-option",
		"value-1",
		"value-2",
		"--v3-post-option-2",
		"--v3-post-option-1",
		"value-3",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 3, .identifier = 5, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 6, .value_type = SMCLValueTypeString, .value.str = "value-1" },
		{ .verb_identifier = 3, .identifier = 7, .value_type = SMCLValueTypeString, .value.str = "value-2" },
		{ .verb_identifier = 3, .identifier = 9, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 8, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 10, .value_type = SMCLValueTypeString,  .value.str = "value-3" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse7
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"--v3-pre-option",
		"value-1",
		"value-2",
		"--v3-post-option-1",
		"value-3",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 3, .identifier = 5, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 6, .value_type = SMCLValueTypeString, .value.str = "value-1" },
		{ .verb_identifier = 3, .identifier = 7, .value_type = SMCLValueTypeString, .value.str = "value-2" },
		{ .verb_identifier = 3, .identifier = 8, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 3, .identifier = 10, .value_type = SMCLValueTypeString, .value.str = "value-3" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse8
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb5",
		"--v5-option1",
		"--v5-option2",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 5, .identifier = 12, .value_type = SMCLValueTypeString, .value.str = NULL },
		{ .verb_identifier = 5, .identifier = 13, .value_type = SMCLValueTypeString, .value.str = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse9
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb5",
		"--v5-option1",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 5, .identifier = 12, .value_type = SMCLValueTypeString, .value.str = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParse10
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb4",
		"my_value",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 4, .identifier = 11, .value_type = SMCLValueTypeString, .value.str = "my_value" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseTypes1
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb6",
		"--v6-string", "hello",
		"--v6-uint32", "42",
		"--v6-int32", "-43",
		"--v6-uint64", "44",
		"--v6-int64", "-45",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 6, .identifier = 14, .value_type = SMCLValueTypeString, .value.str = "hello" },
		{ .verb_identifier = 6, .identifier = 15, .value_type = SMCLValueTypeUInt32, .value.uint32 = 42 },
		{ .verb_identifier = 6, .identifier = 16, .value_type = SMCLValueTypeInt32, .value.int32 = -43 },
		{ .verb_identifier = 6, .identifier = 17, .value_type = SMCLValueTypeUInt64, .value.uint64 = 44 },
		{ .verb_identifier = 6, .identifier = 18, .value_type = SMCLValueTypeInt64, .value.int64 = -45 },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseTypes2
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb6",
		"--v6-string", "hello",
		"--v6-uint32", "0xab",
		"--v6-int32", "-0xac",
		"--v6-uint64", "0xad",
		"--v6-int64", "-0xae",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 6, .identifier = 14, .value_type = SMCLValueTypeString, .value.str = "hello" },
		{ .verb_identifier = 6, .identifier = 15, .value_type = SMCLValueTypeUInt32, .value.uint32 = 0xab },
		{ .verb_identifier = 6, .identifier = 16, .value_type = SMCLValueTypeInt32, .value.int32 = -0xac },
		{ .verb_identifier = 6, .identifier = 17, .value_type = SMCLValueTypeUInt64, .value.uint64 = 0xad },
		{ .verb_identifier = 6, .identifier = 18, .value_type = SMCLValueTypeInt64, .value.int64 = -0xae },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseTypes3
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb6",
		"--v6-string", "hello",
		"--v6-uint32", "0755",
		"--v6-int32", "-0666",
		"--v6-uint64", "0644",
		"--v6-int64", "-0777",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 6, .identifier = 14, .value_type = SMCLValueTypeString, .value.str = "hello" },
		{ .verb_identifier = 6, .identifier = 15, .value_type = SMCLValueTypeUInt32, .value.uint32 = 0755 },
		{ .verb_identifier = 6, .identifier = 16, .value_type = SMCLValueTypeInt32, .value.int32 = -0666 },
		{ .verb_identifier = 6, .identifier = 17, .value_type = SMCLValueTypeUInt64, .value.uint64 = 0644 },
		{ .verb_identifier = 6, .identifier = 18, .value_type = SMCLValueTypeInt64, .value.int64 = -0777 },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseTypes4
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb6",
		"--v6-string", "hello",
		"--v6-uint32", "4294967295",
		"--v6-int32", "-2147483648",
		"--v6-uint64", "18446744073709551615",
		"--v6-int64", "-9223372036854775808",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertSuccess(result, error);
	
	// Validate result.
	SMCLParsedParameterTest expectedResult[] = {
		{ .verb_identifier = 6, .identifier = 14, .value_type = SMCLValueTypeString, .value.str = "hello" },
		{ .verb_identifier = 6, .identifier = 15, .value_type = SMCLValueTypeUInt32, .value.uint32 = UINT32_MAX },
		{ .verb_identifier = 6, .identifier = 16, .value_type = SMCLValueTypeInt32, .value.int32 = INT32_MIN },
		{ .verb_identifier = 6, .identifier = 17, .value_type = SMCLValueTypeUInt64, .value.uint64 = UINT64_MAX },
		{ .verb_identifier = 6, .identifier = 18, .value_type = SMCLValueTypeInt64, .value.int64 = INT64_MIN },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError1
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];
	
	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb2",
		"--v2-mandatory-nvalue",
		"the-value",
		"-a",
		"an-argument",
		"--v2-optional-nvalue",
		"--v2-optional-value",
		"optional-argument"
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);
	
	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnkownOption);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError2
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"value-1",
		"--v3-pre-option",
		"value-2",
		"--v3-post-option-1",
		"value-3",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnkownOption);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError3
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"--v3-pre-option",
		"value-1",
		"value-2",
		"value-3",
		"--v3-post-option-1",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingParameter);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError4
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"--v3-pre-option",
		"value-1",
		"value-2",
		"--v3-post-option-1",
		"value-3",
		"--v3-post-option-2",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnexpectedExtraArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError5
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb3",
		"--v3-pre-option",
		"value-1",
		"value-2",
		"--v3-post-option-1",
		"--v3-post-option-2",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingParameter);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError6
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb4",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseMissingParameter);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError7
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb4",
		"my_value",
		"--option"
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnexpectedExtraArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError8
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb4",
		"--option",
		"my_value",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnkownOption);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError9
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb5",
		"--v5-option1",
		"--v5-option1",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnkownOption);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseError10
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};

	// Parse.
	const char *argv[] = {
		"ut-main",
		"verb4",
		"value1",
		"value2",
	};
	
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, sizeof(argv) / sizeof(*argv), argv, &error);

	// Test result.
	XCTAssertFailure(result, error, SMCLErrorParseUnexpectedExtraArgument);
	
	// Clean.
	SMErrorFree(error);
	SMCLOptionsResultFree(result);
}

- (void)testParseTypesError
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Test octal error.
	const char *argv1[] = { "ut-main", "verb6", "--v6-uint32", "088" };
	
	SMError				*error1 = NULL;
	SMCLOptionsResult	*result1 = SMCLOptionsParse(options, sizeof(argv1) / sizeof(*argv1), argv1, &error1);
	
	XCTAssertFailure(result1, error1, SMCLErrorParseInvalidOptionArgument);
	
	// Test hexa error.
	const char *argv2[] = { "ut-main", "verb6", "--v6-uint32", "0xag" };
	
	SMError				*error2 = NULL;
	SMCLOptionsResult	*result2 = SMCLOptionsParse(options, sizeof(argv2) / sizeof(*argv2), argv2, &error2);
	
	XCTAssertFailure(result2, error2, SMCLErrorParseInvalidOptionArgument);
	
	// Clean.
	SMErrorFree(error1);
	SMCLOptionsResultFree(result1);
	SMErrorFree(error2);
	SMCLOptionsResultFree(result2);
}

- (void)testParseTypesSignedError
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Test uint32 signed error.
	const char *argv1[] = { "ut-main", "verb6", "--v6-uint32", "-42" };
	
	SMError				*error1 = NULL;
	SMCLOptionsResult	*result1 = SMCLOptionsParse(options, sizeof(argv1) / sizeof(*argv1), argv1, &error1);
	
	XCTAssertFailure(result1, error1, SMCLErrorParseInvalidOptionArgument);
	
	// Test uint64 signed error.
	const char *argv2[] = { "ut-main", "verb6", "--v6-uint64", "-51" };
	
	SMError				*error2 = NULL;
	SMCLOptionsResult	*result2 = SMCLOptionsParse(options, sizeof(argv2) / sizeof(*argv2), argv2, &error2);
	
	XCTAssertFailure(result2, error2, SMCLErrorParseInvalidOptionArgument);
	
	// Clean.
	SMErrorFree(error1);
	SMCLOptionsResultFree(result1);
	SMErrorFree(error2);
	SMCLOptionsResultFree(result2);
}

- (void)testParseTypesUnsignedOverflowError
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Test uint32 overflow error.
	const char *argv1[] = { "ut-main", "verb6", "--v6-uint32", "4294967296" };
	
	SMError				*error1 = NULL;
	SMCLOptionsResult	*result1 = SMCLOptionsParse(options, sizeof(argv1) / sizeof(*argv1), argv1, &error1);
	
	XCTAssertFailure(result1, error1, SMCLErrorParseInvalidOptionArgument);
	
	// Test uint64 overflow error.
	const char *argv2[] = { "ut-main", "verb6", "--v6-uint64", "18446744073709551616" };
	
	SMError				*error2 = NULL;
	SMCLOptionsResult	*result2 = SMCLOptionsParse(options, sizeof(argv2) / sizeof(*argv2), argv2, &error2);
	
	XCTAssertFailure(result2, error2, SMCLErrorParseInvalidOptionArgument);
	
	// Clean.
	SMErrorFree(error1);
	SMCLOptionsResultFree(result1);
	SMErrorFree(error2);
	SMCLOptionsResultFree(result2);
}

- (void)testParseTypesSignedOverflowError
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Test uint32 overflow error.
	const char *argv1[] = { "ut-main", "verb6", "--v6-int32", "2147483648" };
	
	SMError				*error1 = NULL;
	SMCLOptionsResult	*result1 = SMCLOptionsParse(options, sizeof(argv1) / sizeof(*argv1), argv1, &error1);
	
	XCTAssertFailure(result1, error1, SMCLErrorParseInvalidOptionArgument);
	
	// Test uint64 overflow error.
	const char *argv2[] = { "ut-main", "verb6", "--v6-int64", "9223372036854775808" };
	
	SMError				*error2 = NULL;
	SMCLOptionsResult	*result2 = SMCLOptionsParse(options, sizeof(argv2) / sizeof(*argv2), argv2, &error2);
	
	XCTAssertFailure(result2, error2, SMCLErrorParseInvalidOptionArgument);
	
	// Clean.
	SMErrorFree(error1);
	SMCLOptionsResultFree(result1);
	SMErrorFree(error2);
	SMCLOptionsResultFree(result2);
}

- (void)testParseTypesSignedUnderflowError
{
	// Get standard options.
	SMCLOptions *options = [self standardOptions];

	_onExit {
		SMCLOptionsFree(options);
	};
	
	// Test uint32 overflow error.
	const char *argv1[] = { "ut-main", "verb6", "--v6-int32", "-2147483649" };
	
	SMError				*error1 = NULL;
	SMCLOptionsResult	*result1 = SMCLOptionsParse(options, sizeof(argv1) / sizeof(*argv1), argv1, &error1);
	
	XCTAssertFailure(result1, error1, SMCLErrorParseInvalidOptionArgument);
	
	// Test uint64 overflow error.
	const char *argv2[] = { "ut-main", "verb6", "--v6-int64", "-9223372036854775809" };
	
	SMError				*error2 = NULL;
	SMCLOptionsResult	*result2 = SMCLOptionsParse(options, sizeof(argv2) / sizeof(*argv2), argv2, &error2);
	
	XCTAssertFailure(result2, error2, SMCLErrorParseInvalidOptionArgument);
	
	// Clean.
	SMErrorFree(error1);
	SMCLOptionsResultFree(result1);
	SMErrorFree(error2);
	SMCLOptionsResultFree(result2);
}

#pragma mark - Helpers

- (SMCLOptions *)standardOptions
{
	SMCLOptions *options = SMCLOptionsCreate();

	// > Verb-1.
	SMCLOptionsVerb *verb1 = SMCLOptionsAddVerb(options, 1, "verb1", "This is an empty verb #token-verb1");

	(void)verb1;

	// > Verb-2.
	SMCLOptionsVerb *verb2 = SMCLOptionsAddVerb(options, 2, "verb2", "This is verb2 #token-verb2");

	SMCLOptionsVerbAddValue(verb2,				0,			"v2-value1",																	"Anonymous value 1 #token-v2-value1");
	SMCLOptionsVerbAddOption(verb2,				1, false,	"v2-mandatory-nvalue",	0,														"Option 1 #token-v2-option1");
	SMCLOptionsVerbAddOptionWithArgument(verb2, 2, false,	"v2-mandatory-value",	'a',	SMCLValueTypeString, "v2-arg-name",	"Option 2 #token-v2-option2");
	SMCLOptionsVerbAddOption(verb2,				3, true,	"v2-optional-nvalue",	0,														"Option 3 #token-v2-option3");
	SMCLOptionsVerbAddOptionWithArgument(verb2, 4, true,	"v2-optional-value",	0,		SMCLValueTypeString, NULL, 			"Option 4 #token-v2-option4");

	// > Verb-3.
	SMCLOptionsVerb *verb3 = SMCLOptionsAddVerb(options, 3, "verb3", "This is verb3 #token-verb3");

	SMCLOptionsVerbAddOption(verb3,				5, false,	"v3-pre-option",		0,														"Option 1 #token-v3-option1");
	SMCLOptionsVerbAddValue(verb3,				6,			"v3-value1",																	"Anonymous value 1 #token-v3-value1");
	SMCLOptionsVerbAddValue(verb3,				7,			"v3-value2",																	"Anonymous value 2 #token-v3-value2");
	SMCLOptionsVerbAddOption(verb3,				8, false,	"v3-post-option-1",		0,														"Option 2 #token-v3-option2");
	SMCLOptionsVerbAddOption(verb3,				9, true,	"v3-post-option-2",		0,														"Option 3 #token-v3-option3");
	SMCLOptionsVerbAddValue(verb3,				10,			"v3-value3",																	"Anonymous value 3 #token-v3-value3");

	// > Verb-4.
	SMCLOptionsVerb *verb4 = SMCLOptionsAddVerb(options, 4, "verb4", "This is verb4 #token-verb4");

	SMCLOptionsVerbAddValue(verb4,				11,			"v4-value1",																	"Anonymous value 1 #token-v4-value1");

	// > Verb-5.
	SMCLOptionsVerb *verb5 = SMCLOptionsAddVerb(options, 5, "verb5", "This is verb5 #token-verb5");

	SMCLOptionsVerbAddOption(verb5,				12, false,	"v5-option1",		0,															"Option 1 #token-v5-option1");
	SMCLOptionsVerbAddOption(verb5,				13, true,	"v5-option2",		0,															"Option 2 #token-v5-option2");
	
	// > Verb-6.
	SMCLOptionsVerb *verb6 = SMCLOptionsAddVerb(options, 6, "verb6", "This is verb6");

	SMCLOptionsVerbAddOptionWithArgument(verb6, 14, true,	"v6-string",	0,	SMCLValueTypeString, 	NULL,	"String Option");
	SMCLOptionsVerbAddOptionWithArgument(verb6, 15, true,	"v6-uint32",	0,	SMCLValueTypeUInt32, 	NULL,	"UInt32 option");
	SMCLOptionsVerbAddOptionWithArgument(verb6, 16, true,	"v6-int32",		0,	SMCLValueTypeInt32, 	NULL,	"Int32 Option");
	SMCLOptionsVerbAddOptionWithArgument(verb6, 17, true,	"v6-uint64",	0,	SMCLValueTypeUInt64,	NULL,	"UInt64 option");
	SMCLOptionsVerbAddOptionWithArgument(verb6, 18, true,	"v6-int64",		0,	SMCLValueTypeInt64,		NULL,	"Int64 option");

	return options;
}

- (void)validateResult:(SMCLOptionsResult *)result testParameters:(SMCLParsedParameterTest *)parameters count:(size_t)count
{
	XCTAssertEqual(SMCLOptionsResultParametersCount(result), count);

	uint64_t verb_identifier = SMCLOptionsResultVerbIdentifier(result);

	for (size_t i = 0; i < count; i++)
	{
		uint64_t 		identifier = SMCLOptionsResultParameterIdentifierAtIndex(result, i);
		SMCLValueType	type = SMCLOptionsResultParameterTypeAtIndex(result, i);
		
		XCTAssertEqual(verb_identifier, parameters[i].verb_identifier);
		XCTAssertEqual(identifier, parameters[i].identifier);
		XCTAssertEqual(type, parameters[i].value_type);
				
		switch (type)
		{
			case SMCLValueTypeString:
			{
				const char	*ref_str = parameters[i].value.str;
				const char	*test_str = SMCLOptionsResultParameterStringValueAtIndex(result, i);
				
				if ((ref_str == NULL && test_str != NULL) || (ref_str != NULL && test_str == NULL))
					XCTFail(@"unexpected value pointer");
				
				if (ref_str && test_str)
					XCTAssertEqualStrings(ref_str, test_str);
				
				break;
			}
				
			case SMCLValueTypeUInt32:
			{
				uint32_t ref_val = parameters[i].value.uint32;
				uint32_t test_val = SMCLOptionsResultParameterUInt32ValueAtIndex(result, i);
				
				XCTAssertEqual(ref_val, test_val);
				
				break;
			}
				
			case SMCLValueTypeInt32:
			{
				int32_t ref_val = parameters[i].value.int32;
				int32_t test_val = SMCLOptionsResultParameterInt32ValueAtIndex(result, i);
				
				XCTAssertEqual(ref_val, test_val);
				
				break;
			}
				
			case SMCLValueTypeUInt64:
			{
				uint64_t ref_val = parameters[i].value.uint64;
				uint64_t test_val = SMCLOptionsResultParameterUInt64ValueAtIndex(result, i);
				
				XCTAssertEqual(ref_val, test_val);
				
				break;
			}
				
			case SMCLValueTypeInt64:
			{
				int64_t ref_val = parameters[i].value.int64;
				int64_t test_val = SMCLOptionsResultParameterInt64ValueAtIndex(result, i);
				
				XCTAssertEqual(ref_val, test_val);
				
				break;
			}
		}
	}
}

@end
