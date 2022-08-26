/*
 *  SMCommandLineTests.m
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


/*
** Externs
*/
#pragma mark - Externs

extern int main(int argc, const char * argv[]);


/*
** SMCommandLineTests
*/
#pragma mark - SMCommandLineTests

@interface SMCommandLineTests : XCTestCase

@end

@implementation SMCommandLineTests

- (void)testNotArguments
{
	const char *argv[] = {
		"ut"
	};
	
	XCTAssertEqual(main(sizeof(argv) / sizeof(*argv), argv), 1);
}

- (void)testVersion
{
	const char *argv[] = {
		"ut",
		"version"
	};
	
	XCTAssertEqual(main(sizeof(argv) / sizeof(*argv), argv), 0);
}

- (void)testInvalidVerb
{
	const char *argv[] = {
		"ut",
		"invalid-verb"
	};
	
	XCTAssertEqual(main(sizeof(argv) / sizeof(*argv), argv), 2);
}

// FIXME: add more exhaustive tests.

@end
