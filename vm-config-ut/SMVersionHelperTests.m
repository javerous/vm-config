/*
 *  SMVersionHelperTests.m
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

#import "SMVersionHelper.h"


/*
** SMVersionHelperTests
*/
#pragma mark - SMVersionHelperTests

@interface SMVersionHelperTests : XCTestCase

@end

@implementation SMVersionHelperTests

- (void)testCreateFromComponents
{
	SMVersion v = SMVersionFromComponents(42, 51, 12);
	
	XCTAssertEqual(v.major_version, 42);
	XCTAssertEqual(v.minor_version, 51);
	XCTAssertEqual(v.patch_version, 12);
}

- (void)testCreateFromString
{
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("0.0.0", NULL), SMVersionFromComponents(0, 0, 0)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromString("0.0.0", NULL), SMVersionFromComponents(1, 2, 3)));

	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("1.2.3", NULL), SMVersionFromComponents(1, 2, 3)));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.51.12", NULL), SMVersionFromComponents(42, 51, 12)));
	
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.51.", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42..12", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString(".51.12", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.51.12a", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.51.12.", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("z42.51.12", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.51b.12", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString(" 42.51.12", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.51.12 ", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42", NULL), SMVersionInvalid));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromString("42.12", NULL), SMVersionInvalid));
}

- (void)testEqual
{
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromComponents(42, 51, 12), SMVersionFromComponents(42, 51, 12)));
	XCTAssertTrue(SMVersionIsEqual(SMVersionFromComponents(0, 0, 0), SMVersionFromComponents(0, 0, 0)));

	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(42, 51, 12), SMVersionFromComponents(0, 51, 12)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(42, 51, 12), SMVersionFromComponents(42, 0, 12)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(42, 51, 12), SMVersionFromComponents(42, 51, 0)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(42, 51, 12), SMVersionFromComponents(0, 0, 0)));

	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(0, 51, 12), SMVersionFromComponents(42, 51, 12)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(42, 0, 12), SMVersionFromComponents(42, 51, 12)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(42, 51, 0), SMVersionFromComponents(42, 51, 12)));
	XCTAssertFalse(SMVersionIsEqual(SMVersionFromComponents(0, 0, 0), SMVersionFromComponents(42, 51, 12)));
}

- (void)testGreater
{
	XCTAssertTrue(SMVersionIsGreater(SMVersionFromComponents(43, 51, 12), SMVersionFromComponents(42, 51, 12)));
	XCTAssertTrue(SMVersionIsGreater(SMVersionFromComponents(42, 52, 12), SMVersionFromComponents(42, 51, 12)));
	XCTAssertTrue(SMVersionIsGreater(SMVersionFromComponents(42, 51, 13), SMVersionFromComponents(42, 51, 12)));
	XCTAssertFalse(SMVersionIsGreater(SMVersionFromComponents(42, 51, 12), SMVersionFromComponents(42, 51, 12)));

	BOOL greaterOrEqual = SMVersionIsGreaterOrEqual(SMVersionFromComponents(43, 51, 12), SMVersionFromComponents(42, 51, 12));
	
	XCTAssertTrue(greaterOrEqual);
}

@end
