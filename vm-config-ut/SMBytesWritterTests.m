/*
 *  SMBytesWritterTests.m
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

#import "SMBytesWritter.h"

#import "SMTestCase.h"


/*
** SMBytesWritterTests
*/
#pragma mark - SMBytesWritterTests

@interface SMBytesWritterTests : SMTestCase

@end

@implementation SMBytesWritterTests

- (void)testOperations
{
	SMBytesWritter	writer = SMBytesWritterInit();
	NSMutableData	*ref = [NSMutableData data];
	
	off_t off = 0;
	
	uint8_t bytes1[] = { 0x42, 0x51, 0x12 };
	uint8_t bytes2[] = { 0xca, 0xfe };
	
	uint8_t byte1 = 0xde;
	uint8_t byte2 = 0xad;
	
	uint8_t rbytes[12] = { };
	
	memset(rbytes, 0xff, sizeof(rbytes));

	//
	off = SMBytesWritterAppendBytes(&writer, bytes1, sizeof(bytes1));
	[ref appendBytes:bytes1 length:sizeof(bytes1)];
	
	XCTAssertEqual(off, ref.length - sizeof(bytes1));
	
	//
	off = SMBytesWritterAppendBytes(&writer, bytes2, sizeof(bytes2));
	[ref appendBytes:bytes2 length:sizeof(bytes2)];
	
	XCTAssertEqual(off, ref.length - sizeof(bytes2));

	//
	off = SMBytesWritterAppendByte(&writer, byte1);
	[ref appendBytes:&byte1 length:sizeof(byte1)];
	
	XCTAssertEqual(off, ref.length - sizeof(byte1));

	//
	off = SMBytesWritterAppendByte(&writer, byte2);
	[ref appendBytes:&byte2 length:sizeof(byte2)];
	
	XCTAssertEqual(off, ref.length - sizeof(byte2));

	//
	off = SMBytesWritterAppendRepeatedByte(&writer, *rbytes, sizeof(rbytes));
	[ref appendBytes:rbytes length:sizeof(rbytes)];
	
	XCTAssertEqual(off, ref.length - sizeof(rbytes));

	//
	off = SMBytesWritterAppendByte(&writer, byte1);
	[ref appendBytes:&byte1 length:sizeof(byte1)];
	
	XCTAssertEqual(off, ref.length - sizeof(byte1));

	//
	XCTAssertEqual(SMBytesWritterSize(&writer), ref.length);
	XCTAssertEqual(memcmp(SMBytesWritterPtr(&writer), ref.bytes, ref.length), 0);
	
	// Clean.
	SMBytesWritterFree(&writer);
}

@end
