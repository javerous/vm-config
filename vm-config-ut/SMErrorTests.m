/*
 *  SMErrorTests.m
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

#import "SMError.h"

#import "SMTestCase.h"

@interface SMErrorTests : SMTestCase

@end

@implementation SMErrorTests

- (void)testSentensize
{
	SMError *error1 = SMErrorCreate("domain", -1, "hello world");
	SMError *error2 = SMErrorCreate("domain", -1, "hello world.");
	SMError *error3 = SMErrorCreate("domain", -1, "Hello world.");
	SMError *error4 = SMErrorCreate("domain", -1, "");

	XCTAssertEqual(strcmp(SMErrorGetSentencizedUserInfo(error1), "Hello world."), 0);
	XCTAssertEqual(strcmp(SMErrorGetSentencizedUserInfo(error2), "Hello world."), 0);
	XCTAssertEqual(strcmp(SMErrorGetSentencizedUserInfo(error3), "Hello world."), 0);
	XCTAssertEqual(strcmp(SMErrorGetSentencizedUserInfo(error4), ""), 0);

	SMErrorFree(error1);
	SMErrorFree(error2);
	SMErrorFree(error3);
	SMErrorFree(error4);
}

@end
