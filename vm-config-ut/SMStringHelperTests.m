/*
 *  SMStringHelperTests.m
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

#import "SMStringHelper.h"


/*
** Defines
*/
#pragma mark - Defines

#define XCTAssertEqualStrings(FreeableString, ReferenceString) ({	\
	char		*__freeable_str = (FreeableString);					\
	const char	*__ref_str = (ReferenceString);						\
	int			__result = strcmp(__freeable_str, __ref_str);		\
																	\
	free(__freeable_str);											\
																	\
	XCTAssertEqual(__result, 0);									\
})


/*
** SMStringHelperTests
*/
#pragma mark - SMStringHelperTests

@interface SMStringHelperTests : XCTestCase

@end

@implementation SMStringHelperTests

- (void)testStringDuplicate
{
	XCTAssertEqualStrings(SMStringDuplicate("toto", 4), "toto");
	XCTAssertEqualStrings(SMStringDuplicate("toto", 3), "tot");
	XCTAssertEqualStrings(SMStringDuplicate("toto", 0), "");
}

- (void)testStringTrimCharacter
{
	char spaces[] = { ' ' };
	char spaces_plus[] = { ' ', '+' };

	XCTAssertEqualStrings(SMStringTrimCharacter(" abcd", spaces, sizeof(spaces), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("   abcd", spaces, sizeof(spaces), false), "abcd");
	
	XCTAssertEqualStrings(SMStringTrimCharacter("abcd ", spaces, sizeof(spaces), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("abcd   ", spaces, sizeof(spaces), false), "abcd");
	
	XCTAssertEqualStrings(SMStringTrimCharacter(" abcd ", spaces, sizeof(spaces), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("   abcd   ", spaces, sizeof(spaces), false), "abcd");

	XCTAssertEqualStrings(SMStringTrimCharacter("++++abcd", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("++++abcd    ", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("++++  abcd+++ ", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("+ +  ++  abcd  +++   +    +", spaces_plus, sizeof(spaces_plus), false), "abcd");
	
	XCTAssertEqualStrings(SMStringTrimCharacter("abcd", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualStrings(SMStringTrimCharacter("", spaces_plus, sizeof(spaces_plus), false), "");

	XCTAssertEqualStrings(SMStringTrimCharacter("ab cd ef    gh", spaces, sizeof(spaces), false), "ab cd ef    gh");
	XCTAssertEqualStrings(SMStringTrimCharacter("ab cd ef    gh    ", spaces, sizeof(spaces), false), "ab cd ef    gh");
	XCTAssertEqualStrings(SMStringTrimCharacter("    ab cd ef    gh", spaces, sizeof(spaces), false), "ab cd ef    gh");
	XCTAssertEqualStrings(SMStringTrimCharacter("    ab cd ef    gh    ", spaces, sizeof(spaces), false), "ab cd ef    gh");
}

- (void)testStringReplaceString
{
	XCTAssertEqualStrings(SMStringReplaceString("hello world", "", "nothing", false), "hello world");
	XCTAssertEqualStrings(SMStringReplaceString("hello world, beautiful world", "world", "", false), "hello , beautiful ");
	
	XCTAssertEqualStrings(SMStringReplaceString("hello world, beautiful world", "world", "people", false), "hello people, beautiful people");
	
	XCTAssertEqualStrings(SMStringReplaceString("", "world", "people", false), "");
	
	XCTAssertEqualStrings(SMStringReplaceString("hello world", "toto", "tutu", false), "hello world");
}

- (void)testPathAppendComponent
{
	XCTAssertEqualStrings(SMStringPathAppendComponent("", "hello"), "/hello");
	XCTAssertEqualStrings(SMStringPathAppendComponent("", ""), "");
	XCTAssertEqualStrings(SMStringPathAppendComponent("/welcome", ""), "/welcome");
	XCTAssertEqualStrings(SMStringPathAppendComponent("/welcome///", "///world"), "/welcome/world");
	XCTAssertEqualStrings(SMStringPathAppendComponent("/welcome", "///world"), "/welcome/world");
	XCTAssertEqualStrings(SMStringPathAppendComponent("/welcome/", "world"), "/welcome/world");
	XCTAssertEqualStrings(SMStringPathAppendComponent("/welcome", "world"), "/welcome/world");
}

- (void)testPathHasExtension
{
	XCTAssertTrue(SMStringPathHasExtension("toto.txt", "txt"));
	XCTAssertFalse(SMStringPathHasExtension("toto.txt", "pdf"));
	XCTAssertFalse(SMStringPathHasExtension("tototxt", "txt"));
	XCTAssertFalse(SMStringPathHasExtension("tototxt", "pdf"));
	XCTAssertTrue(SMStringPathHasExtension(".txt", "txt"));
	
	XCTAssertFalse(SMStringPathHasExtension("f.p", "txt"));
	
	XCTAssertTrue(SMStringPathHasExtension("toto..txt", "txt"));
	XCTAssertTrue(SMStringPathHasExtension("toto.pdf.txt", "txt"));
	XCTAssertFalse(SMStringPathHasExtension("toto.pdf.txt", "pdf"));
	XCTAssertTrue(SMStringPathHasExtension("toto.txt.pdf", "pdf"));

	XCTAssertFalse(SMStringPathHasExtension("toto.txt", ""));
	XCTAssertFalse(SMStringPathHasExtension("toto", "txt"));
}

@end
