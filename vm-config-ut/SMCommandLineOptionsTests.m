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
	const char	*value;
} SMCLParsedParameterTest;


/*
** SMCommandLineOptionsTests
*/
#pragma mark - SMCommandLineOptionsTests

@interface SMCommandLineOptionsTests : XCTestCase

@end

@implementation SMCommandLineOptionsTests

#pragma mark - Setup

- (void)setUp
{
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
		{ 	.verb_identifier = 2, .identifier = 0, .value = "the-value" },
		{ 	.verb_identifier = 2, .identifier = 1, .value = NULL },
		{ 	.verb_identifier = 2, .identifier = 2, .value = "an-argument" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 2, .identifier = 0, .value = "the-value" },
		{ 	.verb_identifier = 2, .identifier = 1, .value = NULL },
		{ 	.verb_identifier = 2, .identifier = 2, .value = "an-argument" },
		{ 	.verb_identifier = 2, .identifier = 3, .value = NULL },
		{ 	.verb_identifier = 2, .identifier = 4, .value = "optional-argument" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 2, .identifier = 0, .value = "the-value" },
		{ 	.verb_identifier = 2, .identifier = 2, .value = "an-argument" },
		{ 	.verb_identifier = 2, .identifier = 1, .value = NULL },
		{ 	.verb_identifier = 2, .identifier = 4, .value = "optional-argument" },
		{ 	.verb_identifier = 2, .identifier = 3, .value = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 2, .identifier = 0, .value = "the-value" },
		{ 	.verb_identifier = 2, .identifier = 2, .value = "an-argument" },
		{ 	.verb_identifier = 2, .identifier = 1, .value = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 3, .identifier = 5, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 6, .value = "value-1" },
		{ 	.verb_identifier = 3, .identifier = 7, .value = "value-2" },
		{ 	.verb_identifier = 3, .identifier = 8, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 9, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 10, .value = "value-3" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 3, .identifier = 5, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 6, .value = "value-1" },
		{ 	.verb_identifier = 3, .identifier = 7, .value = "value-2" },
		{ 	.verb_identifier = 3, .identifier = 9, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 8, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 10, .value = "value-3" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 3, .identifier = 5, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 6, .value = "value-1" },
		{ 	.verb_identifier = 3, .identifier = 7, .value = "value-2" },
		{ 	.verb_identifier = 3, .identifier = 8, .value = NULL },
		{ 	.verb_identifier = 3, .identifier = 10, .value = "value-3" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 5, .identifier = 12, .value = NULL },
		{ 	.verb_identifier = 5, .identifier = 13, .value = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 5, .identifier = 12, .value = NULL },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
		{ 	.verb_identifier = 4, .identifier = 11, .value = "my_value" },
	};
	
	[self validateResult:result testParameters:expectedResult count:sizeof(expectedResult) / sizeof(*expectedResult)];
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
	SMCLOptionsVerbAddOptionWithArgument(verb2, 2, false,	"v2-mandatory-value",	'a',	SMCLParameterArgumentTypeString, "v2-arg-name",	"Option 2 #token-v2-option2");
	SMCLOptionsVerbAddOption(verb2,				3, true,	"v2-optional-nvalue",	0,														"Option 3 #token-v2-option3");
	SMCLOptionsVerbAddOptionWithArgument(verb2, 4, true,	"v2-optional-value",	0,		SMCLParameterArgumentTypeString, NULL, 			"Option 4 #token-v2-option4");

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

	return options;
}

- (void)validateResult:(SMCLOptionsResult *)result testParameters:(SMCLParsedParameterTest *)parameters count:(size_t)count
{
	XCTAssertEqual(SMCLOptionsResultParametersCount(result), count);

	uint64_t verb_identifier = SMCLOptionsResultVerbIdentifier(result);

	for (size_t i = 0; i < count; i++)
	{
		uint64_t	identifier = SMCLOptionsResultParameterIdentifierAtIndex(result, i);
		const char	*value = SMCLOptionsResultParameterValueAtIndex(result, i);

		XCTAssertEqual(verb_identifier, parameters[i].verb_identifier);
		XCTAssertEqual(identifier, parameters[i].identifier);

		if ((parameters[i].value == NULL && value != NULL) || (parameters[i].value != NULL && value == NULL))
			XCTFail(@"unexpected value pointer");

		if (parameters[i].value && value)
			XCTAssertEqualStrings(parameters[i].value, value);
	}
}

@end
