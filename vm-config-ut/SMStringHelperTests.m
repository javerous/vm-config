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

#import "SMTestsTools.h"
#import "SMTestCase.h"


/*
** SMStringHelperTests
*/
#pragma mark - SMStringHelperTests

@interface SMStringHelperTests : SMTestCase

@end

@implementation SMStringHelperTests

- (void)testStringDuplicate
{
	XCTAssertEqualFreeableStrings(SMStringDuplicate("toto", 4), "toto");
	XCTAssertEqualFreeableStrings(SMStringDuplicate("toto", 3), "tot");
	XCTAssertEqualFreeableStrings(SMStringDuplicate("toto", 0), "");
}

- (void)testStringTrimCharacter
{
	char spaces[] = { ' ' };
	char spaces_plus[] = { ' ', '+' };
	char spaces_nl[] = { ' ', '\t', '\n' };

	XCTAssertEqualFreeableStrings(SMStringTrimCharacter(" abcd", spaces, sizeof(spaces), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("   abcd", spaces, sizeof(spaces), false), "abcd");
	
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("abcd ", spaces, sizeof(spaces), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("abcd   ", spaces, sizeof(spaces), false), "abcd");
	
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter(" abcd ", spaces, sizeof(spaces), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("   abcd   ", spaces, sizeof(spaces), false), "abcd");

	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("++++abcd", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("++++abcd    ", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("++++  abcd+++ ", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("+ +  ++  abcd  +++   +    +", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("+ +  ++    +++   +    +", spaces_plus, sizeof(spaces_plus), false), "");

	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("abcd", spaces_plus, sizeof(spaces_plus), false), "abcd");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("", spaces_plus, sizeof(spaces_plus), false), "");

	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("ab cd ef    gh", spaces, sizeof(spaces), false), "ab cd ef    gh");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("ab cd ef    gh    ", spaces, sizeof(spaces), false), "ab cd ef    gh");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("    ab cd ef    gh", spaces, sizeof(spaces), false), "ab cd ef    gh");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("    ab cd ef    gh    ", spaces, sizeof(spaces), false), "ab cd ef    gh");
	
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("#\n", spaces_nl, sizeof(spaces_nl), false), "#");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("    ", spaces, sizeof(spaces), false), "");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter(" ", spaces, sizeof(spaces), false), "");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("", spaces, sizeof(spaces), false), "");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter(" a", spaces, sizeof(spaces), false), "a");
	XCTAssertEqualFreeableStrings(SMStringTrimCharacter("a ", spaces, sizeof(spaces), false), "a");
}

- (void)testStringReplaceString
{
	XCTAssertEqualFreeableStrings(SMStringReplaceString("hello world", "", "nothing", false), "hello world");
	XCTAssertEqualFreeableStrings(SMStringReplaceString("hello world, beautiful world", "world", "", false), "hello , beautiful ");
	
	XCTAssertEqualFreeableStrings(SMStringReplaceString("hello world, beautiful world", "world", "people", false), "hello people, beautiful people");
	
	XCTAssertEqualFreeableStrings(SMStringReplaceString("", "world", "people", false), "");
	
	XCTAssertEqualFreeableStrings(SMStringReplaceString("hello world", "toto", "tutu", false), "hello world");
}

- (void)testPathAppendComponent
{
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("", "hello"), "/hello");
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("", ""), "");
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("/welcome", ""), "/welcome");
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("/welcome///", "///world"), "/welcome/world");
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("/welcome", "///world"), "/welcome/world");
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("/welcome/", "world"), "/welcome/world");
	XCTAssertEqualFreeableStrings(SMStringPathAppendComponent("/welcome", "world"), "/welcome/world");
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
